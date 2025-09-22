#pragma once
#include <filaflat/MaterialChunkType.h>
#include <filaflat/ChunkContainer.h>
#include <filaflat/Unflattener.h>
#include "filaflat/backend/DriverEnums.h"

#include <unordered_map>


namespace filaflat {

class MaterialChunk {
public:
   
    using ShaderStage = filament::backend::ShaderStage;
    using Variant = uint8_t;

    explicit MaterialChunk(ChunkContainer const& container);
    ~MaterialChunk() noexcept;

    // call this once after container.parse() has been called
    bool initialize(filamat::ChunkType materialTag);

    // call this as many times as needed
    // populates "shaderContent" with the requested shader, or returns false on failure.
    bool getShader(ShaderContent& shaderContent, BlobDictionary const& dictionary, uint8_t passTag,
        Variant variant, ShaderStage stage);
 

    uint32_t getShaderCount() const noexcept;


    bool hasShader(uint8_t passTag, Variant variant, ShaderStage stage) const noexcept;


    const std::unordered_map<uint32_t, uint32_t>& getOffsets() const { return mOffsets; }
    // @}

private:
    ChunkContainer const& mContainer;
    filamat::ChunkType mMaterialTag = filamat::ChunkType::Unknown;
    Unflattener mUnflattener;
    const uint8_t* mBase = nullptr;
    std::unordered_map<uint32_t, uint32_t> mOffsets;

    bool getTextShader(Unflattener unflattener,
            BlobDictionary const& dictionary, ShaderContent& shaderContent,
            uint8_t passTag, Variant variant, ShaderStage shaderStage);


};

} // namespace filamat
