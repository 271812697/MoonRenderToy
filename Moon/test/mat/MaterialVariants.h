

#ifndef TNT_FILAMAT_MATERIAL_VARIANTS_H
#define TNT_FILAMAT_MATERIAL_VARIANTS_H

#include "Variant.h"

#include "test/DriverEnums.h"

#include <vector>

namespace TEST {

	struct MaterialVariant {
		using Stage = ShaderStage;
		MaterialVariant(Variant v, Stage s) noexcept : variant(v), stage(s) {}
		Variant variant;
		Stage stage;
	};

	std::vector<MaterialVariant> determineSurfaceVariants(
		UserVariantFilterMask, bool isLit, bool shadowMultiplier);

	std::vector<MaterialVariant> determinePostProcessVariants();

	std::vector<MaterialVariant> determineComputeVariants();

} // namespace filamat

#endif
