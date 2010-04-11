/*	================
 *	SourceDotList.cc
 *	================
 */

#include "A-line/SourceDotList.hh"

// text-input
#include "text_input/feed.hh"
#include "text_input/get_line_from_feed.hh"

// poseven
#include "poseven/extras/fd_reader.hh"
#include "poseven/functions/open.hh"


namespace tool
{
	
	namespace n = nucleus;
	namespace p7 = poseven;
	
	
	void ReadSourceDotList( const std::string&           pathname,
	                        std::vector< std::string >&  files )
	{
		text_input::feed feed;
		
		n::owned< p7::fd_t > fd = p7::open( pathname, p7::o_rdonly );
		
		p7::fd_reader reader( fd );
		
		while ( const std::string* s = get_line_from_feed( feed, reader ) )
		{
			std::string line( s->begin(), s->end() - 1 );
			
			if ( line.empty()      )  continue;
			if ( line[ 0 ] == ';'  )  continue;
			if ( line[ 0 ] == '#'  )  continue;
			if ( ~line.find( ':' ) )  continue;
			
			files.push_back( line.substr( line.find_first_not_of( "\t" ),
			                              line.npos ) );
		}
	}
	
}

