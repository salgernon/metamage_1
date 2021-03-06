/*
	tracker.hh
	----------
*/

#ifndef VLIB_TRACKER_HH
#define VLIB_TRACKER_HH


namespace vlib
{
	
	class Value;
	struct namespace_info;
	
	void track_symbol( const Value& v );
	
	void add_root( const Value& v );
	void del_root( const Value& v );
	
	void cull_unreachable_objects();
	
	class scoped_root
	{
		private:
			const Value& its_root;
			
			// non-copyable
			scoped_root           ( const scoped_root& );
			scoped_root& operator=( const scoped_root& );
		
		public:
			scoped_root( const Value& root ) : its_root( root )
			{
				add_root( root );
			}
			
			~scoped_root()
			{
				del_root( its_root );
			}
	};
	
	extern const namespace_info namespace_tracker;
	
}

#endif
