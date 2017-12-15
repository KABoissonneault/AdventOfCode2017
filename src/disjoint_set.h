#pragma once

#include <initializer_list>
#include <set>
#include "algorithm.h"

namespace kab_advent {
	template<typename T>
	class disjoint_set {
		struct set_element {
			T elem;
			T parent = elem;
			int rank = 0;
		};

	public:
		void add_element( T elem ) {
			elements.push_back( set_element { elem } );
		}
		void add_element( std::initializer_list<T> elems ) {
			for ( T const& elem : elems ) {
				add_element( elem );
			}
		}
		template<typename RangeT> 
		void add_elements(RangeT&& range) {
			for ( auto && e : std::forward<RangeT>(range) ) {
				add_element( e );
			}
		}

		auto find_root( T elem ) const -> T {			
			return do_find_root( get_set_element(elem) ).elem;
		}

		auto find_root( T elem ) -> T {
			return do_find_root( get_set_element( elem ) ).elem;
		}

		void unite( T lhs, T rhs ) {
			auto lhs_it = std::find_if( elements.begin(), elements.end(), [lhs] ( set_element const& e ) { return e.elem == lhs; } );
			auto rhs_it = std::find_if( elements.begin(), elements.end(), [rhs] ( set_element const& e ) { return e.elem == rhs; } );
			assert( lhs_it != elements.end() && rhs_it != elements.end() );
			do_unite( *lhs_it, *rhs_it );
		}

	private:
		auto get_set_element( T elem ) -> set_element & {
			auto const elem_it = std::find_if( elements.begin(), elements.end(), [elem] ( set_element const& e ) { return e.elem == elem; } );
			assert( elem_it != elements.end() );
			return *elem_it;
		}
		auto do_find_root( set_element & e ) -> set_element & {
			if ( e.parent != e.elem ) {
				e.parent = do_find_root( get_set_element( e.parent ) ).elem;
			}
			return get_set_element( e.parent );
		}

		void do_unite( set_element & lhs, set_element & rhs ) {
			auto & lhs_root = do_find_root( lhs );
			auto & rhs_root = do_find_root( rhs );

			if ( lhs_root.elem == rhs_root.elem ) {
				return;
			}

			if ( lhs_root.rank < rhs_root.rank ) {
				lhs_root.parent = rhs_root.elem;
			} else if ( lhs_root.rank > rhs_root.rank ) {
				rhs_root.parent = lhs_root.elem;
			} else {
				rhs_root.parent = lhs_root.elem;
				++lhs_root.rank;
			}
		}

		std::vector<set_element> elements;
	};
}
