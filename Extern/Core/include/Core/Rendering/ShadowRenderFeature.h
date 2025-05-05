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
	* Draw the scene for actor picking
	*/
	class ShadowRenderFeature : public ::Rendering::Features::ARenderFeature
	{
	public:
		/**
		* Constructor
		* @param p_renderer
		*/
		ShadowRenderFeature(::Rendering::Core::CompositeRenderer& p_renderer);

	protected:
		virtual void OnBeforeDraw(::Rendering::Data::PipelineState& p_pso, const ::Rendering::Entities::Drawable& p_drawable);
		virtual void OnAfterDraw(::Rendering::Data::PipelineState& p_pso, const ::Rendering::Entities::Drawable& p_drawable);
	};
}