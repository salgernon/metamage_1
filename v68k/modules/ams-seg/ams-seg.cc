/*
	ams-seg.cc
	----------
*/

// Mac OS
#ifndef __TRAPS__
#include <Traps.h>
#endif

// POSIX
#include <unistd.h>

// command
#include "command/get_option.hh"

// ams-seg
#include "Segments.hh"
#include "options.hh"


#define STR_LEN( s )  "" s, (sizeof s - 1)

#define PROGRAM  "ams-seg"

#define WARN( msg )  write( STDERR_FILENO, STR_LEN( PROGRAM ": " msg "\n" ) )


enum
{
	Opt_linger = 'L',  // linger on ExitToShell
};

static command::option options[] =
{
	{ "linger",  Opt_linger },
	
	NULL,
};


void* toolbox_trap_table[] : 3 * 1024;

#define TBTRAP( Proc )  (toolbox_trap_table[ _##Proc & 0x03FF ] = &Proc##_patch)


static void install_SegmentLoader()
{
	TBTRAP( LoadSeg     );  // A9F0
	TBTRAP( Launch      );  // A9F2
	TBTRAP( ExitToShell );  // A9F4
}

static asm void module_suspend()
{
	JSR      0xFFFFFFF8
}

static char* const* get_options( char** argv )
{
	using command::global_result;
	
	int opt;
	
	++argv;  // skip arg 0
	
	while ( (opt = command::get_option( (char* const**) &argv, options )) > 0 )
	{
		switch ( opt )
		{
			case Opt_linger:
				linger_on_exit = true;
				break;
			
			default:
				break;
		}
	}
	
	return argv;
}

int main( int argc, char** argv )
{
	if ( argc > 0 )
	{
		char* const* args = get_options( argv );
		
		if ( *args != NULL )
		{
			WARN( "no arguments allowed" );
			
			_exit( 1 );
		}
	}
	
	install_SegmentLoader();
	
	module_suspend();  // doesn't return
}
