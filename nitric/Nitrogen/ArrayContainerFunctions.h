// ArrayContainerFunctions.h

#ifndef NITROGEN_ARRAYCONTAINERFUNCTIONS_H
#define NITROGEN_ARRAYCONTAINERFUNCTIONS_H

#include <cstddef>
#include <iterator>
#include <algorithm>

namespace Nitrogen
  {
   template < class Element, std::size_t size >
   Element *ArrayBegin( Element (&array)[size] )
     {
      return array;
     }
   
   template < class Element, std::size_t size >
   Element *ArrayEnd( Element (&array)[size] )
     {
      return array + size;
     }
   
   template < class Element, std::size_t size >
   std::size_t ArraySize( const Element (&/*array*/)[size] )
     {
      return size;
     }
   
   template < class Element, std::size_t size >
   std::reverse_iterator<Element *> ArrayRBegin( Element (&array)[size] )
     {
      return std::reverse_iterator<Element *>( ArrayEnd( array ) );
     }
   
   template < class Element, std::size_t size >
   std::reverse_iterator<Element *> ArrayREnd( Element (&array)[size] )
     {
      return std::reverse_iterator<Element *>( ArrayBegin( array ) );
     }
   
   template < class Element, std::size_t size >
   void ArrayAssign( Element (&destination)[size], const Element (&source)[size] )
     {
      std::copy( ArrayBegin( source ), ArrayEnd( source ), ArrayBegin( destination ) );
     }
   
   template < class Element, std::size_t size >
   void ArraySwap( Element (&a)[size], Element (&b)[size] )
     {
      Element *p = ArrayBegin( a );
      Element *q = ArrayBegin( b );
      while ( p != ArrayEnd( a ) )
         std::swap( *p++, *q++ );
     }
   
   template < class Element, std::size_t size >
   void ArrayEqualTo( const Element (&a)[size], const Element (&b)[size] )
     {
      return std::equal( ArrayBegin( a ), ArrayEnd( a ), ArrayBegin( b ) );
     }

   template < class Element, std::size_t size >
   void ArrayNotEqualTo( const Element (&a)[size], const Element (&b)[size] )
     {
      return !ArrayEqualTo( a, b );
     }
   
   template < class Element, std::size_t size >
   void ArrayLess( const Element (&a)[size], const Element (&b)[size] )
     {
      return std::lexicographical_compare( ArrayBegin( a ), ArrayEnd( a ),
                                           ArrayBegin( b ), ArrayEnd( b ) );
     }
   
   template < class Element, std::size_t size >
   void ArrayGreater( const Element (&a)[size], const Element (&b)[size] )
     {
      return ArrayLess( b, a );
     }
   
   template < class Element, std::size_t size >
   void ArrayLessEqual( const Element (&a)[size], const Element (&b)[size] )
     {
      std::pair< Element *, Element * > m = std::mismatch( ArrayBegin( a ), ArrayEnd( a ),
                                                           ArrayBegin( b ) );
      return m.first == ArrayEnd( a ) || *m.first < *m.second;
     }
   
   template < class Element, std::size_t size >
   void ArrayGreaterEqual( const Element (&a)[size], const Element (&b)[size] )
     {
      return ArrayLessEqual( b, a );
     }
  }

#endif
