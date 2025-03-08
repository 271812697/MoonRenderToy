

#ifndef TNT_FILAMENT_DESCRIPTORSETS_H
#define TNT_FILAMENT_DESCRIPTORSETS_H

#include "test/DriverEnums.h" //<backend/>

#include "EngineEnums.h"//<private/filament/EngineEnums.h>

#include "MaterialEnums.h"

#include <test/utils/CString.h>

namespace TEST {

	DescriptorSetLayout const& getPostProcessLayout() noexcept;
	DescriptorSetLayout const& getDepthVariantLayout() noexcept;
	DescriptorSetLayout const& getSsrVariantLayout() noexcept;
	DescriptorSetLayout const& getPerRenderableLayout() noexcept;

	DescriptorSetLayout getPerViewDescriptorSetLayout(
		MaterialDomain domain,
		UserVariantFilterMask variantFilter,
		bool isLit,
		ReflectionMode reflectionMode,
		RefractionMode refractionMode) noexcept;

	utils::CString getDescriptorName(
		DescriptorSetBindingPoints set,
		descriptor_binding_t binding) noexcept;

} // namespace filament::descriptor_sets


#endif //TNT_FILAMENT_DESCRIPTORSETS_H
