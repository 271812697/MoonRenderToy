#include "filaflat/SamplerInterfaceBlock.h"
#include <filaflat/DescriptorSets.h>
#include <filaflat/backend/DriverEnums.h>
#include <assert.h>

namespace filament {

SamplerInterfaceBlock::Builder::Builder() = default;
SamplerInterfaceBlock::Builder::~Builder() noexcept = default;

SamplerInterfaceBlock::Builder&
SamplerInterfaceBlock::Builder::name(std::string interfaceBlockName) {
    mName = { interfaceBlockName.data(), interfaceBlockName.size() };
    return *this;
}

SamplerInterfaceBlock::Builder&
SamplerInterfaceBlock::Builder::stageFlags(backend::ShaderStageFlags stageFlags) {
    mStageFlags = stageFlags;
    return *this;
}

SamplerInterfaceBlock::Builder& SamplerInterfaceBlock::Builder::add(std::string samplerName,
        Binding binding, Type type, Format format, Precision precision, bool unfilterable,
    bool multisample, bool preload, ShaderStageFlags flags) noexcept
{
    mEntries.push_back({
        { samplerName.data(), samplerName.size() }, // name
        {},                                         // uniform name
        binding, type, format, precision, flags,unfilterable, multisample,preload });
    return *this;
}

SamplerInterfaceBlock SamplerInterfaceBlock::Builder::build() {
    return SamplerInterfaceBlock(*this);
}

SamplerInterfaceBlock::Builder& SamplerInterfaceBlock::Builder::add(
        std::initializer_list<ListEntry> list) noexcept {
    for (auto& e: list) {
        add(e.name, e.binding, e.type, e.format, e.precision, e.unfilterable, e.multisample);
    }
    return *this;
}

// -------------------------------------------------------------------------------------------------

SamplerInterfaceBlock::SamplerInterfaceBlock() = default;
SamplerInterfaceBlock::SamplerInterfaceBlock(SamplerInterfaceBlock&& rhs) noexcept = default;
SamplerInterfaceBlock& SamplerInterfaceBlock::operator=(SamplerInterfaceBlock&& rhs) noexcept = default;
SamplerInterfaceBlock::~SamplerInterfaceBlock() noexcept = default;

SamplerInterfaceBlock::SamplerInterfaceBlock(Builder const& builder) noexcept
    : mName(builder.mName), mStageFlags(builder.mStageFlags),
    mSamplersInfoList(builder.mEntries.size())
{
    auto& infoMap = mInfoMap;
    infoMap.reserve(builder.mEntries.size());

    auto& samplersInfoList = mSamplersInfoList;

    for (auto const& e : builder.mEntries) {
        size_t const i = std::distance(builder.mEntries.data(), &e);
        SamplerInfo& info = samplersInfoList[i];
        info = e;
       
        info.uniformName = e.name;
        //generateUniformName(mName.c_str(), );
        infoMap[{ info.name.data(), info.name.size() }] = i; // info.name.c_str() guaranteed constant
    }
}

const SamplerInterfaceBlock::SamplerInfo* SamplerInterfaceBlock::getSamplerInfo(
        std::string name) const {
    auto pos = mInfoMap.find(name);
    if( pos == mInfoMap.end() )
    {
        assert(false && "getSamplerInfo not found!");
        return nullptr;
    }
   
    return &mSamplersInfoList[pos->second];
}

std::vector<filament::SamplerInterfaceBlock::SamplerInfo>& SamplerInterfaceBlock::getSamplerInfoList()
{
    return mSamplersInfoList;
}

std::string SamplerInterfaceBlock::generateUniformName(const char* group, const char* sampler) noexcept {
    char uniformName[256];

    // sampler interface block name
    char* const prefix = std::copy_n(group,
            std::min(sizeof(uniformName) / 2, strlen(group)), uniformName);
    if (uniformName[0] >= 'A' && uniformName[0] <= 'Z') {
        uniformName[0] |= 0x20; // poor man's tolower()
    }
    *prefix = '_';

    char* last = std::copy_n(sampler,
            std::min(sizeof(uniformName) / 2 - 2, strlen(sampler)),
            prefix + 1);
    *last++ = 0; // null terminator
    assert(last <= std::end(uniformName));

    return std::string{ uniformName, size_t(last - uniformName) - 1u };
}



} // namespace filament
