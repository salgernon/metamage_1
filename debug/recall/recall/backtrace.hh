/*	============
 *	backtrace.hh
 *	============
 */

#ifndef RECALL_BACKTRACE_HH
#define RECALL_BACKTRACE_HH

// Standard C++
#include <vector>

// Recall
#include "recall/stack_crawl.hh"


namespace plus
{
	
	class var_string;
	
}

namespace recall
{
	
	void make_report_from_stack_crawl( plus::var_string&                          result,
	                                   std::vector< frame_data >::const_iterator  begin,
	                                   std::vector< frame_data >::const_iterator  end );
	
	class debugging_context
	{
		private:
			std::vector< frame_data > its_stack_crawl;
		
		public:
			debugging_context();
			
			const std::vector< frame_data >& get_stack_crawl() const
			{
				return its_stack_crawl;
			}
	};
	
}

#endif

