#pragma once
#include "GizmoBehaviour.h"
#include "AViewControllable.h"
#include "PickingRenderPass.h"

namespace OvEditor::Panels
{

	class SceneView : public OvEditor::Panels::AViewControllable
	{
	public:
		SceneView(
			const std::string& p_title
		);

		virtual void Update(float p_deltaTime) override;
		virtual void InitFrame() override;
		virtual OvCore::SceneSystem::Scene* GetScene();
		void FitToSelectedActor(const OvMaths::FVector3& dir);
		void FitToScene(const OvMaths::FVector3& dir);
		void SetGizmoOperation(Core::EGizmoOperation p_operation);
		Core::EGizmoOperation GetGizmoOperation() const;
		void ReceiveEvent(QEvent* e);
	protected:
		virtual OvCore::Rendering::SceneRenderer::SceneDescriptor CreateSceneDescriptor() override;

	private:
		virtual void DrawFrame() override;
		void HandleActorPicking();
	private:
		OvCore::SceneSystem::SceneManager& m_sceneManager;
		OvEditor::Core::GizmoBehaviour m_gizmoOperations;
		OvEditor::Core::EGizmoOperation m_currentOperation = OvEditor::Core::EGizmoOperation::TRANSLATE;
		OvCore::Resources::Material m_fallbackMaterial;

		OvTools::Utils::OptRef<OvCore::ECS::Actor> m_highlightedActor;
		std::optional<OvEditor::Core::GizmoBehaviour::EDirection> m_highlightedGizmoDirection;

	};
}