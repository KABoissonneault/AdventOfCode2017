#pragma once

#include <string_view>

namespace kab_advent {
    template<typename T>
    struct parsed_value {
        T value;
        std::string_view rest_instruction;
    };
}
