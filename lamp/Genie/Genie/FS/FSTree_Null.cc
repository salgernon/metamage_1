/*	==============
 *	FSTree_Null.cc
 *	==============
 */

#include "Genie/FS/FSTree_Null.hh"

// Standard C/C++
#include <cerrno>

// poseven
#include "poseven/Errno.hh"


namespace Genie
{
	
	namespace p7 = poseven;
	
	
	class FSTree_Null : public FSTree
	{
		public:
			bool Exists() const  { return false; }
			bool IsFile() const  { return false; }
			
			FSTreePtr Parent() const;
	};
	
	
	FSTreePtr FSTree_Null::Parent() const
	{
		p7::throw_errno( ENOENT );
		
		throw;
	}
	
	FSTreePtr FSNull()
	{
		return FSTreePtr( new FSTree_Null() );
	}
	
}

