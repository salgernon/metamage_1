/*
	library.cc
	----------
*/

#include "vlib/library.hh"

// POSIX
#include <unistd.h>

// Standard C
#include <stdlib.h>
#include <time.h>

// must
#include "must/write.h"

// plus
#include "plus/string/concat.hh"

// vlib
#include "vlib/error.hh"
#include "vlib/string-utils.hh"


namespace vlib
{
	
	static
	Value v_getenv( const Value& v )
	{
		if ( get_type( v ) != Value_string )
		{
			TYPE_ERROR( "getenv() argument must be a string" );
		}
		
		if ( strlen( get_str( v ).c_str() ) != get_str( v ).size() )
		{
			TYPE_ERROR( "getenv() argument must not contain NUL bytes" );
		}
		
		if ( const char* var = getenv( get_str( v ).c_str() ) )
		{
			return var;
		}
		
		return Value_empty_list;
	}
	
	static
	Value v_print( const Value& v )
	{
		const plus::string s = make_string( v, Stringified_to_print ) + "\n";
		
		must_write( STDOUT_FILENO, s.data(), s.size() );
		
		return Value();
	}
	
	static
	Value v_time( const Value& v )
	{
		if ( ! is_empty( v ) )
		{
			TYPE_ERROR( "time takes no arguments" );
		}
		
		const time_t t = time( NULL );
		
		return t;
	}
	
	const proc_info proc_getenv = { &v_getenv, "getenv" };
	const proc_info proc_print  = { &v_print,  "print"  };
	const proc_info proc_time   = { &v_time,   "time"   };
	
}