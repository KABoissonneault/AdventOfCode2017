#pragma once

#include <vector>
#include <type_traits>

namespace kab_advent {
	template<typename RangeT, typename PredicateT, 
		typename T = std::remove_cv_t<
			std::remove_reference_t< decltype( std::declval<RangeT>()[0] ) >
		>
	>
	std::vector<T> filter( const RangeT & range, PredicateT predicate ) {
		using std::begin; using std::end;
		std::vector<T> out;
		std::copy_if( begin( range ), end( range ), std::back_inserter( out ), predicate );
		return out;
	}
}