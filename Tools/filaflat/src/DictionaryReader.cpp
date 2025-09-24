
#include <filaflat/DictionaryReader.h>

#include <filaflat/ChunkContainer.h>
#include <filaflat/Unflattener.h>
#include <assert.h>

using namespace filamat;

namespace filaflat {

bool DictionaryReader::unflatten(ChunkContainer const& container,
        ChunkContainer::Type dictionaryTag,
        BlobDictionary& dictionary) {
    auto it = container.getChunkRange(dictionaryTag);
    uint8_t const* start = it.first;
    uint8_t const* end = it.second;
  
    Unflattener unflattener(start, end);

    if (dictionaryTag == ChunkType::DictionaryText) {
        uint32_t stringCount = 0;
        if (!unflattener.read(&stringCount)) {
            return false;
        }

        dictionary.reserve(stringCount);
        for (uint32_t i = 0; i < stringCount; i++) {
            const char* str;
            if (!unflattener.read(&str)) {
                return false;
            }
            // BlobDictionary hold binary chunks and does not care if the data holds text, it is
            // therefore crucial to include the trailing null.
            dictionary.emplace_back(static_cast<uint32_t>(strlen(str) + 1u));
            memcpy(dictionary.back().data(), str, dictionary.back().size());
        }
        return true;
    }

    return false;
}

} // namespace filaflat
