#pragma once
#include "filaflat/MaterialChunkType.h"


#include "Chunk.h"
#include "SimpleFieldChunk.h"

#include <memory>
#include <vector>

namespace filamat {

class Flattener;

class ChunkContainer {
public:
    ChunkContainer() = default;
    ~ChunkContainer() = default;

    template <typename T,
             std::enable_if_t<std::is_base_of<Chunk, T>::value, int> = 0,
             typename... Args>
    const T& push(Args&&... args) {
        T* chunk = new T(std::forward<Args>(args)...);
        mChildren.emplace_back(chunk);
        return *chunk;
    }

    // Helper method to add a SimpleFieldChunk to this ChunkContainer.
    template <typename T, typename... Args>
    const SimpleFieldChunk<T>& emplace(Args&&... args) {
        return push<SimpleFieldChunk<T>>(std::forward<Args>(args)...);
    }

    size_t getSize() const;
    size_t flatten(Flattener& f) const;

private:
    using ChunkPtr = std::unique_ptr<Chunk>;
    std::vector<ChunkPtr> mChildren;
};

} // namespace filamat
