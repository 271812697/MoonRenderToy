
#include "MaterialVariants.h"

#include "ShaderGenerator.h"

#include "test/mat/EngineEnums.h"
#include "Variant.h"

#include <test/DriverEnums.h>

#include "MaterialEnums.h"


#include <algorithm>
#include <vector>

#include <stddef.h>
#include <stdint.h>

namespace TEST {

	std::vector<MaterialVariant> determineSurfaceVariants(UserVariantFilterMask userVariantFilter, bool isLit, bool shadowMultiplier) {
		std::vector<MaterialVariant> variants;
		for (size_t k = 0; k < VARIANT_COUNT; k++) {
			Variant const variant(k);
			if (Variant::isReserved(variant)) {
				continue;
			}

			Variant filteredVariant =
				Variant::filterUserVariant(variant, userVariantFilter);

			// Remove variants for unlit materials
			filteredVariant = Variant::filterVariant(
				filteredVariant, isLit || shadowMultiplier);

			auto const vertexVariant = Variant::filterVariantVertex(filteredVariant);
			if (vertexVariant == variant) {
				variants.emplace_back(variant, ShaderStage::VERTEX);
			}

			auto const fragmentVariant = Variant::filterVariantFragment(filteredVariant);
			if (fragmentVariant == variant) {
				variants.emplace_back(variant, ShaderStage::FRAGMENT);
			}

			// Here we make sure that the combination of vertex and fragment variants have compatible
			// PER_VIEW descriptor-set layouts. This could actually be a static/compile-time check
			// because it is entirely decided in DescriptorSets.cpp. Unfortunately it's not possible
			// to write this entirely as a constexpr.

			if ((vertexVariant != fragmentVariant)) {
				// fragment and vertex variants are different, we need to check the layouts are
				// compatible.


				// And we need to do that for all configurations of the "PER_VIEW" descriptor set
				// layouts (there are eight).
				// See ShaderGenerator::getPerViewDescriptorSetLayoutWithVariant.
				for (auto reflection : {
						ReflectionMode::SCREEN_SPACE,
						ReflectionMode::DEFAULT }) {
					for (auto refraction : {
							RefractionMode::SCREEN_SPACE,
							RefractionMode::CUBEMAP,
							RefractionMode::NONE }) {
						auto const vdsl = ShaderGenerator::getPerViewDescriptorSetLayoutWithVariant(
							vertexVariant, userVariantFilter, isLit || shadowMultiplier,
							reflection, refraction);
						auto const fdsl = ShaderGenerator::getPerViewDescriptorSetLayoutWithVariant(
							fragmentVariant, userVariantFilter, isLit || shadowMultiplier,
							reflection, refraction);
						// Check that all bindings present in the vertex shader DescriptorSetLayout
						// are also present in the fragment shader DescriptorSetLayout.
						for (auto const& r : vdsl.bindings) {
							if (!hasShaderType(r.stageFlags, ShaderStage::VERTEX)) {
								// ignore descriptors that are of the fragment stage only
								continue;
							}
							auto const pos = std::find_if(fdsl.bindings.begin(), fdsl.bindings.end(),
								[r](auto const& l) {
									return l.count == r.count && l.type == r.type &&
										l.binding == r.binding && l.flags == r.flags &&
										l.stageFlags == r.stageFlags;
								});

							// A mismatch is fatal. The material is ill-formed. This typically
							// mean a bug / inconsistency in DescriptorsSets.cpp
							assert(pos != fdsl.bindings.end());

						}
					}
				}
			}

		}
		return variants;
	}

	std::vector<MaterialVariant> determinePostProcessVariants() {
		std::vector<MaterialVariant> variants;
		// TODO: add a way to filter out post-process variants (e.g., the transparent variant if only
		// opaque is needed)
		for (Variant::type_t k = 0; k < POST_PROCESS_VARIANT_COUNT; k++) {
			Variant const variant(k);
			variants.emplace_back(variant, ShaderStage::VERTEX);
			variants.emplace_back(variant, ShaderStage::FRAGMENT);
		}
		return variants;
	}

	std::vector<MaterialVariant> determineComputeVariants() {
		// TODO: should we have variants for compute shaders?
		std::vector<MaterialVariant> variants;
		Variant variant(0);
		variants.emplace_back(variant, ShaderStage::COMPUTE);
		return variants;
	}

} // namespace filamat
