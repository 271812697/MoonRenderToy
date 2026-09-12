#pragma once
#include "GizmoBehaviour.h"
#include "AViewControllable.h"
#include "PickingRenderPass.h"

#include <QElapsedTimer>

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
		Editor::Rendering::PickingRenderPass::PickingResult GetPickResult();
		::Rendering::Geometry::Ray GetMouseRay();
		virtual ::Core::ECS::Actor* GetSelectedActor()override;
		virtual void SelectActor(::Core::ECS::Actor& actor)override;
		virtual void UnselectActor()override;
		virtual bool IsSelectActor()override;
	protected:
		virtual ::Core::Rendering::SceneRenderer::SceneDescriptor CreateSceneDescriptor() override;
	private:
		virtual void DrawFrame() override;
		void HandleActorPicking();
		/** Applies a pick result to the hover / selection state. */
		void ApplyPickResult(const Rendering::PickingRenderPass::PickingResult& p_result);
		/** Synchronous click pick against the picking target drawn this frame. */
		void ResolvePendingClick();
	private:
		int64_t mTargetActorId = -1;
		::Core::SceneSystem::SceneManager& m_sceneManager;
		Editor::Core::GizmoBehaviour m_gizmoOperations;
		Editor::Core::EGizmoOperation m_currentOperation = Editor::Core::EGizmoOperation::TRANSLATE;
		::Core::Resources::Material m_fallbackMaterial;
		Tools::Utils::OptRef<::Core::ECS::Actor> m_highlightedActor;
		std::optional<Editor::Core::GizmoBehaviour::EDirection> m_highlightedGizmoDirection;
		Editor::Rendering::PickingRenderPass::PickingResult pickingResult;
		/** Last mouse position the picking readback was performed for. */
		std::pair<double, double> m_lastPickingMouse{ -1.0, -1.0 };
		/** Throttles hover picks: the picking pass is a full scene pass, so it is
		 * rendered on demand at most every kPickThrottleMs milliseconds. */
		QElapsedTimer m_pickRequestTimer;
		/** View projection of the last pick request (refreshes the target while
		 * the camera moves). */
		Maths::FMatrix4 m_lastPickViewProjection = Maths::FMatrix4::Identity;
		bool m_hasLastPickViewProjection = false;
		/** Left click waiting for the synchronous pick on this frame's target. */
		bool m_clickPickPending = false;
		int m_clickPickX = 0;
		int m_clickPickY = 0;
	};
}
