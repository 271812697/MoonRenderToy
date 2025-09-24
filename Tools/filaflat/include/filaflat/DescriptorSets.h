#pragma once
#include <filaflat/backend/DriverEnums.h>

namespace filament
{
namespace descriptor_sets {
backend::DescriptorType getDescriptorType(backend::SamplerType type, backend::SamplerFormat format);
} // namespace filament::descriptor_sets

}

