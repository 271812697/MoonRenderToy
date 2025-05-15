#include "DebugSceneRenderer.h"
#include "PickingRenderPass.h"

#include "Core/Global/ServiceLocator.h"
#include "Core/SceneSystem/SceneManager.h"
#include "SceneView.h"
#include "core/ECS/Components/CPhysicalSphere.h"
#include <QMouseEvent>
namespace Editor::Panels
{
	Maths::FVector3 GetSpherePosition(float a, float b, float radius) {

		float elevation = a / 180.0 * 3.14159265;
		float azimuth = b / 180.0 * 3.14159265;
		return Maths::FVector3(cos(elevation) * sin(azimuth), sin(elevation), cos(elevation) * cos(azimuth)) * radius;
	}

	Editor::Panels::SceneView::SceneView
	(
		const std::string& p_title
	) : AViewControllable(p_title),
		m_sceneManager(::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().sceneManager)
	{
		m_renderer = std::make_unique<::Editor::Rendering::DebugSceneRenderer>(*::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().driver);

		m_camera.SetFar(5000.0f);

		m_fallbackMaterial.SetShader(::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().shaderManager[":Shaders\\Unlit.ovfx"]);
		m_fallbackMaterial.SetProperty("u_Diffuse", Maths::FVector4{ 1.f, 0.f, 1.f, 1.0f });
		m_fallbackMaterial.SetProperty("u_DiffuseMap", static_cast<::Rendering::Resources::Texture*>(nullptr));


		::Core::ECS::Actor::DestroyedEvent += [this](const ::Core::ECS::Actor& actor)
			{
				if (m_highlightedActor.has_value() && m_highlightedActor.value().GetID() == actor.GetID())
				{
					m_highlightedActor.reset();
				}
			};
	}

	void Editor::Panels::SceneView::Update(float p_deltaTime)
	{
		AViewControllable::Update(p_deltaTime);
		auto headLight = GetScene()->FindActorByName("HeadLight");
		headLight->transform.SetWorldPosition(m_camera.GetPosition());
		if (IsSelectActor()) {
			auto& ac = GetSelectedActor();
			//auto bs = ac.GetComponent<::Core::ECS::Components::CPhysicalSphere>();
			float radius = Maths::FVector3::Length(ac.transform.GetWorldPosition() - m_camera.GetPosition()) * 3.5;
			Maths::FMatrix4 transMat = Maths::FMatrix4::Translation(ac.transform.GetWorldPosition() - m_camera.GetPosition());
			Maths::FVector3 forward = Maths::FVector3::Normalize(m_camera.GetPosition() - ac.transform.GetWorldPosition());
			Maths::FVector3 worldUp = Maths::FVector3::AngleBetween(forward, { 0,1,0 }) < FLT_EPSILON ? Maths::FVector3(1, 0, 0) : Maths::FVector3(0, 1, 0);
			Maths::FMatrix4 view = Maths::FMatrix4::CreateCameraView(m_camera.GetPosition(), ac.transform.GetWorldPosition(), worldUp);
			view = Maths::FMatrix4::Inverse(view);

			Maths::FMatrix4 mat = transMat * view;

			auto p1 = Maths::FMatrix4::MulPoint(mat, GetSpherePosition(50, 10, radius));
			auto p2 = Maths::FMatrix4::MulPoint(mat, GetSpherePosition(-75, 10, radius));
			auto p3 = Maths::FMatrix4::MulPoint(mat, GetSpherePosition(0, 110, radius));
			auto p4 = Maths::FMatrix4::MulPoint(mat, GetSpherePosition(0, -110, radius));
			auto target = ac.transform.GetWorldPosition();
			auto cp = m_camera.GetPosition();
			//std::cout << Maths::FVector3::Length(p1 - cp) << ":" << Maths::FVector3::Length(p2) << ":" << Maths::FVector3::Length(p3) << ":" << Maths::FVector3::Length(p4) << std::endl;;
			std::cout << Maths::FVector3::Length(p1 - target) << ":" << Maths::FVector3::Length(p2 - target) << ":" << Maths::FVector3::Length(p3 - target) << ":" << Maths::FVector3::Length(p4 - target) << std::endl;;
			GetScene()->FindActorByName("PointLight1")->transform.SetWorldPosition(p1);
			GetScene()->FindActorByName("PointLight2")->transform.SetWorldPosition(p2);
			GetScene()->FindActorByName("PointLight3")->transform.SetWorldPosition(p3);
			GetScene()->FindActorByName("PointLight4")->transform.SetWorldPosition(p4);
		}


	}

	void Editor::Panels::SceneView::InitFrame()
	{
		AViewControllable::InitFrame();

		Tools::Utils::OptRef<::Core::ECS::Actor> selectedActor;
		if (mTargetActor != nullptr) {
			selectedActor = GetSelectedActor();
		}

		m_renderer->AddDescriptor<Rendering::DebugSceneRenderer::DebugSceneDescriptor>({
			m_currentOperation,
			m_highlightedActor,
			selectedActor,
			m_highlightedGizmoDirection
			});
	}

	::Core::SceneSystem::Scene* Editor::Panels::SceneView::GetScene()
	{
		return m_sceneManager.GetCurrentScene();
	}

	void SceneView::ReceiveEvent(QEvent* e)
	{
		if (e == nullptr)
			return;
		const QEvent::Type t = e->type();

		m_cameraController.ReceiveEvent(e);
		if (!m_cameraController.IsRightMousePressed()) {
			if (t == QEvent::KeyPress) {
				QKeyEvent* e2 = static_cast<QKeyEvent*>(e);
				Qt::Key key = static_cast<Qt::Key>(e2->key());
				if (key == Qt::Key_W) {
					m_currentOperation = Editor::Core::EGizmoOperation::TRANSLATE;
				}
				else if (key == Qt::Key_E) {
					m_currentOperation = Editor::Core::EGizmoOperation::ROTATE;
				}
				else if (key == Qt::Key_R) {
					m_currentOperation = Editor::Core::EGizmoOperation::SCALE;
				}
			}

		}


		//handle pick
		if (t == QEvent::MouseButtonRelease) {
			QMouseEvent* e2 = static_cast<QMouseEvent*>(e);
			if (e2->button() == Qt::LeftButton) {
				m_gizmoOperations.StopPicking();
			}
		}
		std::optional<
			std::variant<Tools::Utils::OptRef<::Core::ECS::Actor>,
			Editor::Core::GizmoBehaviour::EDirection>
		>pickingResult = std::nullopt;
		if (t == QEvent::MouseMove) {
			QMouseEvent* e2 = static_cast<QMouseEvent*>(e);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			auto mouseX = e2->x();

			auto mouseY = e2->y();
#else
			auto x = e2->localPos().x();
			auto y = e2->localPos().y();
#endif

			mouseY = GetSafeSize().second - mouseY - 1;

			auto& scene = *GetScene();

			auto& actorPickingFeature = m_renderer->GetPass<Rendering::PickingRenderPass>("Picking");

			pickingResult = actorPickingFeature.ReadbackPickingResult(
				scene,
				static_cast<uint32_t>(mouseX),
				static_cast<uint32_t>(mouseY)
			);
			m_highlightedActor = {};
			m_highlightedGizmoDirection = {};

			if (!m_cameraController.IsRightMousePressed() && pickingResult.has_value())
			{
				if (const auto pval = std::get_if<Tools::Utils::OptRef<::Core::ECS::Actor>>(&pickingResult.value()))
				{
					m_highlightedActor = *pval;
				}
				else if (const auto pval = std::get_if<::Editor::Core::GizmoBehaviour::EDirection>(&pickingResult.value()))
				{
					m_highlightedGizmoDirection = *pval;
				}
			}
			else
			{
				m_highlightedActor = {};
				m_highlightedGizmoDirection = {};
			}
		}

		if (t == QEvent::MouseButtonPress) {
			QMouseEvent* e2 = static_cast<QMouseEvent*>(e);
			if (e2->button() == Qt::LeftButton && !m_cameraController.IsRightMousePressed()) {
				if (m_highlightedGizmoDirection)
				{
					m_gizmoOperations.StartPicking(
						GetSelectedActor(),
						m_camera.GetPosition(),
						m_currentOperation,
						m_highlightedGizmoDirection.value());
				}
				else if (m_highlightedActor)
				{
					SelectActor(m_highlightedActor.value());
				}
				else
				{
					UnselectActor();
				}
			}
		}


		if (t == QEvent::MouseMove) {
			QMouseEvent* e2 = static_cast<QMouseEvent*>(e);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			auto mouseX = e2->x();

			auto mouseY = e2->y();
#else
			auto x = e2->localPos().x();
			auto y = e2->localPos().y();
#endif
			if (m_gizmoOperations.IsPicking())
			{
				auto [winWidth, winHeight] = GetSafeSize();

				m_gizmoOperations.SetCurrentMouse({ static_cast<float>(mouseX), static_cast<float>(mouseY) });
				m_gizmoOperations.ApplyOperation(m_camera.GetViewMatrix(), m_camera.GetProjectionMatrix(), m_camera.GetPosition(), { static_cast<float>(winWidth), static_cast<float>(winHeight) });
			}
		}


	}

	::Core::Rendering::SceneRenderer::SceneDescriptor Editor::Panels::SceneView::CreateSceneDescriptor()
	{
		auto descriptor = AViewControllable::CreateSceneDescriptor();
		descriptor.fallbackMaterial = m_fallbackMaterial;
		//don't debug the scene camera
		if (false)
		{
			auto& scene = *GetScene();

			if (auto mainCameraComponent = scene.FindMainCamera())
			{
				auto& sceneCamera = mainCameraComponent->GetCamera();
				m_camera.SetFrustumGeometryCulling(sceneCamera.HasFrustumGeometryCulling());
				m_camera.SetFrustumLightCulling(sceneCamera.HasFrustumLightCulling());
				descriptor.frustumerride = sceneCamera.GetFrustum();
			}
		}
		return descriptor;
	}

	void Editor::Panels::SceneView::DrawFrame()
	{
		Editor::Panels::AViewControllable::DrawFrame();
		HandleActorPicking();
	}

	bool IsResizing()
	{
		return false;
	}

	void Editor::Panels::SceneView::HandleActorPicking()
	{


	}

}
