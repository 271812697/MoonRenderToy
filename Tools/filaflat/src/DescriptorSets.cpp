#include "filaflat/DescriptorSets.h"
#include <unordered_map>

namespace filament
{
using namespace backend;
namespace descriptor_sets
{
struct PairSamplerTypeFormatHasher
{
    std::size_t operator()(const std::pair<SamplerType, SamplerFormat>& p) const
    {
        using UnderlyingSamplerType = std::underlying_type_t<SamplerType>;
        using UnderlyingSamplerFormat = std::underlying_type_t<SamplerFormat>;
        std::size_t seed = 0;
        std::size_t const hash1 = std::hash<UnderlyingSamplerType>{}(static_cast<UnderlyingSamplerType>(p.first));
        std::size_t const hash2 =
            std::hash<UnderlyingSamplerFormat>{}(static_cast<UnderlyingSamplerFormat>(p.second));
        seed ^= hash1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

static const std::unordered_map<std::pair<SamplerType, SamplerFormat>, DescriptorType, PairSamplerTypeFormatHasher>
    sDescriptorTypeMap{{{SamplerType::SAMPLER_2D, SamplerFormat::INT}, DescriptorType::SAMPLER_2D_INT},
        {{SamplerType::SAMPLER_2D, SamplerFormat::UINT}, DescriptorType::SAMPLER_2D_UINT},
        {{SamplerType::SAMPLER_2D, SamplerFormat::FLOAT}, DescriptorType::SAMPLER_2D_FLOAT},
        {{SamplerType::SAMPLER_2D, SamplerFormat::SHADOW}, DescriptorType::SAMPLER_2D_DEPTH},
        {{SamplerType::SAMPLER_2D_ARRAY, SamplerFormat::INT}, DescriptorType::SAMPLER_2D_ARRAY_INT},
        {{SamplerType::SAMPLER_2D_ARRAY, SamplerFormat::UINT}, DescriptorType::SAMPLER_2D_ARRAY_UINT},
        {{SamplerType::SAMPLER_2D_ARRAY, SamplerFormat::FLOAT}, DescriptorType::SAMPLER_2D_ARRAY_FLOAT},
        {{SamplerType::SAMPLER_2D_ARRAY, SamplerFormat::SHADOW}, DescriptorType::SAMPLER_2D_ARRAY_DEPTH},
        {{SamplerType::SAMPLER_CUBEMAP, SamplerFormat::INT}, DescriptorType::SAMPLER_CUBE_INT},
        {{SamplerType::SAMPLER_CUBEMAP, SamplerFormat::UINT}, DescriptorType::SAMPLER_CUBE_UINT},
        {{SamplerType::SAMPLER_CUBEMAP, SamplerFormat::FLOAT}, DescriptorType::SAMPLER_CUBE_FLOAT},
        {{SamplerType::SAMPLER_CUBEMAP, SamplerFormat::SHADOW}, DescriptorType::SAMPLER_CUBE_DEPTH},
        {{SamplerType::SAMPLER_CUBEMAP_ARRAY, SamplerFormat::INT}, DescriptorType::SAMPLER_CUBE_ARRAY_INT},
        {{SamplerType::SAMPLER_CUBEMAP_ARRAY, SamplerFormat::UINT}, DescriptorType::SAMPLER_CUBE_ARRAY_UINT},
        {{SamplerType::SAMPLER_CUBEMAP_ARRAY, SamplerFormat::FLOAT}, DescriptorType::SAMPLER_CUBE_ARRAY_FLOAT},
        {{SamplerType::SAMPLER_CUBEMAP_ARRAY, SamplerFormat::SHADOW}, DescriptorType::SAMPLER_CUBE_ARRAY_DEPTH},
        {{SamplerType::SAMPLER_3D, SamplerFormat::INT}, DescriptorType::SAMPLER_3D_INT},
        {{SamplerType::SAMPLER_3D, SamplerFormat::UINT}, DescriptorType::SAMPLER_3D_UINT},
        {{SamplerType::SAMPLER_3D, SamplerFormat::FLOAT}, DescriptorType::SAMPLER_3D_FLOAT},
        {{SamplerType::SAMPLER_EXTERNAL, SamplerFormat::FLOAT}, DescriptorType::SAMPLER_EXTERNAL},
        {{SamplerType::SAMPLER_BUFFER, SamplerFormat::UINT}, DescriptorType::SAMPLER_BUFFERUINT},
        {{SamplerType::SAMPLER_BUFFER, SamplerFormat::INT}, DescriptorType::SAMPLER_BUFFERINT},
        {{SamplerType::SAMPLER_BUFFER, SamplerFormat::FLOAT}, DescriptorType::SAMPLER_BUFFERFLOAT}};

backend::DescriptorType getDescriptorType(backend::SamplerType type, backend::SamplerFormat format)
{
    auto const pos = sDescriptorTypeMap.find({type, format});

    return pos->second;
}

} // namespace descriptor_sets
}

