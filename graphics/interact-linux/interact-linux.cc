/*
	interact-linux.cc
	-----------------
*/

// POSIX
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>

// Standard C
#include <errno.h>
#include <signal.h>
#include <stdlib.h>

// more-posix
#include "more/perror.hh"

// command
#include "command/get_option.hh"

// pass_fd
#include "unet/pass_fd.hh"


#define READER       "/opt/metamage/bin/reader"
#define SPIEL_MOUSE  "spiel-mouse"
#define DEV_MOUSE    "/dev/input/mice"
#define GFX          "gfx"
#define DISPLAY      "display"

#define PROGRAM  "interact-linux"

#define USAGE  \
"usage: " PROGRAM " --raster <raster-path>\n"  \
"       where raster-path is a raster file\n"

#define SUDO_NEEDED "Note: root privileges required to read /dev/input/mice"

#define STR_LEN( s )  "" s, (sizeof s - 1)

#define WARN( msg )  write( STDERR_FILENO, STR_LEN( PROGRAM ": " msg "\n" ) )


enum
{
	Opt_title   = 't',
	Opt_magnify = 'x',  // unimplemented, but accepted for compatibility
	
	Opt_last_byte = 255,
	
	Opt_raster,
};

static command::option options[] =
{
	{ "magnify", Opt_magnify, command::Param_required },
	{ "raster",  Opt_raster,  command::Param_required },
	{ "title",   Opt_title,   command::Param_required },
	{ NULL }
};


static const char* raster_path;


static pid_t mouser_pid = 0;
static pid_t viewer_pid = 0;


static inline
void report_error( const char* path, int err )
{
	more::perror( PROGRAM, path, err );
}

static
char* const* get_options( char** argv )
{
	int opt;
	
	++argv;  // skip arg 0
	
	while ( (opt = command::get_option( (char* const**) &argv, options )) > 0 )
	{
		using command::global_result;
		
		switch ( opt )
		{
			case Opt_raster:
				raster_path = global_result.param;
				break;
			
			case Opt_title:
				// For compatibility with FORGE interact -- ignored
			
			default:
				break;
		}
	}
	
	return argv;
}

static
void exec_or_exit( const char* const argv[] )
{
	execvp( *argv, (char**) argv );
	
	const int saved_errno = errno;
	
	report_error( *argv, saved_errno );
	
	_exit( saved_errno == ENOENT ? 127 : 126 );
}

static
bool readable( const char* path )
{
	int nok = access( path, R_OK );
	
	if ( nok  &&  errno != EACCES )
	{
		report_error( path, errno );
		exit( 1 );
	}
	
	return nok == 0;
}

static
int reader( const char* path )
{
	int fds[ 2 ];
	
	int nok = socketpair( PF_UNIX, SOCK_STREAM, 0, fds );
	
	if ( nok )
	{
		report_error( "socketpair", errno );
		exit( 1 );
	}
	
	pid_t reader_pid = fork();
	
	if ( reader_pid < 0 )
	{
		report_error( "fork", errno );
		exit( 1 );
	}
	
	if ( reader_pid == 0 )
	{
		dup2( fds[ 1 ], STDOUT_FILENO );
		
		const char* argv[] = { "sudo", READER, DEV_MOUSE, NULL };
		
		exec_or_exit( argv );
	}
	
	int reading_fd = unet::recv_fd( fds[ 0 ] );
	
	if ( reading_fd < 0 )
	{
		report_error( "recv_fd", errno );
		exit( 1 );
	}
	
	int status;
	pid_t pid = waitpid( reader_pid, &status, 0 );
	
	if ( pid < 0 )
	{
		report_error( "waitpid", errno );
		exit( 1 );
	}
	
	if ( status != 0 )
	{
		if ( WIFSIGNALED( status ) )
		{
			exit( 128 + WTERMSIG( status ) );
		}
		else if ( status != 0 )
		{
			exit( WEXITSTATUS( status ) );
		}
	}
	
	close( fds[ 0 ] );
	close( fds[ 1 ] );
	
	return reading_fd;
}

static
void launch_subprocesses( char* const* args )
{
	const bool sudo_needed = ! readable( DEV_MOUSE );
	
	if ( sudo_needed )
	{
		WARN( SUDO_NEEDED );
		
		int status = system( "sudo true" );
		
		if ( WIFSIGNALED( status ) )
		{
			exit( 128 + WTERMSIG( status ) );
		}
		else if ( status != 0 )
		{
			exit( WEXITSTATUS( status ) );
		}
	}
	
	mouser_pid = fork();
	
	if ( mouser_pid < 0 )
	{
		report_error( "fork", errno );
		exit( 1 );
	}
	
	if ( mouser_pid == 0 )
	{
		const char* argv[] = { SPIEL_MOUSE, DEV_MOUSE, NULL };
		
		if ( sudo_needed )
		{
			const int reader_fd = reader( DEV_MOUSE );
			
			dup2( reader_fd, STDIN_FILENO );
			
			close( reader_fd );
			
			argv[ 1 ]  = NULL;
		}
		
		exec_or_exit( argv );
	}
	
	/*
		Send a NUL byte.  This may be intercepted by exhibit to indicate
		readiness, or passed on to the raster author as a null SPIEL message.
	*/
	
	write( STDOUT_FILENO, "", 1 );
	
	/*
		Now that mouse input is set up, close the I/O channel so we retain no
		references to the pipe/socket that the interactive is reading from.
		That way, if the spiel-mouse process is killed, read() will return 0
		and the interactive will exit.
	*/
	
	dup2( STDERR_FILENO, STDIN_FILENO  );
	dup2( STDERR_FILENO, STDOUT_FILENO );
	
	viewer_pid = fork();
	
	if ( viewer_pid < 0 )
	{
		report_error( "fork", errno );
		exit( 1 );
	}
	
	if ( viewer_pid == 0 )
	{
		const char* argv[] = { GFX, DISPLAY, "-w", raster_path, NULL };
		
		exec_or_exit( argv );
	}
}

int main( int argc, char** argv )
{
	if ( argc == 0 )
	{
		return 0;
	}
	
	char* const* args = get_options( argv );
	
	int argn = argc - (args - argv);
	
	if ( raster_path == NULL )
	{
		write( STDERR_FILENO, STR_LEN( USAGE ) );
		return 2;
	}
	
	launch_subprocesses( args );
	
	int wait_status;
	
	pid_t child = wait( &wait_status );
	
	/*
		One of out subprocesses has terminated.  If it's the viewer, kill the
		other one.  But if it's the input translator, do nothing, since the
		viewer will exit on its own (after restoring any configuration changes
		it made to the console ).
	*/
	
	if ( child == viewer_pid )
	{
		kill( mouser_pid, SIGTERM );
	}
	
	child = wait( &wait_status );
	
	return 0;
}
