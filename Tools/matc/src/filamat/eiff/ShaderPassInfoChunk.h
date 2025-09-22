#pragma once
#include "Chunk.h"

namespace filament {
class ShaderPassInfo;
} // namespace filament

namespace filamat {

class ShaderPassInfoChunk final : public Chunk {
public:
    explicit ShaderPassInfoChunk(filament::ShaderPassInfo & uib);
    ~ShaderPassInfoChunk() override = default;

private:
    void flatten(Flattener&) override;

    filament::ShaderPassInfo & mPass;
};



} // namespace filamat
