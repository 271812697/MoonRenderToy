#include <OvCore/ECS/Components/CMaterialRenderer.h>
#include <OvCore/ECS/Components/CModelRenderer.h>
#include "DebugSceneRenderer.h"
#include "PickingRenderPass.h"
#include "OvCore/Global/ServiceLocator.h"
#include "SceneView.h"
#include <iostream>
#include <QMouseEvent>

static OvMaths::FVector3 GetSpherePosition(float a, float b, float radius) {
	float elevation = a / 180.0 * 3.14159265;
	float azimuth = b / 180.0 * 3.14159265;
	return OvMaths::FVector3(cos(elevation) * sin(azimuth), sin(elevation), cos(elevation) * cos(azimuth)) * radius;
}
namespace
{
	OvTools::Utils::OptRef<OvCore::ECS::Actor> GetActorFromPickingResult(
		OvEditor::Rendering::PickingRenderPass::PickingResult p_result
	)
	{
		if (p_result)
		{
			if (const auto actor = std::get_if<OvTools::Utils::OptRef<OvCore::ECS::Actor>>(&p_result.value()))
			{
				return *actor;
			}
		}

		return std::nullopt;
	}
}

OvEditor::Panels::SceneView::SceneView
(
	const std::string& p_title)
	: AViewControllable(p_title),
	m_sceneManager(OvCore::Global::ServiceLocator::Get<OvEditor::Core::Context>().sceneManager)
{
	m_renderer = std::make_unique<OvEditor::Rendering::DebugSceneRenderer>(*OvCore::Global::ServiceLocator::Get<OvEditor::Core::Context>().driver);
	m_camera.SetFar(5000.0f);
	m_fallbackMaterial.SetShader(OvCore::Global::ServiceLocator::Get<OvEditor::Core::Context>().shaderManager[":Shaders\\Unlit.ovfx"]);
	m_fallbackMaterial.SetProperty("u_Diffuse", OvMaths::FVector4{ 1.f, 0.f, 1.f, 1.0f });
	m_fallbackMaterial.SetProperty("u_DiffuseMap", static_cast<OvRendering::Resources::Texture*>(nullptr));
	OvCore::ECS::Actor::DestroyedEvent += [this](const OvCore::ECS::Actor& actor)
		{
			if (m_highlightedActor.has_value() && m_highlightedActor.value().GetID() == actor.GetID())
			{
				m_highlightedActor.reset();
			}
		};
}

void OvEditor::Panels::SceneView::Update(float p_deltaTime)
{
	AViewControllable::Update(p_deltaTime);
	input.ClearEvents();
	auto headLight = GetScene()->FindActorByName("HeadLight");
	if (!headLight) {
		return;
	}
	headLight->transform.SetWorldPosition(m_camera.GetPosition());
	if (IsSelectActor()) {
		float pi = 3.14159265359f;
		auto& ac = GetSelectedActor();
		//auto bs = ac.GetComponent<::Core::ECS::Components::CPhysicalSphere>();
		//ac.GetComponent<:Core::ECS::Components::>();
		auto target = ac.transform.GetWorldPosition();
		auto cp = m_camera.GetPosition();
		float radius = OvMaths::FVector3::Length(target - cp) ;
		OvMaths::FMatrix4 transMat = OvMaths::FMatrix4::Translation(target - cp);
		OvMaths::FVector3 forward = OvMaths::FVector3::Normalize(cp - target);
		float angle = OvMaths::FVector3::AngleBetween(forward, { 0,1,0 });
		OvMaths::FVector3 worldUp = (angle < FLT_EPSILON || abs(angle - pi) < FLT_EPSILON) ? OvMaths::FVector3(1, 0, 0) : OvMaths::FVector3(0, 1, 0);
		OvMaths::FMatrix4 view = OvMaths::FMatrix4::CreateCameraView(cp, target, worldUp);
		view = OvMaths::FMatrix4::Inverse(view);

		OvMaths::FMatrix4 mat = transMat * view;

		auto p1 = OvMaths::FMatrix4::MulPoint(mat, GetSpherePosition(50, 10, radius));
		auto p2 = OvMaths::FMatrix4::MulPoint(mat, GetSpherePosition(-75, 10, radius));
		auto p3 = OvMaths::FMatrix4::MulPoint(mat, GetSpherePosition(0, 110, radius));
		auto p4 = OvMaths::FMatrix4::MulPoint(mat, GetSpherePosition(0, -110, radius));
		GetScene()->FindActorByName("PointLight1")->transform.SetWorldPosition(p1);
		GetScene()->FindActorByName("PointLight2")->transform.SetWorldPosition(p2);
		GetScene()->FindActorByName("PointLight3")->transform.SetWorldPosition(p3);
		GetScene()->FindActorByName("PointLight4")->transform.SetWorldPosition(p4);
	}
}

void OvEditor::Panels::SceneView::InitFrame()
{
	AViewControllable::InitFrame();

	OvTools::Utils::OptRef<OvCore::ECS::Actor> selectedActor;

	if (mTargetActor != nullptr) {
		selectedActor = GetSelectedActor();
	}

	m_renderer->AddDescriptor<Rendering::DebugSceneRenderer::DebugSceneDescriptor>({
		m_currentOperation,
		m_highlightedActor,
		selectedActor,
		m_highlightedGizmoDirection
		});
	// Enable picking pass only when the scene view is hovered, not picking, and not operating the camera
	auto& pickingPass = m_renderer->GetPass<OvEditor::Rendering::PickingRenderPass>("Picking");
	pickingPass.SetEnabled(

		!m_gizmoOperations.IsPicking() &&
		!m_cameraController.IsOperating()
	);
}

OvCore::SceneSystem::Scene* OvEditor::Panels::SceneView::GetScene()
{
	return m_sceneManager.GetCurrentScene();
}

void OvEditor::Panels::SceneView::FitToSelectedActor(const OvMaths::FVector3& dir)
{
	if (mTargetActor) {
		auto modelRenderer = mTargetActor->GetComponent<OvCore::ECS::Components::CModelRenderer>();
		if (modelRenderer) {
			auto model=modelRenderer->GetModel();
			if (model) {
				auto transform=mTargetActor->GetComponent<OvCore::ECS::Components::CTransform>();
				auto sphere=modelRenderer->GetModel()->GetBoundingSphere();
				sphere.position=OvMaths::FMatrix4::MulPoint(transform->GetWorldMatrix(), sphere.position);
				m_camera.ProjectionFitToSphere(sphere,dir);

				float pi = 3.14159265359f;
				OvMaths::FVector3 forward = dir;
				float angle = OvMaths::FVector3::AngleBetween(forward, { 0,1,0 });
				OvMaths::FVector3 up = (angle < FLT_EPSILON || abs(angle - pi) < FLT_EPSILON) ? OvMaths::FVector3(1, 0, 0) : OvMaths::FVector3(0, 1, 0);
				OvMaths::FQuaternion quat=  OvMaths::FQuaternion::LookAt(forward, up);
				float eff = pi/ 180.0;
				
				if (m_camera.GetProjectionMode() == OvRendering::Settings::EProjectionMode::ORTHOGRAPHIC) {
					this->GetCameraController().MoveToPose(sphere.position - dir * sphere.radius,quat);
				}
				else
				{
					float distance = sphere.radius / std::sin(eff * m_camera.GetFov() / 2.0f);
				    this->GetCameraController().MoveToPose(sphere.position - dir * distance,quat);
				}
			}
		}
	}
}


void OvEditor::Panels::SceneView::FitToScene(const OvMaths::FVector3& dir)
{
	//m_camera.ProjectionFitToSphere->the code make no sence
	auto& models = GetScene()->GetFastAccessComponents().modelRenderers;
	if (models.size()>0) {
		OvRendering::Geometry::BoundingSphere sphere=models[0]->GetModel()->GetBoundingSphere();
		for (size_t i = 1; i < models.size(); i++)
		{
			sphere.merge(models[i]->GetModel()->GetBoundingSphere());
		}	
		//m_camera.ProjectionFitToSphere(sphere, dir);
	}

}

void OvEditor::Panels::SceneView::SetGizmoOperation(OvEditor::Core::EGizmoOperation p_operation)
{
	m_currentOperation = p_operation;
}

OvEditor::Core::EGizmoOperation OvEditor::Panels::SceneView::GetGizmoOperation() const
{
	return m_currentOperation;
}

void OvEditor::Panels::SceneView::ReceiveEvent(QEvent* e)
{
	if (e == nullptr)
		return;
	input.ReceiveEvent(e);
	const QEvent::Type t = e->type();
	if (!m_cameraController.IsRightMousePressed()) {
		if (t == QEvent::KeyPress) {
			QKeyEvent* e2 = static_cast<QKeyEvent*>(e);
			Qt::Key key = static_cast<Qt::Key>(e2->key());
			if (key == Qt::Key_W) {
				m_currentOperation = OvEditor::Core::EGizmoOperation::TRANSLATE;
			}
			else if (key == Qt::Key_E) {
				m_currentOperation = OvEditor::Core::EGizmoOperation::ROTATE;
			}
			else if (key == Qt::Key_R) {
				m_currentOperation = OvEditor::Core::EGizmoOperation::SCALE;
			}
		}
	}
}


OvCore::Rendering::SceneRenderer::SceneDescriptor OvEditor::Panels::SceneView::CreateSceneDescriptor()
{
	auto descriptor = AViewControllable::CreateSceneDescriptor();
	descriptor.fallbackMaterial = m_fallbackMaterial;

	if (false)
	{
		auto& scene = *GetScene();

		if (auto mainCameraComponent = scene.FindMainCamera())
		{
			auto& sceneCamera = mainCameraComponent->GetCamera();
			m_camera.SetFrustumGeometryCulling(sceneCamera.HasFrustumGeometryCulling());
			m_camera.SetFrustumLightCulling(sceneCamera.HasFrustumLightCulling());
			descriptor.frustumOverride = sceneCamera.GetFrustum();
		}
	}

	return descriptor;
}

void OvEditor::Panels::SceneView::DrawFrame()
{
	OvEditor::Panels::AViewControllable::DrawFrame();
	HandleActorPicking();
}

bool IsResizing()
{
	return false;
}

void OvEditor::Panels::SceneView::HandleActorPicking()
{
	if (input.IsMouseButtonReleased(MouseButton::MOUSE_BUTTON_LEFT))
	{
		m_gizmoOperations.StopPicking();
	}

	if (!m_gizmoOperations.IsPicking())
	{
		auto mousePos = input.GetMousePosition();
		int mouseY = GetSafeSize().second - mousePos.second - 1;
		int mouseX = mousePos.first;
		auto& scene = *GetScene();
		auto& actorPickingFeature = m_renderer->GetPass<Rendering::PickingRenderPass>("Picking");
		const auto pickingResult = actorPickingFeature.ReadbackPickingResult(
			scene,
			static_cast<uint32_t>(mouseX),
			static_cast<uint32_t>(mouseY)
		);


		m_highlightedActor = {};
		m_highlightedGizmoDirection = {};

		if (!m_cameraController.IsRightMousePressed() && pickingResult.has_value())
		{
			if (const auto pval = std::get_if<OvTools::Utils::OptRef<OvCore::ECS::Actor>>(&pickingResult.value()))
			{
				m_highlightedActor = *pval;
			}
			else if (const auto pval = std::get_if<OvEditor::Core::GizmoBehaviour::EDirection>(&pickingResult.value()))
			{
				m_highlightedGizmoDirection = *pval;
			}
		}
		else
		{
			m_highlightedActor = {};
			m_highlightedGizmoDirection = {};
		}

		if (input.IsMouseButtonPressed(MouseButton::MOUSE_BUTTON_LEFT) && !m_cameraController.IsRightMousePressed())
		{
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
	else
	{
		m_highlightedActor = std::nullopt;
		m_highlightedGizmoDirection = std::nullopt;
	}

	if (m_gizmoOperations.IsPicking())
	{
		auto [winWidth, winHeight] = GetSafeSize();
		auto mousePosition = input.GetMousePosition();
		m_gizmoOperations.SetCurrentMouse({ static_cast<float>(mousePosition.first), static_cast<float>(mousePosition.second) });
		m_gizmoOperations.ApplyOperation(m_camera.GetViewMatrix(), m_camera.GetProjectionMatrix(), m_camera.GetPosition(), { static_cast<float>(winWidth), static_cast<float>(winHeight) });

	}
}


