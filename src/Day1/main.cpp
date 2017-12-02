#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <numeric>
#include <string_view>
#include <system_error>

#include <gsl/span>
#include <expected.hpp>

#include "algorithm.h"
#include "error.h"

using namespace std::literals;

namespace kab_advent {
	namespace {
		auto day1_input(gsl::span<std::string_view const> args) -> expected<std::string> {
			auto input = std::string();
			if (args.size() == 0) {
				if (!std::getline(std::cin, input)) {
					return expected<std::string>{ unexpect,
						error_info(std::make_error_code(std::errc::invalid_argument), "Could not read input") };
				}
			}
			else if (args[0] == "--input") {
				if (args.size() < 2) {
					return expected<std::string>{ unexpect,
						error_info(std::make_error_code(std::errc::invalid_argument), "Missing input after --input") };
				}
				input = args[1];
			}
			else {
				return expected<std::string>{ unexpect,
					error_info(std::make_error_code(std::errc::invalid_argument), "Invalid parameter \""s.append(args[0]).append("\"")) };
			}
			if (!std::all_of(input.begin(), input.end(),
				[](const char c) { return std::isdigit(static_cast<unsigned char>(c)); })) {
				return expected<std::string>{ unexpect,
					error_info(std::make_error_code(std::errc::invalid_argument), "Input \""s.append(input).append("\" was not numerical")) };
			}

			return input;
		}

		auto part(std::string_view captcha, int steps) -> int {
			struct char_pair {
				char c1;
				char c2;
			};
			using char_pair_arr = std::vector<char_pair>;

			const auto characters = [captcha, steps] {
				std::vector<char_pair> characters;
				for (auto i = char_pair_arr::size_type{ 0 }; i < captcha.size(); ++i) {
					characters.emplace_back(char_pair{ captcha[i], captcha[(i + steps) % captcha.size()] });
				}
				return characters;
			}();

			const auto adjacentCharacters = filter(characters, [](const char_pair c) { return c.c1 == c.c2; });
			const auto sum = std::accumulate(adjacentCharacters.begin(), adjacentCharacters.end(), 0,
				[](const int counter, const char_pair c) { return counter + static_cast<int>(c.c1 - '0'); });

			return sum;
		}

		auto part1(std::string_view captcha) -> int {
			return part(captcha, 1);
		}

		auto part2(std::string_view captcha) -> int {
			return part(captcha, gsl::narrow<int>(captcha.size() / 2));
		}

		auto day1(gsl::span<std::string_view const> args) -> int {
			if (args.size() < 2) {
				throw std::runtime_error("Missing part parameter");
			}
			auto const part = args[1];

			auto const captcha = day1_input(args.subspan(2));
			if (!captcha) {
				std::cerr << captcha.error() << "\n";
				return EXIT_FAILURE;
			}

			if (part == "1") {
				std::cout << part1(captcha.value()) << "\n";
				return EXIT_SUCCESS;
			}
			else if (part == "2") {
				std::cout << part2(captcha.value()) << "\n";
				return EXIT_SUCCESS;
			}
			else {
				throw std::runtime_error{ "Parameter \""s.append(part).append("\" was not a valid part (try 1 or 2)") };
			}
		}
	}
}

int main( int argc, const char* argv[] ) try {
	std::cout.sync_with_stdio(false);
	std::cerr.sync_with_stdio(false);
	std::cin.sync_with_stdio(false);
	
	auto const args = std::vector<std::string_view>( argv, argv + argc );
	try {
		return kab_advent::day1(args);
	}
	catch ( const std::runtime_error & e ) {
		std::cerr << "Invalid input: " << e.what() << "\n";
		return EXIT_FAILURE;
	}
} catch ( const std::exception & e ) {
	std::cerr << "Unhandled error in main: " << e.what() << "\n";
}
