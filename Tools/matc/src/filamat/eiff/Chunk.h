#pragma once

#include <vector>

#include "Flattener.h"

#include <filaflat/MaterialChunkType.h>

namespace filamat {

class Chunk{
public:
    virtual ~Chunk();

    ChunkType getType() const noexcept {
        return mType;
    }

    virtual void flatten(Flattener &f) = 0;

protected:
    explicit Chunk(ChunkType type) : mType(type) {
    }

private:
    ChunkType mType;
};

} // namespace filamat
