#pragma once
#include <filaflat/MaterialChunkType.h>
#include <unordered_map>
#include <vector>
namespace filaflat {

using ShaderContent = std::vector<uint8_t>;
using BlobDictionary = std::vector<ShaderContent>;

class Unflattener;

// Allows to build a map of chunks in a Package and get direct individual access based on chunk ID.
class  ChunkContainer {
public:
    using Type = filamat::ChunkType;

    ChunkContainer(void const* data, size_t size) : mData(data), mSize(size) {}

    ~ChunkContainer() noexcept;

    // Must be called before trying to access any of the chunk. Fails and return false ONLY if
    // an incomplete chunk is found or if a chunk with bogus size is found.
    bool parse() noexcept;

    typedef struct {
        const uint8_t* start;
        size_t size;
    } ChunkDesc;

    typedef struct {
        Type type;
        ChunkDesc desc;
    } Chunk;

    size_t getChunkCount() const noexcept {
        return mChunks.size();
    }

    Chunk getChunk(size_t index) const noexcept {
        auto it = mChunks.begin();
        std::advance(it, index);
        return { it->first, it->second };
    }

    std::pair<uint8_t const*, uint8_t const*> getChunkRange(Type type) const noexcept {
        ChunkDesc chunkDesc;
        bool const success = hasChunk(type, &chunkDesc);
        if (success) {
            return { chunkDesc.start, chunkDesc.start + chunkDesc.size };
        }
        return { nullptr, nullptr };
    }

    bool hasChunk(Type type) const noexcept;
    bool hasChunk(Type type, ChunkDesc* pChunkDesc) const noexcept;

    void const* getData() const { return mData; }

    size_t getSize() const { return mSize; }

private:
    bool parseChunk(Unflattener& unflattener);

    void const* mData;
    size_t mSize;
    std::unordered_map<Type, ChunkContainer::ChunkDesc> mChunks;
};

} // namespace filaflat
