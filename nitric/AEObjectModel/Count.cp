/*	========
 *	Count.cp
 *	========
 */

#ifndef AEOBJECTMODEL_COUNT_H
#include "AEObjectModel/Count.h"
#endif


namespace Nitrogen
{
	
	std::size_t Count( Mac::AEObjectClass        desiredClass,
	                   Mac::AEObjectClass        containerClass,
	                   const Mac::AEDesc_Token&  containerToken )
	{
		return TheGlobalCounter().Count( desiredClass, containerClass, containerToken );
	}
	
	std::size_t Counter::Count( Mac::AEObjectClass        desiredClass,
	                            Mac::AEObjectClass        containerClass,
	                            const Mac::AEDesc_Token&  containerToken )
	{
		Map::const_iterator found = map.find( Key( desiredClass, Mac::DescType( containerToken.descriptorType ) ) );
		
		if ( found == map.end() )
		{
			found = map.find( Key( Mac::AEObjectClass( typeWildCard ), Mac::DescType( containerToken.descriptorType ) ) );
			
			if ( found == map.end() )
			{
				found = map.find( Key( desiredClass, Mac::typeWildCard ) );
				
				if ( found == map.end() )
				{
					found = map.find( Key( Mac::AEObjectClass( typeWildCard ), Mac::typeWildCard ) );
					
					if ( found == map.end() )
					{
						ThrowOSStatus( errAEEventNotHandled );
					}
				}
			}
		}
		
		return found->second( desiredClass, containerClass, containerToken );
	}
	
	Counter& TheGlobalCounter()
	{
		static Counter theGlobalCounter;
		
		return theGlobalCounter;
	}
	
}

