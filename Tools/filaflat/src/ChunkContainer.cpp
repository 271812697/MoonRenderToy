
#include <filaflat/ChunkContainer.h>

#include <filaflat/Unflattener.h>

namespace filaflat {

ChunkContainer::~ChunkContainer() noexcept = default;

bool ChunkContainer::hasChunk(Type type) const noexcept {
    auto const& chunks = mChunks;
    auto pos = chunks.find(type);
    return pos != chunks.end();
}

bool ChunkContainer::hasChunk(Type type, ChunkDesc* pChunkDesc) const noexcept {
    assert(pChunkDesc);
    auto const& chunks = mChunks;
    auto pos = chunks.find(type);
    if (pos != chunks.end()) {
        *pChunkDesc = pos->second;
        return true;
    }
    return false;
}

bool ChunkContainer::parseChunk(Unflattener& unflattener) {
    uint64_t type;
    if (!unflattener.read(&type)) {
        return false;
    }

    uint32_t size;
    if (!unflattener.read(&size)) {
        return false;
    }

    // If size goes beyond the boundaries of the package, this is an invalid chunk. Discard it.
    // All remaining chunks cannot be accessed and will not be mapped.
    auto cursor = unflattener.getCursor();
    if (!(cursor + size >= (uint8_t *)mData &&
          cursor + size <= (uint8_t *)mData + mSize)) {
        return false;
    }

    mChunks[Type(type)] = { cursor, size };
    unflattener.setCursor(cursor + size);
    return true;
}

bool ChunkContainer::parse() noexcept {
    Unflattener unflattener((uint8_t *)mData, (uint8_t *)mData + mSize);
    do {
        if (!parseChunk(unflattener)) {
            return false;
        }
    } while (unflattener.hasData());
    return true;
}

} // namespace filaflat
