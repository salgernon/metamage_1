/*
	full_iterator.cc
	----------------
*/

#include "vlib/iterators/full_iterator.hh"

// debug
#include "debug/assert.hh"

// vlib
#include "vlib/symbol.hh"
#include "vlib/dispatch/dispatch.hh"
#include "vlib/dispatch/refs.hh"


namespace vlib
{
	
	static inline
	get_refs get_getrefs( const Value& v )
	{
		if ( const dispatch* methods = v.dispatch_methods() )
		{
			if ( const refs* ref = methods->ref )
			{
				return ref->getrefs;
			}
		}
		
		return NULL;
	}
	
	static
	void put( const Value& v, void* param )
	{
		typedef std::vector< const Value* > Stack;
		
		Stack& stack = *(Stack*) param;
		
		stack.push_back( &v );
	}
	
	full_iterator::full_iterator( const Value& start )
	{
		its_stack.push_back( &start );
	}
	
	const Value& full_iterator::get() const
	{
		ASSERT( ! finished() );
		
		return *its_stack.back();
	}
	
	static
	void push( std::vector< const Value* >& stack, const Value* v )
	{
		if ( ! v->is_cycle_free() )
		{
			stack.push_back( v );
		}
	}
	
	void full_iterator::step()
	{
		ASSERT( ! finished() );
		
		const Value& v = *its_stack.back();
		
		its_stack.pop_back();
		
		if ( Expr* expr = v.expr() )
		{
			push( its_stack, &expr->right );
			push( its_stack, &expr->left  );
		}
		else if ( const Symbol* sym = v.sym() )
		{
			push( its_stack, &sym->get() );
			push( its_stack, &sym->vtype() );
		}
		else if ( get_refs getrefs = get_getrefs( v ) )
		{
			getrefs( v, &put, &its_stack );
		}
	}
	
	void full_iterator::prune()
	{
		ASSERT( ! finished() );
		
		its_stack.pop_back();
	}
	
}
