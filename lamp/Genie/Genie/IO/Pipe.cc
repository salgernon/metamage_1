/*	=======
 *	Pipe.cc
 *	=======
 */

#include "Genie/IO/Pipe.hh"

// POSIX
#include <fcntl.h>

// Debug
#include "debug/boost_assert.hh"

// Boost
#include <boost/intrusive_ptr.hpp>

// plus
#include "plus/conduit.hh"

// vfs
#include "vfs/filehandle.hh"
#include "vfs/enum/poll_result.hh"
#include "vfs/filehandle/functions/nonblocking.hh"
#include "vfs/filehandle/methods/filehandle_method_set.hh"
#include "vfs/filehandle/methods/stream_method_set.hh"

// Genie
#include "Genie/api/signals.hh"
#include "Genie/api/yield.hh"


namespace Genie
{
	
	enum pipe_end_type
	{
		Pipe_reader,
		Pipe_writer,
	};
	
	class PipeEndHandle : public vfs::filehandle
	{
		private:
			boost::intrusive_ptr< plus::conduit > itsConduit;
			pipe_end_type                         its_pipe_end_type;
		
		public:
			PipeEndHandle( const boost::intrusive_ptr< plus::conduit >&  conduit,
			               int                                           open_flags,
			               pipe_end_type                                 end_type );
			
			~PipeEndHandle();
			
			plus::conduit& get_conduit() const  { return *itsConduit; }
			
			pipe_end_type which_end() const  { return its_pipe_end_type; }
	};
	
	static unsigned pipein_poll( vfs::filehandle* that )
	{
		plus::conduit& conduit = static_cast< PipeEndHandle& >( *that ).get_conduit();
		
		return + vfs::Poll_read
		       | vfs::Poll_write * conduit.is_writable();
	}
	
	static ssize_t pipein_read( vfs::filehandle* that, char* buffer, size_t n )
	{
		plus::conduit& conduit = static_cast< PipeEndHandle& >( *that ).get_conduit();
		
		return conduit.read( buffer, n, is_nonblocking( *that ), &try_again );
	}
	
	static unsigned pipeout_poll( vfs::filehandle* that )
	{
		plus::conduit& conduit = static_cast< PipeEndHandle& >( *that ).get_conduit();
		
		return + vfs::Poll_read * conduit.is_readable()
		       | vfs::Poll_write;
	}
	
	static ssize_t pipeout_write( vfs::filehandle* that, const char* buffer, size_t n )
	{
		plus::conduit& conduit = static_cast< PipeEndHandle& >( *that ).get_conduit();
		
		return conduit.write( buffer, n, is_nonblocking( *that ), &try_again, &broken_pipe );
	}
	
	static const vfs::stream_method_set pipein_stream_methods =
	{
		&pipein_poll,
		&pipein_read,
	};
	
	static const vfs::stream_method_set pipeout_stream_methods =
	{
		&pipeout_poll,
		NULL,
		&pipeout_write,
	};
	
	static const vfs::filehandle_method_set pipein_methods =
	{
		NULL,
		NULL,
		&pipein_stream_methods,
	};
	
	static const vfs::filehandle_method_set pipeout_methods =
	{
		NULL,
		NULL,
		&pipeout_stream_methods,
	};
	
	
	static inline const vfs::filehandle_method_set& methods_for_end( pipe_end_type type )
	{
		return type == Pipe_reader ? pipein_methods
		                           : pipeout_methods;
	}
	
	PipeEndHandle::PipeEndHandle( const boost::intrusive_ptr< plus::conduit >&  conduit,
	                              int                                           open_flags,
	                              pipe_end_type                                 end_type )
	:
		vfs::filehandle( open_flags, &methods_for_end( end_type ) ),
		itsConduit( conduit ),
		its_pipe_end_type( end_type )
	{
	}
	
	PipeEndHandle::~PipeEndHandle()
	{
		if ( which_end() == Pipe_reader )
		{
			itsConduit->close_egress();
		}
		else
		{
			itsConduit->close_ingress();
		}
	}
	
	pipe_ends new_pipe( int nonblock )
	{
		boost::intrusive_ptr< plus::conduit > conduit( new plus::conduit );
		
		pipe_ends result;
		
		const int open_flags = nonblock ? O_RDONLY | O_NONBLOCK
		                                : O_RDONLY;
		
		result.writer.reset( new PipeEndHandle( conduit, open_flags, Pipe_writer ) );
		result.reader.reset( new PipeEndHandle( conduit, open_flags, Pipe_reader ) );
		
		return result;
	}
	
}

