#pragma once

#include "filaflat/backend/DriverEnums.h"
#include <string>

namespace filamat {

// TextEntry stores a shader in ASCII text format, like GLSL.
struct TextEntry {
    int passTag;
    uint8_t variant;
    filament::backend::ShaderStage stage;
    std::string shader;
};


}  