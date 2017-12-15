#pragma once

#include <string_view>

namespace kab_advent {
    template<typename T>
    struct parsed_value {
        T value;
        std::string_view rest_instruction;
    };

	inline auto consume_newline( std::string_view & line ) -> bool {
		if ( !line.empty() && line.front() == '\n' ) {
			line.remove_prefix( 1 );
			return true;
		} else {
			return false;
		}
	}

	inline auto consume_char( std::string_view & line, char c ) -> bool {
		if ( !line.empty() && line.front() == c ) {
			line.remove_prefix( 1 );
			return true;
		} else {
			return false;
		}
	}

	inline auto consume_delimiter( std::string_view & line ) -> bool {
		return consume_char( line, ',' );
	}
}
