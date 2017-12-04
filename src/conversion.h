#pragma once

#include <string>
#include <string_view>
#include "error.h"

namespace kab_advent {
	inline auto to_int(std::string const& str) -> expected<int> {
		using namespace std::string_literals;
		char* conversionEnd = nullptr;
		auto const tokenValue = std::strtol(str.c_str(), &conversionEnd, 10);
		if (conversionEnd == str.c_str()) {
			return expected<int>{ unexpect,
				error_info(std::make_error_code(std::errc::invalid_argument), "Invalid integer string \""s.append(str).append("\"")) };
		}
		return tokenValue;
	}
	inline auto to_int(std::string_view str) -> expected<int> {
		return to_int(std::string(str));
	}
}