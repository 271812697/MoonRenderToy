/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/Entities/Camera.h>
#include <Rendering/Features/DebugShapeRenderFeature.h>

#include <Core/ECS/Actor.h>
#include <Core/SceneSystem/SceneManager.h>
#include <Core/ECS/Components/CModelRenderer.h>
#include <Core/Resources/Material.h>
#include <Core/ECS/Components/CAmbientBoxLight.h>
#include <Core/ECS/Components/CAmbientSphereLight.h>
#include <Core/Rendering/SceneRenderer.h>

namespace Core::Rendering
{
	/**
	* Draw the scene to a depth buffer from the point of view of each light source
	*/
	class ShadowRenderPass : public ::Rendering::Core::ARenderPass
	{
	public:
		/**
		* Constructor
		* @param p_renderer
		*/
		ShadowRenderPass(::Rendering::Core::CompositeRenderer& p_renderer);

	private:
		virtual void Draw(::Rendering::Data::PipelineState p_pso) override;
		void DrawOpaques(::Rendering::Data::PipelineState p_pso, Core::SceneSystem::Scene& p_scene);

	private:
		Core::Resources::Material m_shadowMaterial;
	};
}