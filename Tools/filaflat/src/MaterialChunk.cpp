#include <filaflat/MaterialChunk.h>
#include <filaflat/ChunkContainer.h>
#include "filaflat/backend/DriverEnums.h"
namespace filaflat {


static inline uint32_t makeKey(
    uint8_t& pass, MaterialChunk::Variant variant, MaterialChunk::ShaderStage stage) noexcept
{
    
    return (uint32_t(pass) << 16) | (uint32_t(stage) << 8) | variant;
}


MaterialChunk::MaterialChunk(ChunkContainer const& container)
        : mContainer(container) {
}

MaterialChunk::~MaterialChunk() noexcept = default;

bool MaterialChunk::initialize(filamat::ChunkType materialTag) {

    if (mBase != nullptr) {
        // initialize() should be called only once.
        return true;
    }

    auto it=mContainer.getChunkRange(materialTag);
    uint8_t const* start = it.first;
    uint8_t const* end = it.second;

    Unflattener unflattener(start, end);

    mUnflattener = unflattener;
    mMaterialTag = materialTag;
    mBase = unflattener.getCursor();

    // Read how many shaders we have in the chunk.
    uint64_t numShaders;
    if (!unflattener.read(&numShaders) || numShaders == 0) {
        return false;
    }

    // Read all index entries.
    for (uint64_t i = 0 ; i < numShaders; i++) {
        
        uint8_t passTag;
        Variant variant;
        uint8_t stage;
        uint32_t offsetValue;

        if( !unflattener.read(&passTag) )
        {
            return false;
        }

        if (!unflattener.read(&variant)) {
            return false;
        }

        if (!unflattener.read(&stage)) {
            return false;
        }

        if (!unflattener.read(&offsetValue)) {
            return false;
        }

        uint32_t key = makeKey(passTag, variant, ShaderStage(stage));
        mOffsets[key] = offsetValue;
    }
    return true;
}

bool MaterialChunk::getTextShader(Unflattener unflattener,
        BlobDictionary const& dictionary, ShaderContent& shaderContent, uint8_t passTag, Variant variant, ShaderStage shaderStage)
{
    if (mBase == nullptr) {
        return false;
    }

    // Jump and read
    uint32_t key = makeKey(passTag, variant, shaderStage);
    auto pos = mOffsets.find(key);
    if (pos == mOffsets.end()) {
        return false;
    }

    size_t offset = pos->second;
    if (offset == 0) {
        // This shader was not found.
        return false;
    }
    unflattener.setCursor(mBase + offset);

    // Read how big the shader is.
    uint32_t shaderSize = 0;
    if (!unflattener.read(&shaderSize)){
        return false;
    }

    // Read how many lines there are.
    uint32_t lineCount = 0;
    if (!unflattener.read(&lineCount)){
        return false;
    }

    shaderContent.reserve(shaderSize);
    shaderContent.resize(shaderSize);
    size_t cursor = 0;

    // Read all lines.
    for(int32_t i = 0 ; i < lineCount; i++) {
        uint16_t lineIndex;
        if (!unflattener.read(&lineIndex)) {
            return false;
        }
        const auto& content = dictionary[lineIndex];

        // Replace null with newline.
        memcpy(&shaderContent[cursor], content.data(), content.size() - 1);
        cursor += content.size() - 1;
        shaderContent[cursor++] = '\n';
    }

    // Write the terminating null character.
    shaderContent[cursor++] = 0;
    assert(cursor == shaderSize);

    return true;
}



bool MaterialChunk::hasShader(uint8_t passTag, Variant variant, ShaderStage stage) const noexcept
{
    if (mBase == nullptr) {
        return false;
    }
    auto pos = mOffsets.find(makeKey(passTag, variant, stage));
    return pos != mOffsets.end();
}

bool MaterialChunk::getShader(ShaderContent& shaderContent, BlobDictionary const& dictionary, uint8_t passTag,
    Variant variant, ShaderStage stage)
{
    return getTextShader(mUnflattener, dictionary, shaderContent, passTag, variant, stage);
    
}


uint32_t MaterialChunk::getShaderCount() const noexcept {
    Unflattener unflattener{ mUnflattener }; // make a copy
    uint64_t numShaders;
    unflattener.read(&numShaders);
    return uint32_t(numShaders);
}


} // namespace filaflat

