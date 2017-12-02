#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <string_view>
#include <system_error>

#include "day.h"

int main( int argc, const char* argv[] ) try {
	std::cout.sync_with_stdio(false);
	std::cerr.sync_with_stdio(false);
	std::cin.sync_with_stdio(false);
	
	auto const args = std::vector<std::string_view>( argv + 1, argv + argc );
	try {
		return kab_advent::day(args);
	}
	catch ( const std::runtime_error & e ) {
		std::cerr << "Invalid input: " << e.what() << "\n";
		return EXIT_FAILURE;
	}
} catch ( const std::exception & e ) {
	std::cerr << "Unhandled error in main: " << e.what() << "\n";
}
