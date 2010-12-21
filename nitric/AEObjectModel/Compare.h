/*	=========
 *	Compare.h
 *	=========
 */

#ifndef AEOBJECTMODEL_COMPARE_H
#define AEOBJECTMODEL_COMPARE_H

#include <map>

#ifndef NITROGEN_AEOBJECTS_HH
#include "Nitrogen/AEObjects.hh"
#endif


namespace Nitrogen
{
	
	bool Compare( AECompOperator            op,
			      const Mac::AEDesc_Token&  obj1,
			      const Mac::AEDesc_Token&  obj2 );
	
	template < ::DescType tokenType > struct Compare_Traits;
	
	class Comparer
	{
		public:
			typedef bool (*Callback)( AECompOperator, const Mac::AEDesc_Token&, const Mac::AEDesc_Token& );
		
		private:
			typedef std::map< Mac::DescType, Callback >  Map;
			
			Map map;
			
			// not implemented:
			Comparer( const Comparer& );
			Comparer& operator=( const Comparer& );
		
		public:
			Comparer()  {}
			
			void Register( Mac::DescType tokenType, Comparer::Callback callback )
			{
				map[ tokenType ] = callback;
			}
			
			template < ::DescType tokenType >
			void Register()
			{
				Register( tokenType, Compare_Traits< tokenType >::Compare );
			}
			
			bool Compare( AECompOperator            op,
			              const Mac::AEDesc_Token&  obj1,
			              const Mac::AEDesc_Token&  obj2 );
	};
	
	Comparer& TheGlobalComparer();
	
	inline void RegisterComparer( Mac::DescType tokenType, Comparer::Callback callback )
	{
		TheGlobalComparer().Register( tokenType, callback );
	}
	
	template < ::DescType tokenType >
	inline void RegisterComparer()
	{
		TheGlobalComparer().template Register< tokenType >();
	}
	
	template < ::DescType descType >
	struct BasicComparer
	{
		typedef typename DescType_Traits< descType >::Result Result;
		
		static bool Compare( AECompOperator            op,
		                     const Mac::AEDesc_Token&  obj1,
		                     const Mac::AEDesc_Token&  obj2 )
		{
			if ( op != AECompOperator( kAEEquals ) )
			{
				ThrowOSStatus( errAEEventNotHandled );
			}
			
			Result a = AEGetDescData< descType >( obj1 );
			Result b = AEGetDescData< descType >( obj2 );
			
			return a == b;
		}
	};
	
	template < ::DescType descType >
	struct NumericComparer
	{
		typedef typename DescType_Traits< descType >::Result Result;
		
		static bool Compare( AECompOperator            op,
		                     const Mac::AEDesc_Token&  obj1,
		                     const Mac::AEDesc_Token&  obj2 )
		{
			Result a = AEGetDescData< descType >( obj1 );
			Result b = AEGetDescData< descType >( obj2 );
			
			switch ( op )
			{
				case kAEEquals:
					return a == b;
				
				case kAEGreaterThan:
					return a > b;
				
				case kAEGreaterThanEquals:
					return a >= b;
				
				case kAELessThan:
					return a < b;
				
				case kAELessThanEquals:
					return a <= b;
				
				default:
					break;
			}
			
			ThrowOSStatus( errAEEventNotHandled );
		}
	};
	
}

#endif
