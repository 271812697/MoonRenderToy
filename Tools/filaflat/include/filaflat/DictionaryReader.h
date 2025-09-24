#pragma once
#include <filaflat/ChunkContainer.h>
namespace filaflat {

struct DictionaryReader {
    static bool unflatten(ChunkContainer const& container,
            ChunkContainer::Type dictionaryTag,
            BlobDictionary& dictionary);
};

} // namespace filaflat
