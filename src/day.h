#pragma once

#include <string_view>
#include <gsl/span>

namespace kab_advent {
	auto day(gsl::span<std::string_view const> args) -> int;
}