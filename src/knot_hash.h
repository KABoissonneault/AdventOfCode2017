#pragma once

#include <algorithm>
#include <string>

namespace kab_advent {
    template<typename ListIt, typename SkipListT>
    auto skip_round(ListIt & circular_it, SkipListT const& skip_list, int & skip_size) {
        for(auto const skip : skip_list) {
            auto const twist_end = circular_it + skip;
            std::reverse(circular_it, twist_end);
            circular_it += skip + skip_size++;
        }
    }

    auto knot_hash(std::string input)->std::string;
}