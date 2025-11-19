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
	/**
	* Extension of the CompositeRenderer adding support for the scene system (parsing/drawing entities)
	*/
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

			/**
			* Determines the order of the drawables.
			* Current order is: order -> distance
			* @param p_other
			*/
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

		/**
		* Input data for the scene renderer.
		*/
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

		/**
		* Result of the scene parsing, containing the drawables to be rendered.
		*/
		struct SceneDrawablesDescriptor
		{
			std::vector<::Rendering::Entities::Drawable> drawables;
		};

		/**
		* Additional information for a drawable computed by the scene renderer.
		*/
		struct SceneDrawableDescriptor
		{
			Core::ECS::Actor& actor;
			EVisibilityFlags visibilityFlags = EVisibilityFlags::NONE;
			std::optional<::Rendering::Geometry::BoundingSphere> bounds;
		};

		/**
		* Filtered drawables for the scene, categorized by their render pass, and sorted by their draw order.
		*/
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

		/**
		* Constructor of the Renderer
		* @param p_driver
		* @param p_stencilWrite (if set to true, also write all the scene geometry to the stencil buffer)
		*/
		SceneRenderer(::Rendering::Context::Driver& p_driver, bool p_stencilWrite = false);

		/**
		* Begin Frame
		* @param p_frameDescriptor
		*/
		virtual void BeginFrame(const ::Rendering::Data::FrameDescriptor& p_frameDescriptor) override;

		/**
		* Draw a model with a single material
		* @param p_pso
		* @param p_model
		* @param p_material
		* @param p_modelMatrix
		*/
		virtual void DrawModelWithSingleMaterial(
			::Rendering::Data::PipelineState p_pso,
			::Rendering::Resources::Model& p_model,
			::Rendering::Data::Material& p_material,
			const Maths::FMatrix4& p_modelMatrix
		);

		/**
		* Parse the scene (as defined in the SceneDescriptor) to find the drawables to render.
		* @param p_sceneDescriptor
		* @param p_options
		*/
		SceneDrawablesDescriptor ParseScene(
			const SceneParsingInput& p_input
		);

		/**
		* Filter and prepare drawables based on the given context.
		* This is where culling and sorting happens.
		* @param p_drawables
		* @param p_filteringInput
		*/
		SceneFilteredDrawablesDescriptor FilterDrawables(
			const SceneDrawablesDescriptor& p_drawables,
			const SceneDrawablesFilteringInput& p_filteringInput
		);
	};
}
