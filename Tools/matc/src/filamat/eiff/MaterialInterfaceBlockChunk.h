#pragma once
#include "Chunk.h"
#include <filaflat/backend/DriverEnums.h>
#include <tuple>
#include <stdint.h>

namespace
{

   

}
namespace filament {
class SamplerInterfaceBlock;
class BufferInterfaceBlock;
struct SubpassInfo;
struct MaterialConstant;
struct MaterialPushConstant;
} // namespace filament

namespace filamat {

class MaterialUniformInterfaceBlockChunk final : public Chunk {
public:
    explicit MaterialUniformInterfaceBlockChunk(filament::BufferInterfaceBlock const& uib);
    ~MaterialUniformInterfaceBlockChunk() override = default;

private:
    void flatten(Flattener&) override;

    filament::BufferInterfaceBlock const& mUib;
};

// ------------------------------------------------------------------------------------------------

class MaterialSamplerInterfaceBlockChunk final : public Chunk {
public:
    explicit MaterialSamplerInterfaceBlockChunk(filament::SamplerInterfaceBlock const& sib);
    ~MaterialSamplerInterfaceBlockChunk() override = default;

private:
    void flatten(Flattener&) override;

    filament::SamplerInterfaceBlock const& mSib;
};

// ------------------------------------------------------------------------------------------------

class MaterialSubpassInterfaceBlockChunk final : public Chunk {
public:
    explicit MaterialSubpassInterfaceBlockChunk(filament::SubpassInfo const& subpass);
    ~MaterialSubpassInterfaceBlockChunk() override = default;

private:
    void flatten(Flattener&) override;

    filament::SubpassInfo const& mSubpass;
};

// ------------------------------------------------------------------------------------------------

class MaterialConstantParametersChunk final : public Chunk {
public:
    explicit MaterialConstantParametersChunk(
            std::vector<filament::MaterialConstant> constants);
    ~MaterialConstantParametersChunk() override = default;

private:
    void flatten(Flattener&) override;

    std::vector<filament::MaterialConstant> mConstants;
};

// ------------------------------------------------------------------------------------------------

class MaterialDescriptorBindingsChuck final : public Chunk {
    using Container = filament::SamplerInterfaceBlock;
public:
    explicit MaterialDescriptorBindingsChuck(Container const& sib) noexcept;
    ~MaterialDescriptorBindingsChuck() override = default;

private:
    void flatten(Flattener&) override;

    Container const& mSamplerInterfaceBlock;
};

// ------------------------------------------------------------------------------------------------

class MaterialDescriptorSetLayoutChunk final : public Chunk {
    using Container = filament::SamplerInterfaceBlock;
public:
    explicit MaterialDescriptorSetLayoutChunk(Container const& sib) noexcept;
    ~MaterialDescriptorSetLayoutChunk() override = default;

private:
    void flatten(Flattener&) override;

    Container const& mSamplerInterfaceBlock;
};

} // namespace filamat
