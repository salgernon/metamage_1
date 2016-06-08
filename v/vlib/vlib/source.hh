/*
	source.hh
	---------
*/

#ifndef VLIB_SOURCE_HH
#define VLIB_SOURCE_HH

// plus
#include "plus/string.hh"


namespace vlib
{
	
	class source_spec
	{
		private:
			plus::string  its_file;
			unsigned      its_line;
		
		public:
			source_spec( const char* path = NULL ) : its_file(), its_line()
			{
				if ( path != NULL )
				{
					its_file = path;
					its_line = 1;
				}
			}
			
			bool empty() const  { return its_line == 0; }
			
			const plus::string& file() const  { return its_file; };
			unsigned            line() const  { return its_line; };
			
			void next_line()  { ++its_line; }
	};
	
}

#endif
