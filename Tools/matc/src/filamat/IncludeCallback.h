#pragma once
#include <string>
#include <functional>

namespace filamat {

struct IncludeResult {

    std::string includeName;

    std::string text;

    size_t lineNumberOffset = 0;


    std::string name;
};

using IncludeCallback = std::function<bool(
        IncludeResult& result)>;

} // namespace filamat

