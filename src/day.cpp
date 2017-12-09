#include <gsl/span>
#include <expected.hpp>
#include <numeric>
#include <cctype>
#include <cassert>
#include <cmath>
#include <set>
#include <regex>
#include <variant>
#include <map>

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
                if(args.size() == 0) {
                    if(!std::getline(std::cin, input)) {
                        return expected<std::string>{ unexpect,
                            error_info(std::make_error_code(std::errc::invalid_argument), "Could not read input") };
                    }
                } else if(args[0] == "--input") {
                    if(args.size() < 2) {
                        return expected<std::string>{ unexpect,
                            error_info(std::make_error_code(std::errc::invalid_argument), "Missing input after --input") };
                    }
                    input = args[1];
                } else {
                    return expected<std::string>{ unexpect,
                        error_info(std::make_error_code(std::errc::invalid_argument), "Invalid parameter \""s.append(args[0]).append("\"")) };
                }
                if(!std::all_of(input.begin(), input.end(),
                                [] (const char c) { return std::isdigit(static_cast<unsigned char>(c)); })) {
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
                    for(auto i = char_pair_arr::size_type{0}; i < captcha.size(); ++i) {
                        characters.emplace_back(char_pair{captcha[i], captcha[(i + steps) % captcha.size()]});
                    }
                    return characters;
                }();

                const auto adjacentCharacters = filter(characters, [] (const char_pair c) { return c.c1 == c.c2; });
                const auto sum = std::accumulate(adjacentCharacters.begin(), adjacentCharacters.end(), 0,
                                                 [] (const int counter, const char_pair c) { return counter + static_cast<int>(c.c1 - '0'); });

                return sum;
            }

            auto part1(std::string_view captcha) -> int {
                return part(captcha, 1);
            }

            auto part2(std::string_view captcha) -> int {
                return part(captcha, gsl::narrow<int>(captcha.size() / 2));
            }

            auto solve(gsl::span<std::string_view const> args) -> int {
                if(args.size() < 1) {
                    throw std::runtime_error("Missing part parameter");
                }
                auto const part = args[0];
                args = args.subspan(1);

                auto const captcha = input(args);
                if(!captcha) {
                    std::cerr << captcha.error() << "\n";
                    return EXIT_FAILURE;
                }

                if(part == "1") {
                    std::cout << part1(captcha.value()) << "\n";
                    return EXIT_SUCCESS;
                } else if(part == "2") {
                    std::cout << part2(captcha.value()) << "\n";
                    return EXIT_SUCCESS;
                } else {
                    throw std::runtime_error{"Parameter \""s.append(part).append("\" was not a valid part (try 1 or 2)")};
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

                while(arg.size() > 0) {
                    auto const itSeparator = std::find_if(arg.begin(), arg.end(), [] (char c) {
                        return c == '\t' || c == '\n';
                    });
                    if(itSeparator == arg.end()) {
                        if(row.size() > 0) {
                            input.push_back(std::move(row));
                        }
                        return input;
                    } else if(*itSeparator == '\t') {
                        auto const token_size = static_cast<std::string_view::size_type>(std::distance(arg.begin(), itSeparator));
                        auto const token_value = to_int(arg.substr(token_size));
                        if(!token_value) {
                            return expected<input_t>(unexpect, token_value.error());
                        }
                        row.push_back(token_value.value().data);
                        arg = arg.substr(token_size + 1);
                    } else if(*itSeparator == '\n') {
                        auto const token_size = static_cast<std::string_view::size_type>(std::distance(arg.begin(), itSeparator));
                        auto const token_value = to_int(arg.substr(token_size));
                        if(!token_value) {
                            return expected<input_t>(unexpect, token_value.error());
                        }
                        row.push_back(token_value.value().data);
                        input.push_back(std::move(row));
                        row.clear();
                        arg = arg.substr(token_size + 1);
                    }
                }
                return input;
            }

            auto input(gsl::span<std::string_view const> args) -> expected<input_t> {
                if(args.size() == 0) {
                    return expected<input_t>{ unexpect,
                        error_info(std::make_error_code(std::errc::invalid_argument), "No stdin support in this puzzle") };
                } else if(args[0] == "--input") {
                    if(args.size() < 2) {
                        return expected<input_t>{ unexpect,
                            error_info(std::make_error_code(std::errc::invalid_argument), "Missing input after --input") };
                    }

                    return do_input(args[1]);
                } else if(args[0] == "--file") {
                    if(args.size() < 2) {
                        return expected<input_t>{ unexpect,
                            error_info(std::make_error_code(std::errc::invalid_argument), "Missing filename after --input") };
                    }

                    auto const filepath = args[1];
                    auto file = std::ifstream(std::string(filepath));
                    if(!file) {
                        return expected<input_t>{ unexpect,
                            error_info(std::make_error_code(std::errc::invalid_argument), "File \""s.append(filepath).append("\" could not be opened")) };
                    }

                    auto arg = std::string();
                    auto line = std::string();
                    while(std::getline(file, line)) {
                        arg.append(std::move(line)).append("\n");
                    }

                    return do_input(arg);
                } else {
                    return expected<input_t>{ unexpect,
                        error_info(std::make_error_code(std::errc::invalid_argument), "Invalid parameter \""s.append(args[0]).append("\"")) };
                }
            }

            auto part1(input_view_t matrix) -> int {
                auto differences = std::vector<int>();
                std::transform(matrix.begin(), matrix.end(), std::back_inserter(differences),
                               [] (row_t const& row) -> int {
                    auto diff = [] (auto p) { return *p.second - *p.first; };
                    return diff(std::minmax_element(row.begin(), row.end()));
                });

                return std::accumulate(differences.begin(), differences.end(), 0);
            }

            auto part2(input_view_t matrix) -> int {
                auto dividends = std::vector<int>();
                std::transform(matrix.begin(), matrix.end(), std::back_inserter(dividends),
                               [] (row_t const& row) -> int {
                    auto const endRow = row.data() + row.size();
                    for(int const& element : row) {
                        auto const itFound = std::find_if(&element + 1, endRow, [element] (const int n) {
                            return element % n == 0 || n % element == 0;
                        });

                        if(itFound != endRow) {
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
                if(args.size() < 1) {
                    throw std::runtime_error("Missing part parameter");
                }
                auto const part = args[0];
                args = args.subspan(1);

                auto const matrix = input(args);
                if(!matrix) {
                    std::cerr << matrix.error() << "\n";
                    return EXIT_FAILURE;
                }

                if(part == "1") {
                    std::cout << part1(matrix.value()) << "\n";
                    return EXIT_SUCCESS;
                } else if(part == "2") {
                    std::cout << part2(matrix.value()) << "\n";
                    return EXIT_SUCCESS;
                } else {
                    throw std::runtime_error{"Parameter \""s.append(part).append("\" was not a valid part (try 1 or 2)")};
                }
            }
        }

        namespace day3 {
            using input_t = int;

            auto input(gsl::span<std::string_view const> args) -> expected<input_t> {
                if(args.size() == 0) {
                    auto input = input_t();
                    if(!std::cin >> input) {
                        return expected<input_t>{ unexpect,
                            error_info(std::make_error_code(std::errc::invalid_argument), "Could not parse input to an integer") };
                    }

                    return input;
                } else if(args[0] == "--input") {
                    if(args.size() < 2) {
                        return expected<input_t>{ unexpect,
                            error_info(std::make_error_code(std::errc::invalid_argument), "Missing input after --input") };
                    }

                    return to_int(args[1]).map(&conversion_result<int>::data);
                } else if(args[0] == "--file") {
                    if(args.size() < 2) {
                        return expected<input_t>{ unexpect,
                            error_info(std::make_error_code(std::errc::invalid_argument), "Missing filename after --input") };
                    }

                    auto const filepath = args[1];
                    auto file = std::ifstream(std::string(filepath));
                    if(!file) {
                        return expected<input_t>{ unexpect,
                            error_info(std::make_error_code(std::errc::invalid_argument), "File \""s.append(filepath).append("\" could not be opened")) };
                    }

                    auto input = input_t();
                    if(!(file >> input)) {
                        return expected<input_t>{ unexpect,
                            error_info(std::make_error_code(std::errc::invalid_argument), "Could not parse input to an integer") };
                    }

                    return input;
                } else {
                    return expected<input_t>{ unexpect,
                        error_info(std::make_error_code(std::errc::invalid_argument), "Invalid parameter \""s.append(args[0]).append("\"")) };
                }
            }

            auto part1(input_t n) -> int {
                if(n < 1) {
                    throw std::runtime_error("Invalid index \"" + std::to_string(n) + "\": must be greater than 0");
                }

                // Algorithm from https://math.stackexchange.com/a/163093
                auto const m = static_cast<input_t>(std::sqrt(n));
                auto const k = [n, m] {
                    if(m % 2 == 1) {
                        return (m - 1) / 2;
                    } else if(n >= m * (m + 1)) {
                        return m / 2;
                    } else {
                        return (m / 2) - 1;
                    }
                }();

                struct vector {
                    input_t x, y;
                };

                auto const coords = [k] (input_t n) -> vector {
                    auto const square = [] (input_t a) { return static_cast<input_t>(pow(a, 2)); };

                    if(k * 2 * (k * 2 + 1) < n && n <= square(k * 2 + 1)) {
                        return {n - 4 * square(k) - 3 * k, k};
                    } else if(square(2 * k + 1) < n && n <= 2 * (k + 1)*(2 * k + 1)) {
                        return {k + 1, 4 * square(k) + 5 * k + 1 - n};
                    } else if(2 * (k + 1)*(2 * k + 1) < n && n <= 4 * square(k + 1)) {
                        return {4 * square(k) + 7 * k + 3 - n, -k - 1};
                    } else if(4 * square(k + 1) < n && n <= 2 * (k + 1)*(2 * k + 3)) {
                        return {-k - 1, n - 4 * square(k) - 9 * k - 5};
                    } else {
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
                if(args.size() < 1) {
                    throw std::runtime_error("Missing part parameter");
                }
                auto const part = args[0];
                args = args.subspan(1);

                auto const in = input(args);
                if(!in) {
                    std::cerr << in.error() << "\n";
                    return EXIT_FAILURE;
                }

                if(part == "1") {
                    std::cout << part1(in.value()) << "\n";
                    return EXIT_SUCCESS;
                } else if(part == "2") {
                    std::cout << part2(in.value()) << "\n";
                    return EXIT_SUCCESS;
                } else {
                    throw std::runtime_error{"Parameter \""s.append(part).append("\" was not a valid part (try 1 or 2)")};
                }
            }
        }

        namespace day4 {
            auto input(gsl::span<std::string_view const> args) -> expected<std::string> {
                if(args.size() == 0) {
                    auto input = std::string();
                    if(!std::getline(std::cin, input)) {
                        return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Could not parse input to an integer"));
                    }

                    return input;
                } else if(args[0] == "--input") {
                    if(args.size() < 2) {
                        return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Missing input after --input"));
                    }

                    return std::string(args[1]);
                } else if(args[0] == "--file") {
                    if(args.size() < 2) {
                        return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Missing filename after --input"));
                    }

                    auto const filepath = args[1];
                    auto file = std::ifstream(std::string(filepath));
                    if(!file) {
                        return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "File \""s.append(filepath).append("\" could not be opened")));
                    }

                    auto input = std::string();
                    auto line = std::string();
                    while(std::getline(file, line)) {
                        if(!line.empty()) {
                            input.append(line).append("\n");
                        }
                    }

                    return input;
                } else {
                    return make_unexpected(
                        error_info(std::make_error_code(std::errc::invalid_argument), "Invalid parameter \""s.append(args[0]).append("\""))
                    );
                }
            }

            auto part1(std::string_view s) -> int {
                auto validLineCount = 0;
                for(auto lineEnd = std::find(s.begin(), s.end(), '\n')
                    ; !s.empty() && lineEnd != s.end()
                    ; s.remove_prefix(std::distance(s.begin(), lineEnd) + 1)
                    , lineEnd = std::find(s.begin(), s.end(), '\n')) {
                    auto line = std::string_view(s.data(), std::distance(s.begin(), lineEnd));
                    auto foundTokens = std::set<std::string_view>();
                    for(auto tokenEnd = std::find(line.begin(), line.end(), ' ')
                        ; !line.empty()
                        ; (tokenEnd != line.end()) ? line.remove_prefix(std::distance(line.begin(), tokenEnd) + 1) : line = std::string_view()
                        , tokenEnd = std::find(line.begin(), line.end(), ' ')) {
                        auto const token = std::string_view(line.data(), std::distance(line.begin(), tokenEnd));
                        if(foundTokens.count(token) == 1) {
                            break;
                        }
                        foundTokens.insert(token);
                    }
                    if(line.empty()) {
                        ++validLineCount;
                    }
                }

                return validLineCount;
            }

            auto part2(std::string_view s) -> int {
                auto validLineCount = 0;
                for(auto lineEnd = std::find(s.begin(), s.end(), '\n')
                    ; !s.empty() && lineEnd != s.end()
                    ; s.remove_prefix(std::distance(s.begin(), lineEnd) + 1)
                    , lineEnd = std::find(s.begin(), s.end(), '\n')) {
                    auto line = std::string_view(s.data(), std::distance(s.begin(), lineEnd));
                    auto foundTokens = std::vector<std::string_view>();
                    for(auto tokenEnd = std::find(line.begin(), line.end(), ' ')
                        ; !line.empty()
                        ; (tokenEnd != line.end()) ? line.remove_prefix(std::distance(line.begin(), tokenEnd) + 1) : line = std::string_view()
                        , tokenEnd = std::find(line.begin(), line.end(), ' ')) {
                        auto const token = std::string_view(line.data(), std::distance(line.begin(), tokenEnd));
                        if(std::any_of(foundTokens.begin(), foundTokens.end(), [token] (std::string_view const foundToken) {
                            return std::is_permutation(foundToken.begin(), foundToken.end(), token.begin(), token.end());
                        })) {
                            break;
                        }
                        foundTokens.push_back(token);
                    }
                    if(line.empty()) {
                        ++validLineCount;
                    }
                }

                return validLineCount;
            }

            auto solve(gsl::span<std::string_view const> args) -> int {
                if(args.size() < 1) {
                    throw std::runtime_error("Missing part parameter");
                }
                auto const part = args[0];
                args = args.subspan(1);

                auto const in = input(args);
                if(!in) {
                    std::cerr << in.error() << "\n";
                    return EXIT_FAILURE;
                }

                if(part == "1") {
                    std::cout << part1(in.value()) << "\n";
                    return EXIT_SUCCESS;
                } else if(part == "2") {
                    std::cout << part2(in.value()) << "\n";
                    return EXIT_SUCCESS;
                } else {
                    throw std::runtime_error{"Parameter \""s.append(part).append("\" was not a valid part (try 1 or 2)")};
                }
            }
        }

        namespace day5 {
            auto input_impl(std::string_view arg) -> expected<std::vector<int>> {
                auto input = std::vector<int>();
                while(auto const convert_result = to_int(arg)) {
                    auto const& convert_value = convert_result.value();
                    input.push_back(convert_value.data);
                    arg = arg.substr(std::distance(arg.data(), convert_value.conversion_end));
                }
                return input;
            }

            auto input(gsl::span<std::string_view const> args) -> expected<std::vector<int>> {
                if(args.size() == 0) {
                    auto line = std::string();
                    if(!std::getline(std::cin, line)) {
                        return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Could not parse input to an integer"));
                    }

                    return input_impl(line);
                } else if(args[0] == "--input") {
                    if(args.size() < 2) {
                        return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Missing input after --input"));
                    }

                    return input_impl(args[1]);
                } else if(args[0] == "--file") {
                    if(args.size() < 2) {
                        return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Missing filename after --input"));
                    }

                    auto const filepath = args[1];
                    auto file = std::ifstream(std::string(filepath));
                    if(!file) {
                        return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "File \""s.append(filepath).append("\" could not be opened")));
                    }

                    auto input = std::string();
                    auto line = std::string();
                    while(std::getline(file, line)) {
                        if(!line.empty()) {
                            input.append(line).append("\n");
                        }
                    }

                    return input_impl(input);
                } else {
                    return make_unexpected(
                        error_info(std::make_error_code(std::errc::invalid_argument), "Invalid parameter \""s.append(args[0]).append("\""))
                    );
                }
            }

            auto part1(gsl::span<int const> input) -> int {
                auto maze = std::vector<int>(input.begin(), input.end());
                auto maze_position = maze.begin();
                auto step_count = 0;
                while(maze_position != maze.end()) {
                    auto & current_value = *maze_position;
                    auto const distance_from_end = std::distance(maze_position, maze.end());
                    ++step_count;
                    if(current_value >= distance_from_end) {
                        maze_position = maze.end();
                        continue;
                    }

                    maze_position += current_value;
                    ++current_value;
                }
                return step_count;
            }

            auto part2(gsl::span<int const> input) -> int {
                auto maze = std::vector<int>(input.begin(), input.end());
                auto maze_position = maze.begin();
                auto step_count = 0;
                while(maze_position != maze.end()) {
                    auto & current_value = *maze_position;
                    auto const distance_from_end = std::distance(maze_position, maze.end());
                    ++step_count;
                    if(current_value >= distance_from_end) {
                        maze_position = maze.end();
                        continue;
                    }

                    maze_position += current_value;
                    if(current_value >= 3) {
                        --current_value;
                    } else {
                        ++current_value;
                    }

                }
                return step_count;
            }

            auto solve(gsl::span<std::string_view const> args) -> int {
                if(args.size() < 1) {
                    throw std::runtime_error("Missing part parameter");
                }
                auto const part = args[0];
                args = args.subspan(1);

                auto const in = input(args);
                if(!in) {
                    std::cerr << in.error() << "\n";
                    return EXIT_FAILURE;
                }

                if(part == "1") {
                    std::cout << part1(in.value()) << "\n";
                    return EXIT_SUCCESS;
                } else if(part == "2") {
                    std::cout << part2(in.value()) << "\n";
                    return EXIT_SUCCESS;
                } else {
                    throw std::runtime_error{"Parameter \""s.append(part).append("\" was not a valid part (try 1 or 2)")};
                }
            }
        }

        namespace day6 {
            auto input_impl(std::string_view arg) -> expected<std::vector<int>> {
                auto input = std::vector<int>();
                while(auto const convert_result = to_int(arg)) {
                    auto const& convert_value = convert_result.value();
                    input.push_back(convert_value.data);
                    arg = arg.substr(std::distance(arg.data(), convert_value.conversion_end));
                }
                return input;
            }

            auto input(gsl::span< std::string_view const > args) -> expected<std::vector<int>> {
                if(args.size() == 0) {
                    auto line = std::string();
                    if(!std::getline(std::cin, line)) {
                        return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Could not parse input to an integer"));
                    }

                    return input_impl(line);
                } else if(args[0] == "--input") {
                    if(args.size() < 2) {
                        return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Missing input after --input"));
                    }

                    return input_impl(args[1]);
                } else if(args[0] == "--file") {
                    if(args.size() < 2) {
                        return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Missing filename after --input"));
                    }

                    auto const filepath = args[1];
                    auto file = std::ifstream(std::string(filepath));
                    if(!file) {
                        return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "File \""s.append(filepath).append("\" could not be opened")));
                    }

                    auto input = std::string();
                    auto line = std::string();
                    while(std::getline(file, line)) {
                        if(!line.empty()) {
                            input.append(line).append("\n");
                        }
                    }

                    return input_impl(input);
                } else {
                    return make_unexpected(
                        error_info(std::make_error_code(std::errc::invalid_argument), "Invalid parameter \""s.append(args[0]).append("\""))
                    );
                }
            }

            struct redistribution_sentinel {

            };

            class redistribution_iterator {
            public:
                using value_type = std::vector<int>::value_type;
                using difference_type = std::vector<int>::difference_type;
                using reference = std::vector<int>::reference;
                using pointer = std::vector<int>::pointer;
                using iterator_category = std::forward_iterator_tag;

                redistribution_iterator(std::vector<int> & vec, std::vector<int>::iterator first_bank, int redistribution_count)
                    : vec(&vec)
                    , current_element(first_bank)
                    , redistribution_count(redistribution_count) {
                    assert(redistribution_count >= 0);
                }

                auto operator*() const -> reference {
                    return *current_element;
                }

                auto operator++() -> redistribution_iterator & {
                    ++current_element;
                    if(current_element == vec->end()) {
                        current_element = vec->begin();
                    }
                    --redistribution_count;
                    return *this;
                }

                auto operator!=(redistribution_sentinel) const {
                    return redistribution_count != 0;
                }

                auto begin() const noexcept -> redistribution_iterator {
                    return *this;
                }

                auto end() const noexcept -> redistribution_sentinel {
                    return {};
                }

            private:
                std::vector<int>* vec;
                std::vector<int>::iterator current_element;
                int redistribution_count;
            };

            auto part1(gsl::span<int const> input) -> int {
                auto banks = std::vector<int>(input.begin(), input.end());
                auto states = std::vector<std::vector<int>>();
                auto const has_state = [&states] (std::vector<int> const& expected_state) {
                    return std::find(states.begin(), states.end(), expected_state) != states.end();
                };

                auto cycle_count = 0;
                while(!has_state(banks)) {
                    states.push_back(banks);
                    auto const max_bank = std::max_element(banks.begin(), banks.end());
                    auto const redistribution_count = *max_bank;
                    *max_bank = 0;
                    auto const next_bank = (max_bank + 1 != banks.end()) ? max_bank + 1 : banks.begin();
                    auto redistribution = redistribution_iterator(banks, next_bank, redistribution_count);
                    for(int & element : redistribution) {
                        ++element;
                    }
                    ++cycle_count;
                }

                return cycle_count;
            }

            auto part2(gsl::span<int const> input) -> int {
                auto banks = std::vector<int>(input.begin(), input.end());
                auto states = std::vector<std::pair<std::vector<int>, int>>();
                enum { state_index, cycle_index };
                auto const find_state = [&states] (std::vector<int> const& expected_state) {
                    return std::find_if(states.begin(), states.end(), [expected_state] (auto const& current_state) -> bool {
                        return std::get<state_index>(current_state) == expected_state;
                    });
                };

                auto cycle_count = 0;
                auto found_state = decltype(states)::iterator();
                while((found_state = find_state(banks)) == states.end()) {
                    states.emplace_back(banks, cycle_count);
                    auto const max_bank = std::max_element(banks.begin(), banks.end());
                    auto const redistribution_count = *max_bank;
                    *max_bank = 0;
                    auto const next_bank = (max_bank + 1 != banks.end()) ? max_bank + 1 : banks.begin();
                    auto redistribution = redistribution_iterator(banks, next_bank, redistribution_count);
                    for(int & element : redistribution) {
                        ++element;
                    }
                    ++cycle_count;
                }

                return cycle_count - std::get<cycle_index>(*found_state);
            }

            auto solve(gsl::span<std::string_view const> args) -> int {
                if(args.size() < 1) {
                    throw std::runtime_error("Missing part parameter");
                }
                auto const part = args[0];
                args = args.subspan(1);

                auto const in = input(args);
                if(!in) {
                    std::cerr << in.error() << "\n";
                    return EXIT_FAILURE;
                }

                if(part == "1") {
                    std::cout << part1(in.value()) << "\n";
                    return EXIT_SUCCESS;
                } else if(part == "2") {
                    std::cout << part2(in.value()) << "\n";
                    return EXIT_SUCCESS;
                } else {
                    throw std::runtime_error{"Parameter \""s.append(part).append("\" was not a valid part (try 1 or 2)")};
                }
            }
        }

        namespace day7 {
            struct tower {
                std::string name;
                int weight;
                std::vector<std::string> dependencies;
            };

            using input_t = std::vector<tower>;

            auto to_tower(std::string_view line) -> expected<tower> {
                std::cmatch m;
                if(!std::regex_match(line.data(), line.data() + line.size(), m, std::regex(R"(([a-z]+) \(([0-9]+)\)( -> (.*))?)"))) {
                    return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "\""s.append(line).append("\" did not match the expected format")));
                }
                tower t;
                t.name = m[1];
                t.weight = std::stoi(m[2]);
                if(m[4].matched) {
                    auto const child_list = m[4].str();
                    auto const child_pattern = std::regex(R"(([[:lower:]]+)(, )?)");
                    auto child_iterator = std::sregex_iterator(child_list.begin(), child_list.end(), child_pattern);
                    auto const child_sentinel = std::sregex_iterator();
                    std::transform(child_iterator, child_sentinel, std::back_inserter(t.dependencies), [] (std::smatch const& s) {
                        return s[1];
                    });
                }
                return t;
            }

            auto input(gsl::span<std::string_view const> args) -> expected<input_t> {
                if(args.size() == 0) {
                    return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Stdin input not supported for this day"));
                } else if(args[0] == "--input") {
                    return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "--input not supported for this day"));
                } else if(args[0] == "--file") {
                    if(args.size() < 2) {
                        return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Missing filename after --input"));
                    }

                    auto const filepath = args[1];
                    auto file = std::ifstream(std::string(filepath));
                    if(!file) {
                        return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "File \""s.append(filepath).append("\" could not be opened")));
                    }

                    auto input = input_t();
                    auto line = std::string();
                    while(std::getline(file, line)) {
                        if(!line.empty()) {
                            auto const tower = to_tower(line);
                            if(!tower) {
                                return make_unexpected(tower.error());
                            }
                            input.push_back(std::move(tower).value());
                        }
                    }

                    return input;
                } else {
                    return make_unexpected(
                        error_info(std::make_error_code(std::errc::invalid_argument), "Invalid parameter \""s.append(args[0]).append("\""))
                    );
                }
            }

            auto part1(input_t input) -> std::string {
                auto const find_top = [&input]
                (std::string_view name) {
                    return std::find_if(input.begin(), input.end(), [name] (tower const& t) { return std::find(t.dependencies.begin(), t.dependencies.end(), name) != t.dependencies.end(); });
                };

                auto found_bottom_tower = std::find_if(input.begin(), input.end(), [] (tower const& t) { return t.dependencies.size() > 0; });
                assert(found_bottom_tower != input.end());
                while(true) {
                    auto const& current_tower = *found_bottom_tower;
                    found_bottom_tower = find_top(current_tower.name);
                    if(found_bottom_tower == input.end()) {
                        return current_tower.name;
                    }
                }
            }

            auto part2(input_t) -> int {
                throw std::runtime_error("Not implemented");
            }

            auto solve(gsl::span<std::string_view const> args) -> int {
                if(args.size() < 1) {
                    throw std::runtime_error("Missing part parameter");
                }
                auto const part = args[0];
                args = args.subspan(1);

                auto const in = input(args);
                if(!in) {
                    std::cerr << in.error() << "\n";
                    return EXIT_FAILURE;
                }

                if(part == "1") {
                    std::cout << part1(std::move(in).value()) << "\n";
                    return EXIT_SUCCESS;
                } else if(part == "2") {
                    std::cout << part2(std::move(in).value()) << "\n";
                    return EXIT_SUCCESS;
                } else {
                    throw std::runtime_error{"Parameter \""s.append(part).append("\" was not a valid part (try 1 or 2)")};
                }
            }
        }

        namespace day8 {
            enum class arithmetic_operator { inc, dec };
            enum class comparison_operator { equal, not_equal, less, less_equal, greater, greater_equal };

            template<typename T, typename U>
            auto operator_compare(comparison_operator op, T const& lhs, U const& rhs) noexcept -> bool {
                switch(op) {
                    case comparison_operator::equal: return lhs == rhs;
                    case comparison_operator::not_equal: return lhs != rhs;
                    case comparison_operator::less: return lhs < rhs;
                    case comparison_operator::less_equal: return lhs <= rhs;
                    case comparison_operator::greater: return lhs > rhs;
                    case comparison_operator::greater_equal: return lhs >= rhs;
                }
                assert(false && "Invalid operator");
                UNREACHABLE();
            }

            struct comparison_expression {
                std::string register_name;
                comparison_operator comp;
                int value;
            };

            struct instruction {
                std::string register_name;
                arithmetic_operator op;
                int operand;
                comparison_expression expr;
            };

            using input_t = std::vector<instruction>;

            template<typename T>
            struct parsed_value {
                T value;
                std::string_view rest_instruction;
            };

            auto find_end_token(std::string_view line) -> std::string_view::iterator {
                return std::find_if(line.begin(), line.end(), [] (char const c) { return std::isspace(c); });
            }

            auto parse_register(std::string_view instruction) -> expected<parsed_value<std::string>> {
                auto const end_token = find_end_token(instruction);
                auto const token = std::string_view(instruction.data(), std::distance(instruction.begin(), end_token));
                if(token.empty()) {
                    return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Expected register, but was empty token"));
                }

                auto const rest_tokens = end_token != instruction.end() ? instruction.substr(std::distance(instruction.begin(), end_token) + 1) : std::string_view{};
                return parsed_value<std::string>{std::string(token), rest_tokens};
            }

            auto parse_arithmetic_operator(std::string_view instruction) -> expected<parsed_value<arithmetic_operator>> {
                auto const end_token = find_end_token(instruction);
                auto const token = std::string_view(instruction.data(), std::distance(instruction.begin(), end_token));
                if(token.empty()) {
                    return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Expected arithmetic operator, but was empty token"));
                }
                auto const rest_tokens = end_token != instruction.end() ? instruction.substr(std::distance(instruction.begin(), end_token) + 1) : std::string_view{};

                if(token == "inc") {
                    return parsed_value<arithmetic_operator>{arithmetic_operator::inc, rest_tokens};
                } else if(token == "dec") {
                    return parsed_value<arithmetic_operator>{arithmetic_operator::dec, rest_tokens};
                } else {
                    return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Expected arithmetic operator, but was \""s.append(token).append("\"")));
                }
            }

            auto parse_value(std::string_view instruction) -> expected<parsed_value<int>> {
                auto const end_token = find_end_token(instruction);
                auto const token = std::string_view(instruction.data(), std::distance(instruction.begin(), end_token));
                if(token.empty()) {
                    return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Expected value, but was empty token"));
                }
                auto const rest_tokens = end_token != instruction.end() ? instruction.substr(std::distance(instruction.begin(), end_token) + 1) : std::string_view{};

                return to_int(token).map([rest_tokens] (conversion_result<int> const c) -> parsed_value<int> { return {c.data, rest_tokens}; });
            }

            auto parse_comparison_operator(std::string_view line) -> expected<parsed_value<comparison_operator>> {
                auto const end_token = find_end_token(line);
                auto const token = std::string_view(line.data(), std::distance(line.begin(), end_token));
                if(token.empty()) {
                    return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Expected comparison operator, but was empty token"));
                }
                auto const rest_tokens = end_token != line.end() ? line.substr(std::distance(line.begin(), end_token) + 1) : std::string_view{};

                if(token == "==") {
                    return parsed_value<comparison_operator>{comparison_operator::equal, rest_tokens};
                } else if(token == "!=") {
                    return parsed_value<comparison_operator>{comparison_operator::not_equal, rest_tokens};
                } else if(token == "<") {
                    return parsed_value<comparison_operator>{comparison_operator::less, rest_tokens};
                } else if(token == "<=") {
                    return parsed_value<comparison_operator>{comparison_operator::less_equal, rest_tokens};
                } else if(token == ">") {
                    return parsed_value<comparison_operator>{comparison_operator::greater, rest_tokens};
                } else if(token == ">=") {
                    return parsed_value<comparison_operator>{comparison_operator::greater_equal, rest_tokens};
                } else {
                    return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Expected comparison operator, but was \""s.append(token).append("\"")));
                }
            }

            auto parse_comparison(std::string_view line) -> expected<parsed_value<comparison_expression>> {
                auto const register_result = parse_register(line);
                if(!register_result) {
                    return make_unexpected(register_result.error());
                }

                auto const op_result = parse_comparison_operator(register_result.value().rest_instruction);
                if(!op_result) {
                    return make_unexpected(op_result.error());
                }

                auto const value_result = parse_value(op_result.value().rest_instruction);
                if(!value_result) {
                    return make_unexpected(value_result.error());
                }

                return parsed_value<comparison_expression>{
                    comparison_expression{register_result.value().value, op_result.value().value, value_result.value().value},
                        value_result.value().rest_instruction
                };
            }

            auto parse_if(std::string_view line) -> expected<parsed_value<std::monostate>> {
                auto const end_token = find_end_token(line);
                auto const token = std::string_view(line.data(), std::distance(line.begin(), end_token));
                if(token.empty()) {
                    return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Expected \"if\", but was empty token"));
                }
                auto const rest_tokens = end_token != line.end() ? line.substr(std::distance(line.begin(), end_token) + 1) : std::string_view{};

                if(token != "if") {
                    return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Expected \"if\", but was \""s.append(token).append("\"")));
                }

                return parsed_value<std::monostate>{ {}, rest_tokens};
            }

            auto parse_instruction(std::string_view line) -> expected<instruction> {
                auto const register_result = parse_register(line);
                if(!register_result) {
                    return make_unexpected(register_result.error());
                }

                auto const op_result = parse_arithmetic_operator(register_result.value().rest_instruction);
                if(!op_result) {
                    return make_unexpected(op_result.error());
                }

                auto const value_result = parse_value(op_result.value().rest_instruction);
                if(!value_result) {
                    return make_unexpected(value_result.error());
                }

                auto const if_result = parse_if(value_result.value().rest_instruction);
                if(!if_result) {
                    return make_unexpected(if_result.error());
                }

                auto const condition_result = parse_comparison(if_result.value().rest_instruction);
                if(!condition_result) {
                    return make_unexpected(condition_result.error());
                }

                return instruction{register_result.value().value, op_result.value().value, value_result.value().value, condition_result.value().value};
            }

            auto input(gsl::span<std::string_view const> args) -> expected<input_t> {
                if(args.size() == 0) {
                    return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Stdin input not supported for this day"));
                } else if(args[0] == "--input") {
                    return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "--input not supported for this day"));
                } else if(args[0] == "--file") {
                    if(args.size() < 2) {
                        return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "Missing filename after --input"));
                    }

                    auto const filepath = args[1];
                    auto file = std::ifstream(std::string(filepath));
                    if(!file) {
                        return make_unexpected(error_info(std::make_error_code(std::errc::invalid_argument), "File \""s.append(filepath).append("\" could not be opened")));
                    }

                    auto input = input_t();
                    auto line = std::string();
                    while(std::getline(file, line)) {
                        if(!line.empty()) {
                            auto const instruction = parse_instruction(line);
                            if(!instruction) {
                                return make_unexpected(instruction.error());
                            }
                            input.push_back(std::move(instruction).value());
                        }
                    }

                    return input;
                } else {
                    return make_unexpected(
                        error_info(std::make_error_code(std::errc::invalid_argument), "Invalid parameter \""s.append(args[0]).append("\""))
                    );
                }
            }

            auto part1(input_t in) -> int {
                auto register_state = std::map<std::string_view, int>();

                for(instruction const& instruction : in) {
                    auto const& condition = instruction.expr;
                    if(operator_compare(condition.comp, register_state[condition.register_name], condition.value)) {
                        if(instruction.op == arithmetic_operator::inc) {
                            register_state[instruction.register_name] += instruction.operand;
                        } else if(instruction.op == arithmetic_operator::dec) {
                            register_state[instruction.register_name] -= instruction.operand;
                        } else {
                            assert(false && "Invalid operator");
                            UNREACHABLE();
                        }
                    }
                }

                return std::max_element(register_state.begin(), register_state.end(), [] (auto const& lhs, auto const& rhs) { return lhs.second < rhs.second; })->second;
            }

            auto part2(input_t in) -> int {
                (void)in;
                throw std::runtime_error("Not implemented");
            }

            auto solve(gsl::span<std::string_view const> args) -> int {
                if(args.size() < 1) {
                    throw std::runtime_error("Missing part parameter");
                }
                auto const part = args[0];
                args = args.subspan(1);

                auto const in = input(args);
                if(!in) {
                    std::cerr << in.error() << "\n";
                    return EXIT_FAILURE;
                }

                if(part == "1") {
                    std::cout << part1(std::move(in).value()) << "\n";
                    return EXIT_SUCCESS;
                } else if(part == "2") {
                    std::cout << part2(std::move(in).value()) << "\n";
                    return EXIT_SUCCESS;
                } else {
                    throw std::runtime_error{"Parameter \""s.append(part).append("\" was not a valid part (try 1 or 2)")};
                }
            }
        }
    }

    auto day(gsl::span<std::string_view const> args) -> int {
        if(args.size() < 1) {
            throw std::runtime_error("Missing part parameter");
        }
        auto const day = args[0];
        args = args.subspan(1);

        if(day == "1") {
            return day1::solve(args);
        } else if(day == "2") {
            return day2::solve(args);
        } else if(day == "3") {
            return day3::solve(args);
        } else if(day == "4") {
            return day4::solve(args);
        } else if(day == "5") {
            return day5::solve(args);
        } else if(day == "6") {
            return day6::solve(args);
        } else if(day == "7") {
            return day7::solve(args);
        } else if(day == "8") {
            return day8::solve(args);
        } else {
            throw std::runtime_error{"Parameter \""s.append(day).append("\" was not a valid day (try 1-25)")};
        }
    }
}