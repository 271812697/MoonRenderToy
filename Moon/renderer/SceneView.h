#pragma once
#include "GizmoBehaviour.h"
#include "AViewControllable.h"
#include "PickingRenderPass.h"
namespace Editor::Panels
{

	class SceneView : public Editor::Panels::AViewControllable
	{
	public:
		SceneView(
			const std::string& p_title
		);

		virtual void Update(float p_deltaTime) override;
		virtual void InitFrame() override;
		virtual ::Core::SceneSystem::Scene* GetScene();
		void FitToSelectedActor(const Maths::FVector3& dir);
		void LookAt(const Maths::FVector3& pivot,const Maths::FVector3& dir,float radius);
		Maths::FVector2 worldToScreen(const Maths::FVector3& worldPos);
		void FitToScene(const Maths::FVector3& dir);
		void BuildBvh();
		void SetGizmoOperation(Core::EGizmoOperation p_operation);
		Core::EGizmoOperation GetGizmoOperation() const;
		void ReceiveEvent(QEvent* e);
		bool MouseHit(Maths::FVector3& out);
		bool MouseClipHit(Maths::FVector3& out,const Maths::FVector4& clipPlane);
		::Rendering::Geometry::Ray GetMouseRay();
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