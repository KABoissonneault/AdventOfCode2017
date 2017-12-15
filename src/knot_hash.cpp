#include "knot_hash.h"

#include <vector>
#include <numeric>
#include <functional>

#include "algorithm.h"

namespace kab_advent {
    auto knot_hash(std::string input) -> std::string {
        auto const end_sequence = {17, 31, 73, 47, 23};
        input.insert(input.end(), end_sequence.begin(), end_sequence.end());

        auto list = std::vector<int>(256);
        std::iota(list.begin(), list.end(), 0);

        auto circular_list = make_circular_view(list);
        auto circular_it = circular_list.begin();
        auto skip_size = 0;

        for(int i = 0; i < 64; ++i) {
            skip_round(circular_it, input, skip_size);
        }

        auto dense_list = std::vector<int>(16);
        for(int i = 0; i < 16; ++i) {
            auto const list_begin = list.begin() + i * 16;
            dense_list[i] = std::accumulate(list_begin, list_begin + 16, 0, std::bit_xor<>());
        }

        input.clear();
        for(auto const value : dense_list) {
            auto const current_size = input.size();
            input.resize(current_size + 2);
            sprintf(input.data() + current_size, "%02x", value);
        }

        return input;
    }
}