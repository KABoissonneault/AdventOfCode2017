#pragma once

#include <string>
#include <string_view>
#include "error.h"

namespace kab_advent {
	template<typename T>
	struct conversion_result {
		T data;
		char const* conversion_end;
	};

	inline auto to_int(std::string_view input) -> expected<conversion_result<int>> {
		using namespace std::string_literals;
		char* conversion_end = nullptr;
		auto const str = std::string( input );
		auto const token_value = std::strtol( str.c_str(), &conversion_end, 10 );
		if ( conversion_end == str.c_str() ) {
			return make_unexpected( error_info( std::make_error_code( std::errc::invalid_argument ), "Invalid integer string \""s.append( str ).append( "\"" ) ) );
		}

		auto const input_conversion_end = input.data() + std::distance( str.c_str(), static_cast<char const*>(conversion_end) );
		return conversion_result<int>{ static_cast<int>( token_value ), input_conversion_end };
	}
}