#pragma once

#include <stdint.h>
#include <vector>

#include "Chunk.h"
#include "Flattener.h"
#include "LineDictionary.h"

namespace filamat {

class DictionaryTextChunk final : public Chunk {
public:
    DictionaryTextChunk(LineDictionary&& dictionary, ChunkType chunkType);
    ~DictionaryTextChunk() = default;

    const LineDictionary& getDictionary() const noexcept { return mDictionary; }

private:
    void flatten(Flattener& f) override;

    const LineDictionary mDictionary;
};

} // namespace filamat
