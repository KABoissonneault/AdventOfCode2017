#include <gsl/span>
#include <expected.hpp>
#include <numeric>
#include <cctype>
#include <cassert>
#include <cmath>
#include <set>

#include "algorithm.h"
#include "error.h"
#include "conversion.h"
#include <fstream>

#if defined(_MSC_VER)
#define UNREACHABLE() __assume(0)
#else
#define UNREACHABLE() __builtin_unreachable()
#endif

namespace kab_advent {
	namespace {
		using namespace std::literals;

		namespace day1 {
			auto input(gsl::span<std::string_view const> args) -> expected<std::string> {
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

			auto solve(gsl::span<std::string_view const> args) -> int {
				if (args.size() < 1) {
					throw std::runtime_error("Missing part parameter");
				}
				auto const part = args[0];
				args = args.subspan(1);

				auto const captcha = input(args);
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

		namespace day2 {
			using row_t = std::vector<int>;
			using input_t = std::vector<row_t>;
			using input_view_t = gsl::span<row_t const>;
			
			auto do_input(std::string_view arg) -> expected<input_t> {
				auto input = input_t();
				auto row = row_t();

				while (arg.size() > 0) {
					auto const itSeparator = std::find_if(arg.begin(), arg.end(), [](char c) { 
						return c == '\t' || c == '\n'; 
					});
					if (itSeparator == arg.end()) {
						if (row.size() > 0) {
							input.push_back(std::move(row));
						}
						return input;
					}
					else if (*itSeparator == '\t') {
						auto const token_size = static_cast<std::string_view::size_type>(std::distance(arg.begin(), itSeparator));
						auto const token_value = to_int(arg.substr(token_size));
						if (!token_value) {
							return expected<input_t>(unexpect, token_value.error());
						}
						row.push_back(token_value.value());
						arg = arg.substr(token_size + 1);
					}
					else if (*itSeparator == '\n') {
						auto const token_size = static_cast<std::string_view::size_type>(std::distance(arg.begin(), itSeparator));
						auto const token_value = to_int(arg.substr(token_size));
						if (!token_value) {
							return expected<input_t>(unexpect, token_value.error());
						}
						row.push_back(token_value.value());
						input.push_back(std::move(row));
						row.clear();
						arg = arg.substr(token_size + 1);
					}
				}
				return input;
			}

			auto input(gsl::span<std::string_view const> args) -> expected<input_t> {	
				if (args.size() == 0) {
					return expected<input_t>{ unexpect,
						error_info(std::make_error_code(std::errc::invalid_argument), "No stdin support in this puzzle") };
				}
				else if (args[0] == "--input") {
					if (args.size() < 2) {
						return expected<input_t>{ unexpect,
							error_info(std::make_error_code(std::errc::invalid_argument), "Missing input after --input") };
					}

					return do_input(args[1]);									
				}
				else if (args[0] == "--file") {
					if (args.size() < 2) {
						return expected<input_t>{ unexpect,
							error_info(std::make_error_code(std::errc::invalid_argument), "Missing filename after --input") };
					}
					
					auto const filepath = args[1];
					auto file = std::ifstream(std::string(filepath));
					if (!file) {
						return expected<input_t>{ unexpect,
							error_info(std::make_error_code(std::errc::invalid_argument), "File \""s.append(filepath).append("\" could not be opened")) };
					}

					auto arg = std::string();
					auto line = std::string();
					while (std::getline(file, line)) {
						arg.append(std::move(line)).append("\n");
					}

					return do_input(arg);
				}
				else {
					return expected<input_t>{ unexpect,
						error_info(std::make_error_code(std::errc::invalid_argument), "Invalid parameter \""s.append(args[0]).append("\"")) };
				}
			}

			auto part1(input_view_t matrix) -> int {
				auto differences = std::vector<int>();
				std::transform( matrix.begin(), matrix.end(), std::back_inserter(differences),
					[](row_t const& row) -> int {
					auto diff = [](auto p) { return *p.second - *p.first; };
					return diff(std::minmax_element(row.begin(), row.end()));
				});

				return std::accumulate(differences.begin(), differences.end(), 0);
			}

			auto part2(input_view_t matrix) -> int {
				auto dividends = std::vector<int>();
				std::transform(matrix.begin(), matrix.end(), std::back_inserter(dividends),
					[](row_t const& row) -> int {
					auto const endRow = row.data() + row.size();
					for (int const& element : row) {
						auto const itFound = std::find_if(&element + 1, endRow, [element] (const int n) {
							return element % n == 0 || n % element == 0;
						} );
						
						if (itFound != endRow) {
							auto const n = *itFound;
							assert(element != n);
							return std::max(element, *itFound) / std::min(element, *itFound);
						}
					}
					throw std::runtime_error("one row had no numbers evenly divisible");
				});

				return std::accumulate(dividends.begin(), dividends.end(), 0);
			}

			auto solve(gsl::span<std::string_view const> args) -> int {
				if (args.size() < 1) {
					throw std::runtime_error("Missing part parameter");
				}
				auto const part = args[0];
				args = args.subspan(1);

				auto const matrix = input(args);
				if (!matrix) {
					std::cerr << matrix.error() << "\n";
					return EXIT_FAILURE;
				}

				if (part == "1") {
					std::cout << part1(matrix.value()) << "\n";
					return EXIT_SUCCESS;
				}
				else if (part == "2") {
					std::cout << part2(matrix.value()) << "\n";
					return EXIT_SUCCESS;
				}
				else {
					throw std::runtime_error{ "Parameter \""s.append(part).append("\" was not a valid part (try 1 or 2)") };
				}
			}
		}

		namespace day3 {
			using input_t = int;

			auto input(gsl::span<std::string_view const> args) -> expected<input_t> {
				if (args.size() == 0) {
					auto input = input_t();
					if (!std::cin >> input) {
						return expected<input_t>{ unexpect,
							error_info(std::make_error_code(std::errc::invalid_argument), "Could not parse input to an integer") };
					}

					return input;
				}
				else if (args[0] == "--input") {
					if (args.size() < 2) {
						return expected<input_t>{ unexpect,
							error_info(std::make_error_code(std::errc::invalid_argument), "Missing input after --input") };
					}
					
					return to_int(args[1]);
				}
				else if (args[0] == "--file") {
					if (args.size() < 2) {
						return expected<input_t>{ unexpect,
							error_info(std::make_error_code(std::errc::invalid_argument), "Missing filename after --input") };
					}

					auto const filepath = args[1];
					auto file = std::ifstream(std::string(filepath));
					if (!file) {
						return expected<input_t>{ unexpect,
							error_info(std::make_error_code(std::errc::invalid_argument), "File \""s.append(filepath).append("\" could not be opened")) };
					}

					auto input = input_t();
					if (!(file >> input)) {
						return expected<input_t>{ unexpect,
							error_info(std::make_error_code(std::errc::invalid_argument), "Could not parse input to an integer") };
					}

					return input;
				}
				else {
					return expected<input_t>{ unexpect,
						error_info(std::make_error_code(std::errc::invalid_argument), "Invalid parameter \""s.append(args[0]).append("\"")) };
				}
			}

			auto part1(input_t n) -> int {
				if (n < 1) {
					throw std::runtime_error("Invalid index \"" + std::to_string(n) + "\": must be greater than 0");
				}

				// Algorithm from https://math.stackexchange.com/a/163093
				auto const m = static_cast<input_t>(std::sqrt(n));
				auto const k = [n, m] {
					if (m % 2 == 1) {
						return (m - 1) / 2;
					}
					else if (n >= m*(m + 1)) {
						return m / 2;
					}
					else {
						return (m / 2) - 1;
					}
				}();

				struct vector {
					input_t x, y;
				};

				auto const coords = [k] ( input_t n ) -> vector {
					auto const square = [](input_t a) { return static_cast<input_t>(pow(a,2)); };

					if (k * 2 *(k * 2 + 1) < n && n <= square(k * 2 + 1)) {
						return { n - 4 * square(k) - 3 * k, k };
					}
					else if (square(2 * k + 1) < n && n <= 2 * (k + 1)*(2*k + 1)) {
						return { k + 1, 4 * square(k) + 5 * k + 1 - n };
					}
					else if (2 * (k + 1)*(2*k + 1) < n && n <= 4 * square(k + 1)) {
						return { 4 * square(k) + 7 * k + 3 - n, -k - 1 };
					}
					else if (4 * square(k + 1) < n && n <= 2 * (k + 1)*(2 * k + 3)) {
						return { -k - 1, n - 4 * square(k) - 9 * k - 5 };
					}
					else {
						assert(false);
						UNREACHABLE();
					}
				};

				auto const pos = coords(n - 1); // Make the grid start at 0
				return std::abs(pos.x) + std::abs(pos.y);
			}

			auto part2(input_t input) -> int {
				(void)input;
				throw std::runtime_error("Part2 not implemented");
			}

			auto solve(gsl::span<std::string_view const> args) -> int {
				if (args.size() < 1) {
					throw std::runtime_error("Missing part parameter");
				}
				auto const part = args[0];
				args = args.subspan(1);

				auto const in = input(args);
				if (!in) {
					std::cerr << in.error() << "\n";
					return EXIT_FAILURE;
				}

				if (part == "1") {
					std::cout << part1(in.value()) << "\n";
					return EXIT_SUCCESS;
				}
				else if (part == "2") {
					std::cout << part2(in.value()) << "\n";
					return EXIT_SUCCESS;
				}
				else {
					throw std::runtime_error{ "Parameter \""s.append(part).append("\" was not a valid part (try 1 or 2)") };
				}
			}
		}

		namespace day4 {
			auto input(gsl::span<std::string_view const> args) -> expected<std::string> {
				if (args.size() == 0) {
					auto input = std::string();
					if (!std::getline(std::cin, input)) {
						return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Could not parse input to an integer"));
					}

					return input;
				}
				else if (args[0] == "--input") {
					if (args.size() < 2) {
						return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Missing input after --input"));
					}

					return std::string(args[1]);
				}
				else if (args[0] == "--file") {
					if (args.size() < 2) {
						return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Missing filename after --input"));
					}

					auto const filepath = args[1];
					auto file = std::ifstream(std::string(filepath));
					if (!file) {
						return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "File \""s.append(filepath).append("\" could not be opened")));
					}

					auto input = std::string();
					auto line = std::string();
					while(std::getline(file, line)) {
						if (!line.empty()) {
							input.append(line).append("\n");
						}
					}

					return input;
				}
				else {
					return make_unexpected(
						error_info(std::make_error_code(std::errc::invalid_argument), "Invalid parameter \""s.append(args[0]).append("\""))
					);
				}
			}

			auto part1(std::string_view s) -> int {
				auto validLineCount = 0;
				for (auto lineEnd = std::find(s.begin(), s.end(), '\n')
					; !s.empty() && lineEnd != s.end()
					; s.remove_prefix(std::distance(s.begin(), lineEnd) + 1)
					, lineEnd = std::find(s.begin(), s.end(), '\n')) {
					auto line = std::string_view(s.data(), std::distance(s.begin(), lineEnd));
					auto foundTokens = std::set<std::string_view>();
					for (auto tokenEnd = std::find(line.begin(), line.end(), ' ')
						; !line.empty()
						; (tokenEnd != line.end()) ? line.remove_prefix(std::distance(line.begin(), tokenEnd) + 1) : line = std::string_view()
						, tokenEnd = std::find(line.begin(), line.end(), ' ')) {
						auto const token = std::string_view(line.data(), std::distance(line.begin(), tokenEnd));
						if (foundTokens.count(token) == 1) {
							break;
						}
						foundTokens.insert(token);
					}
					if (line.empty()) {
						++validLineCount;
					}
				}

				return validLineCount;
			}

			auto part2(std::string_view s) -> int {
				auto validLineCount = 0;
				for (auto lineEnd = std::find(s.begin(), s.end(), '\n')
					; !s.empty() && lineEnd != s.end()
					; s.remove_prefix(std::distance(s.begin(), lineEnd) + 1)
					, lineEnd = std::find(s.begin(), s.end(), '\n')) {
					auto line = std::string_view(s.data(), std::distance(s.begin(), lineEnd));
					auto foundTokens = std::vector<std::string_view>();
					for (auto tokenEnd = std::find(line.begin(), line.end(), ' ')
						; !line.empty()
						; (tokenEnd != line.end()) ? line.remove_prefix(std::distance(line.begin(), tokenEnd) + 1) : line = std::string_view()
						, tokenEnd = std::find(line.begin(), line.end(), ' ')) {
						auto const token = std::string_view(line.data(), std::distance(line.begin(), tokenEnd));
						if (std::any_of(foundTokens.begin(), foundTokens.end(), [token](std::string_view const foundToken) {
							return std::is_permutation(foundToken.begin(), foundToken.end(), token.begin(), token.end());
						})) {
							break;
						}
						foundTokens.push_back(token);
					}
					if (line.empty()) {
						++validLineCount;
					}
				}

				return validLineCount;
			}

			auto solve(gsl::span<std::string_view const> args) -> int {
				if (args.size() < 1) {
					throw std::runtime_error("Missing part parameter");
				}
				auto const part = args[0];
				args = args.subspan(1);

				auto const in = input(args);
				if (!in) {
					std::cerr << in.error() << "\n";
					return EXIT_FAILURE;
				}

				if (part == "1") {
					std::cout << part1(in.value()) << "\n";
					return EXIT_SUCCESS;
				}
				else if (part == "2") {
					std::cout << part2(in.value()) << "\n";
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
		else if (day == "2") {
			return day2::solve(args);
		}
		else if (day == "3") {
			return day3::solve(args);
		}
		else if (day == "4") {
			return day4::solve(args);
		}
		else {
			throw std::runtime_error{ "Parameter \""s.append(day).append("\" was not a valid day (try 1-25)") };
		}
	}
}