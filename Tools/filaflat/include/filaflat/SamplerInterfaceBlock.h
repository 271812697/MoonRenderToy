#pragma once
#include "filaflat/backend/DriverEnums.h"

#include "filaflat/DescriptorSets.h"

#include <initializer_list>
#include <unordered_map>

#include <vector>

#include <stddef.h>
#include <stdint.h>

namespace filament {

class SamplerInterfaceBlock {
public:
    SamplerInterfaceBlock();

    SamplerInterfaceBlock(const SamplerInterfaceBlock& rhs) = delete;
    SamplerInterfaceBlock(SamplerInterfaceBlock&& rhs) noexcept;

    SamplerInterfaceBlock& operator=(const SamplerInterfaceBlock& rhs) = delete;
    SamplerInterfaceBlock& operator=(SamplerInterfaceBlock&& rhs) noexcept;

    ~SamplerInterfaceBlock() noexcept;

    using Type = backend::SamplerType;
    using Format = backend::SamplerFormat;
    using Precision = backend::Precision;
    using Binding = backend::descriptor_binding_t;
    using ShaderStageFlags = backend::ShaderStageFlags;

    struct SamplerInfo { // NOLINT(cppcoreguidelines-pro-type-member-init)
        std::string name;        // name of this sampler
        std::string uniformName; // name of the uniform holding this sampler (needed for glsl/MSL)
        Binding binding;            // binding in the descriptor set
        Type type;                  // type of this sampler
        Format format;              // format of this sampler
        Precision precision;        // precision of this sampler
        ShaderStageFlags stageFlags;
        bool unfilterable;          // whether the sampling should be unfiltered.
        bool multisample;           // multisample capable
        bool preLoad;
    };

    using SamplerInfoList = std::vector<SamplerInfo>;

    class Builder {
    public:
        Builder();
        ~Builder() noexcept;

        Builder(Builder const& rhs) = default;
        Builder(Builder&& rhs) noexcept = default;
        Builder& operator=(Builder const& rhs) = default;
        Builder& operator=(Builder&& rhs) noexcept = default;

        struct ListEntry { // NOLINT(cppcoreguidelines-pro-type-member-init)
            std::string name;          // name of this sampler
            Binding binding;                // binding in the descriptor set
            Type type;                      // type of this sampler
            Format format;                  // format of this sampler
            Precision precision;            // precision of this sampler
            bool unfilterable = false;      // whether the sampling should be unfiltered.
            bool multisample = false;       // multisample capable
            ShaderStageFlags stages =
                    ShaderStageFlags::ALL_SHADER_STAGE_FLAGS; // shader stages using this sampler
            bool preLoad = false;
        };

        // Give a name to this sampler interface block
        Builder& name(std::string interfaceBlockName);

        Builder& stageFlags(backend::ShaderStageFlags stageFlags);

        // Add a sampler
        Builder& add(std::string samplerName, Binding binding, Type type, Format format,
                     Precision precision = Precision::MEDIUM, bool unfilterable = false, bool multisample = false,
                    
                     bool preLoad = false, ShaderStageFlags flags = ShaderStageFlags::ALL_SHADER_STAGE_FLAGS) noexcept;

        // Add multiple samplers
        Builder& add(std::initializer_list<ListEntry> list) noexcept;

        // build and return the SamplerInterfaceBlock
        SamplerInterfaceBlock build();
    private:
        friend class SamplerInterfaceBlock;
        std::string mName;
        backend::ShaderStageFlags mStageFlags = backend::ShaderStageFlags::ALL_SHADER_STAGE_FLAGS;
        std::vector<SamplerInfo> mEntries;
    };

    // name of this sampler interface block
    const std::string& getName() const noexcept { return mName; }

    backend::ShaderStageFlags getStageFlags() const noexcept { return mStageFlags; }

    // size needed to store the samplers described by this interface block in a SamplerGroup
    size_t getSize() const noexcept { return mSamplersInfoList.size(); }

    // list of information records for each sampler
    SamplerInfoList const& getSamplerInfoList() const noexcept {
        return mSamplersInfoList;
    }

    // information record for sampler of the given name
    SamplerInfo const* getSamplerInfo(std::string name) const;
    std::vector<SamplerInfo>& getSamplerInfoList();
    bool hasSampler(std::string name) const noexcept {
        return mInfoMap.find(name) != mInfoMap.end();
    }

    bool isEmpty() const noexcept { return mSamplersInfoList.empty(); }

    static std::string generateUniformName(const char* group, const char* sampler) noexcept;


private:
    friend class Builder;


    explicit SamplerInterfaceBlock(Builder const& builder) noexcept;

    std::string mName;
    backend::ShaderStageFlags mStageFlags{}; // It's needed to check if MAX_SAMPLER_COUNT is exceeded.
    std::vector<SamplerInfo> mSamplersInfoList;
    std::unordered_map<std::string, uint32_t> mInfoMap;
};

} // namespace filament

