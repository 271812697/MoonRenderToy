#pragma once
#include <map>
#include <Rendering/Core/CompositeRenderer.h>
#include <Rendering/Data/Frustum.h>
#include <Rendering/Entities/Drawable.h>
#include <Rendering/HAL/UniformBuffer.h>
#include <Rendering/HAL/ShaderStorageBuffer.h>
#include <Rendering/Resources/Mesh.h>

#include <Core/ECS/Actor.h>
#include <Core/ECS/Components/CCamera.h>
#include <Core/Rendering/EVisibilityFlags.h>
#include <Core/Resources/Material.h>
#include <Core/SceneSystem/Scene.h>

namespace Core::Rendering
{

	class SceneRenderer : public ::Rendering::Core::CompositeRenderer
	{
	public:
		enum class EOrderingMode
		{
			BACK_TO_FRONT,
			FRONT_TO_BACK,
		};

		template<EOrderingMode OrderingMode>
		struct DrawOrder
		{
			const int order;
			const float distance;
			bool operator<(const DrawOrder& p_other) const
			{
				if (order == p_other.order)
				{
					if constexpr (OrderingMode == EOrderingMode::BACK_TO_FRONT)
					{
						return distance > p_other.distance;
					}
					else
					{
						return distance < p_other.distance;
					}
				}
				else
				{
					return order < p_other.order;
				}
			}
		};

		template<EOrderingMode OrderingMode>
		using DrawableMap = std::multimap<DrawOrder<OrderingMode>, ::Rendering::Entities::Drawable>;


		struct SceneDescriptor
		{
			Core::SceneSystem::Scene& scene;
			Tools::Utils::OptRef<const ::Rendering::Data::Frustum> frustumerride;
			Tools::Utils::OptRef<::Rendering::Data::Material> errideMaterial;
			Tools::Utils::OptRef<::Rendering::Data::Material> fallbackMaterial;
		};

		struct SceneParsingInput
		{
			Core::SceneSystem::Scene& scene;
		};

		struct SceneDrawablesDescriptor
		{
			std::vector<::Rendering::Entities::Drawable> drawables;
		};


		struct SceneDrawableDescriptor
		{
			Core::ECS::Actor& actor;
			EVisibilityFlags visibilityFlags = EVisibilityFlags::NONE;
			std::optional<::Rendering::Geometry::BoundingSphere> bounds;
		};


		struct SceneFilteredDrawablesDescriptor
		{
			DrawableMap<EOrderingMode::FRONT_TO_BACK> opaques;
			DrawableMap<EOrderingMode::BACK_TO_FRONT> transparents;
			DrawableMap<EOrderingMode::BACK_TO_FRONT> ui;
		};

		struct SceneDrawablesFilteringInput
		{
			const ::Rendering::Entities::Camera& camera;
			Tools::Utils::OptRef<const ::Rendering::Data::Frustum> frustumerride;
			Tools::Utils::OptRef<::Rendering::Data::Material> errideMaterial;
			Tools::Utils::OptRef<::Rendering::Data::Material> fallbackMaterial;
			EVisibilityFlags requiredVisibilityFlags = EVisibilityFlags::NONE;
			bool includeUI = true; // Whether to include UI drawables in the filtering
			bool includeTransparent = true; // Whether to include transparent drawables in the filtering
			bool includeOpaque = true; // Whether to include opaque drawables in the filtering
		};


		SceneRenderer(::Rendering::Context::Driver& p_driver, bool p_stencilWrite = false);

		virtual void BeginFrame(const ::Rendering::Data::FrameDescriptor& p_frameDescriptor) override;


		virtual void DrawModelWithSingleMaterial(
			::Rendering::Data::PipelineState p_pso,
			::Rendering::Resources::Model& p_model,
			::Rendering::Data::Material& p_material,
			const Maths::FMatrix4& p_modelMatrix
		);

		void Resize(int width,int height);
		SceneDrawablesDescriptor ParseScene(
			const SceneParsingInput& p_input
		);


		SceneFilteredDrawablesDescriptor FilterDrawables(
			const SceneDrawablesDescriptor& p_drawables,
			const SceneDrawablesFilteringInput& p_filteringInput
		);
	};
}
