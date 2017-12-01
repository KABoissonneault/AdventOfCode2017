#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <numeric>

#include "Algorithm.h"

using namespace std::literals;

namespace kab_advent {
	int part1() {
		const auto captcha = [] {
			std::string input;
			if ( !std::getline( std::cin, input ) ) {
				throw std::runtime_error { "Could not read input" };
			}
			return input;
		}( );
		if ( !std::all_of( captcha.begin(), captcha.end(),
			[] ( const char c ) { return std::isdigit( static_cast< unsigned char >( c ) ); } ) ) {
			throw std::runtime_error { "Input \"" + captcha + "\" was not numerical" };
		}

		struct char_pair {
			char c1;
			char c2;
		};
		using char_pair_arr = std::vector<char_pair>;

		const auto characters = [captcha] {
			std::vector<char_pair> characters;
			for ( auto i = char_pair_arr::size_type { 0 }; i < captcha.size(); ++i ) {
				characters.emplace_back( char_pair { captcha[i], captcha[( i + 1 ) % captcha.size()] } );
			}
			return characters;
		}( );


		const auto adjacentCharacters = filter( characters, [] ( const char_pair c ) { return c.c1 == c.c2; } );
		const auto sum = std::accumulate( adjacentCharacters.begin(), adjacentCharacters.end(), 0,
			[] ( const int counter, const char_pair c ) { return counter + static_cast< int >( c.c1 - '0' ); } );
		std::cout << sum;

		return EXIT_SUCCESS;
	}

	int part2() {
		return EXIT_SUCCESS;
	}

	int day1( int argc, const char * argv[] ) {
		char * strEnd = nullptr;
		const auto part = argc >= 1 ? std::strtol( argv[0], &strEnd, 10 ) : 1;
		if ( strEnd == argv[0] ) {
			throw std::runtime_error { "Parameter \""s + argv[0] + "\" was not an integer" };
		}
		switch ( part ) {
		case 1:
			return part1();
		case 2:
			return part2();
		default:
			throw std::runtime_error { "Parameter \""s + argv[0] + "\" was not a valid part" };
		}
	}
}

int main( int argc, const char* argv[] ) try {
	std::cout.sync_with_stdio( false );
	std::cerr.sync_with_stdio( false );
	std::cin.sync_with_stdio( false );
	
	try {
		return kab_advent::day1( argc - 1, argv + 1 );
	}
	catch ( const std::runtime_error & e ) {
		std::cerr << "Invalid input: " << e.what() << "\n";
		return EXIT_FAILURE;
	}
} catch ( const std::exception & e ) {
	std::cerr << "Unhandled error in main: " << e.what() << "\n";
}
