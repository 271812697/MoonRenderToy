
#pragma once

#include "AViewControllable.h"
#include "GizmoBehaviour.h"

class QEvent;
namespace Core::SceneSystem
{
	class SceneManager;
}
namespace Editor::Panels
{
	class SceneView : public Editor::Panels::AViewControllable
	{
	public:
		/**
		* Constructor
		* @param p_title
		* @param p_opened
		* @param p_windowSettings
		*/
		SceneView(
			const std::string& p_title
		);

		/**
		* Update the scene view
		*/
		virtual void Update(float p_deltaTime) override;

		/**
		* Prepare the renderer for rendering
		*/
		virtual void InitFrame() override;

		/**
		* Returns the scene used by this view
		*/
		virtual ::Core::SceneSystem::Scene* GetScene();

		void ReceiveEvent(QEvent* e);
	protected:
		virtual ::Core::Rendering::SceneRenderer::SceneDescriptor CreateSceneDescriptor() override;

	private:
		virtual void DrawFrame() override;
		void HandleActorPicking();

	private:
		::Core::SceneSystem::SceneManager& m_sceneManager;
		Editor::Core::GizmoBehaviour m_gizmoOperations;
		Editor::Core::EGizmoOperation m_currentOperation = Editor::Core::EGizmoOperation::TRANSLATE;
		::Core::Resources::Material m_fallbackMaterial;

		Tools::Utils::OptRef<::Core::ECS::Actor> m_highlightedActor;
		std::optional<Editor::Core::GizmoBehaviour::EDirection> m_highlightedGizmoDirection;
	};
}