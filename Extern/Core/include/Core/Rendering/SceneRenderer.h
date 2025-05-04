/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <map>

#include <Rendering/Core/CompositeRenderer.h>
#include <Rendering/Resources/Mesh.h>
#include <Rendering/Data/Frustum.h>
#include <Rendering/Entities/Drawable.h>
#include <Rendering/HAL/UniformBuffer.h>
#include <Rendering/HAL/ShaderStorageBuffer.h>

#include "Core/Resources/Material.h"
#include "Core/ECS/Actor.h"
#include "Core/ECS/Components/CCamera.h"
#include "Core/SceneSystem/Scene.h"

namespace Core::Rendering
{
	/**
	* Extension of the CompositeRenderer adding support for the scene system (parsing/drawing entities)
	*/
	class SceneRenderer : public ::Rendering::Core::CompositeRenderer
	{
	public:
		using OpaqueDrawables = std::multimap<float, ::Rendering::Entities::Drawable, std::less<float>>;
		using TransparentDrawables = std::multimap<float, ::Rendering::Entities::Drawable, std::greater<float>>;
		using UIDrawables = std::multimap<float, ::Rendering::Entities::Drawable, std::greater<float>>;

		struct AllDrawables
		{
			OpaqueDrawables opaques;
			TransparentDrawables transparents;
			UIDrawables ui;
		};

		struct SceneDescriptor
		{
			Core::SceneSystem::Scene& scene;
			Tools::Utils::OptRef<const ::Rendering::Data::Frustum> frustumerride;
			Tools::Utils::OptRef<::Rendering::Data::Material> overrideMaterial;
			Tools::Utils::OptRef<::Rendering::Data::Material> fallbackMaterial;
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

	protected:
		AllDrawables ParseScene();
	};
}
