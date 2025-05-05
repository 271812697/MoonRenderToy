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

#include "Context.h"
#include "GizmoBehaviour.h"

namespace Editor::Rendering
{
	/**
	* Draw a gizmo
	*/
	class GizmoRenderFeature : public ::Rendering::Features::ARenderFeature
	{
	public:
		/**
		* Constructor
		* @param p_renderer
		*/
		GizmoRenderFeature(::Rendering::Core::CompositeRenderer& p_renderer);

		/**
		* Render a gizmo at position
		* @param p_position
		* @param p_rotation
		* @param p_operation
		* @param p_pickable (Determine the shader to use to render the gizmo)
		* @param p_highlightedDirection
		*/
		void DrawGizmo(
			const Maths::FVector3& p_position,
			const Maths::FQuaternion& p_rotation,
			Editor::Core::EGizmoOperation p_operation,
			bool p_pickable,
			std::optional<Editor::Core::GizmoBehaviour::EDirection> p_highlightedDirection
		);

	private:
		::Core::Resources::Material m_gizmoArrowMaterial;
		::Core::Resources::Material m_gizmoBallMaterial;
	};
}