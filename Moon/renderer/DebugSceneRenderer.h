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
#include "GizmoBehaviour.h"

#include "Context.h"

namespace Editor::Panels { class AView; }

namespace Editor::Rendering
{
	/**
	* Pride a debug layer on top of the default scene renderer to see "invisible" entities such as
	* lights, cameras,
	*/
	class DebugSceneRenderer : public ::Core::Rendering::SceneRenderer
	{
	public:
		struct DebugSceneDescriptor
		{
			Editor::Core::EGizmoOperation gizmoOperation;
			Tools::Utils::OptRef<::Core::ECS::Actor> highlightedActor;
			Tools::Utils::OptRef<::Core::ECS::Actor> selectedActor;
			std::optional<Editor::Core::GizmoBehaviour::EDirection> highlightedGizmoDirection;
		};

		/**
		* Constructor of the Renderer
		* @param p_driver
		*/
		DebugSceneRenderer(::Rendering::Context::Driver& p_driver);
	};
}