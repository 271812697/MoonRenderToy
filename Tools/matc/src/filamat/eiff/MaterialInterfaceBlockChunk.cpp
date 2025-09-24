#include "MaterialInterfaceBlockChunk.h"
#include "filaflat/MaterialChunkType.h"
#include "filaflat/BufferInterfaceBlock.h"
#include "filaflat/DescriptorSets.h"
#include "filaflat/SamplerInterfaceBlock.h"
#include "filaflat/backend/DriverEnums.h"





#include <utility>

#include <stdint.h>

using namespace filament;

namespace filamat {

MaterialUniformInterfaceBlockChunk::MaterialUniformInterfaceBlockChunk(
        BufferInterfaceBlock const& uib) :
        Chunk(MaterialUib),
        mUib(uib) {
}

void MaterialUniformInterfaceBlockChunk::flatten(Flattener& f) {
    f.writeString(mUib.getName());
    auto uibFields = mUib.getFieldInfoList();
    f.writeUint64(uibFields.size());
    for (auto uInfo: uibFields) {
        f.writeString(uInfo.name.c_str());
        f.writeUint64(uInfo.size);
        f.writeUint8(static_cast<uint8_t>(uInfo.type));
        f.writeUint8(static_cast<uint8_t>(uInfo.precision));
        f.writeUint8(static_cast<uint8_t>(uInfo.associatedSampler));
    }
}

// ------------------------------------------------------------------------------------------------

MaterialSamplerInterfaceBlockChunk::MaterialSamplerInterfaceBlockChunk(
        SamplerInterfaceBlock const& sib) :
        Chunk(MaterialSib),
        mSib(sib) {
}

void MaterialSamplerInterfaceBlockChunk::flatten(Flattener& f) {
    f.writeString(mSib.getName().c_str());
    auto sibFields = mSib.getSamplerInfoList();
    f.writeUint64(sibFields.size());
    for (auto sInfo: sibFields) {
        f.writeString(sInfo.name.c_str());
        f.writeUint8(static_cast<uint8_t>(sInfo.binding));
        f.writeUint8(static_cast<uint8_t>(sInfo.type));
        f.writeUint8(static_cast<uint8_t>(sInfo.format));
        f.writeUint8(static_cast<uint8_t>(sInfo.precision));
        f.writeBool(sInfo.unfilterable);
        f.writeBool(sInfo.multisample);
        f.writeBool(sInfo.preLoad);
    }
}





// ------------------------------------------------------------------------------------------------

MaterialDescriptorBindingsChuck::MaterialDescriptorBindingsChuck(Container const& sib) noexcept
        : Chunk(MaterialDescriptorBindingsInfo),
          mSamplerInterfaceBlock(sib) {
}

void MaterialDescriptorBindingsChuck::flatten(Flattener& f) {


    using namespace backend;

    // samplers + 1 descriptor for the UBO
    f.writeUint8(mSamplerInterfaceBlock.getSize() + 1);

    // our UBO descriptor is always at binding 0
    std::string const uboName = "MaterialParams";
    f.writeString(uboName);
    f.writeUint8(uint8_t(DescriptorType::UNIFORM_BUFFER));
    f.writeUint8(0);

    // all the material's sampler descriptors
    for (auto const& entry: mSamplerInterfaceBlock.getSamplerInfoList()) {
        f.writeString({ entry.uniformName.data(), entry.uniformName.size() });
        f.writeUint8(uint8_t(descriptor_sets::getDescriptorType(entry.type, entry.format)));
        f.writeUint8(entry.binding);
    }
}

// ------------------------------------------------------------------------------------------------

MaterialDescriptorSetLayoutChunk::MaterialDescriptorSetLayoutChunk(Container const& sib) noexcept
        : Chunk(MaterialDescriptorSetLayoutInfo),
          mSamplerInterfaceBlock(sib) {
}

void MaterialDescriptorSetLayoutChunk::flatten(Flattener& f) {


    using namespace backend;

    // samplers + 1 descriptor for the UBO
    f.writeUint8(mSamplerInterfaceBlock.getSize() + 1);

    // our UBO descriptor is always at binding 0
    f.writeUint8(uint8_t(DescriptorType::UNIFORM_BUFFER));
    
    f.writeUint8(0);
    f.writeUint8(uint8_t(DescriptorFlags::NONE));
    f.writeUint16(0);

    // all the material's sampler descriptors
    for (auto const& entry: mSamplerInterfaceBlock.getSamplerInfoList()) {
        f.writeUint8(uint8_t(descriptor_sets::getDescriptorType(entry.type, entry.format)));
       
        f.writeUint8(entry.binding);
        if (entry.unfilterable) {
            f.writeUint8(uint8_t(DescriptorFlags::UNFILTERABLE));
        } else {
            f.writeUint8(uint8_t(DescriptorFlags::NONE));
        }
        f.writeUint16(0);
    }
}

} // namespace filamat
