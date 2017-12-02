#include <gsl/span>
#include <expected.hpp>
#include <numeric>
#include <cctype>

#include "algorithm.h"
#include "error.h"


namespace kab_advent {
	namespace {
		using namespace std::literals;

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

		namespace day1 {
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

			auto solve(gsl::span<std::string_view const> args) -> int {
				if (args.size() < 1) {
					throw std::runtime_error("Missing part parameter");
				}
				auto const part = args[0];
				args = args.subspan(1);

				auto const captcha = day1_input(args);
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

	auto day(gsl::span<std::string_view const> args) -> int {
		if (args.size() < 1) {
			throw std::runtime_error("Missing part parameter");
		}
		auto const day = args[0];
		args = args.subspan(1);

		if (day == "1") {
			return day1::solve(args);
		}
		else {
			throw std::runtime_error{ "Parameter \""s.append(day).append("\" was not a valid day (try 1-25)") };
		}
	}
}