#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/ECS/Components/CModelRenderer.h>
#include <Core/SceneSystem/Intersection.h>
#include "DebugSceneRenderer.h"
#include "PickingRenderPass.h"
#include "Core/Global/ServiceLocator.h"
#include "SceneView.h"
#include "Settings/DebugSetting.h"
#include <iostream>
#include <QMouseEvent>

static Maths::FVector3 GetSpherePosition(float a, float b, float radius) {
	float elevation = a / 180.0 * 3.14159265;
	float azimuth = b / 180.0 * 3.14159265;
	return Maths::FVector3(cos(elevation) * sin(azimuth), sin(elevation), cos(elevation) * cos(azimuth)) * radius;
}
namespace
{
	Tools::Utils::OptRef<Core::ECS::Actor> GetActorFromPickingResult(
		Editor::Rendering::PickingRenderPass::PickingResult p_result
	)
	{
		if (p_result)
		{
			if (const auto actor = std::get_if<Tools::Utils::OptRef<Core::ECS::Actor>>(&p_result.value()))
			{
				return *actor;
			}
		}

		return std::nullopt;
	}
}

Editor::Panels::SceneView::SceneView
(
	const std::string& p_title)
	: AViewControllable(p_title),
	m_sceneManager(::Core::Global::ServiceLocator::Get<Editor::Core::Context>().sceneManager)
{
	RegService(Editor::Panels::SceneView, *this);
	m_renderer = std::make_unique<Editor::Rendering::DebugSceneRenderer>(*::Core::Global::ServiceLocator::Get<Editor::Core::Context>().driver);
	m_camera.SetFar(5000.0f);
	m_fallbackMaterial.SetShader(::Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager[":Shaders\\Unlit.ovfx"]);
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
	if (!headLight) {
		return;
	}
	headLight->transform.SetWorldPosition(m_camera.GetPosition());
	if (IsSelectActor()) {
		auto& ac = GetSelectedActor();
		auto name=ac.GetName();
		if (name== "PointLight1"|| name == "PointLight2"|| name == "PointLight3"|| name == "PointLight4") {
			return;
		}
		float pi = 3.14159265359f;
		//auto bs = ac.GetComponent<::Core::ECS::Components::CPhysicalSphere>();
		//ac.GetComponent<:Core::ECS::Components::>();
		auto target = ac.transform.GetWorldPosition();
		auto cp = m_camera.GetPosition();
		float radius = Maths::FVector3::Length(target - cp) ;
		Maths::FMatrix4 transMat = Maths::FMatrix4::Translation(target - cp);
		Maths::FVector3 forward = Maths::FVector3::Normalize(cp - target);
		float angle = Maths::FVector3::AngleBetween(forward, { 0,1,0 });
		Maths::FVector3 worldUp = (angle < FLT_EPSILON || abs(angle - pi) < FLT_EPSILON) ? Maths::FVector3(1, 0, 0) : Maths::FVector3(0, 1, 0);
		Maths::FMatrix4 view = Maths::FMatrix4::CreateCameraView(cp, target, worldUp);
		view = Maths::FMatrix4::Inverse(view);

		Maths::FMatrix4 mat = transMat * view;

		auto p1 = Maths::FMatrix4::MulPoint(mat, GetSpherePosition(50, 10, radius));
		auto p2 = Maths::FMatrix4::MulPoint(mat, GetSpherePosition(-75, 10, radius));
		auto p3 = Maths::FMatrix4::MulPoint(mat, GetSpherePosition(0, 110, radius));
		auto p4 = Maths::FMatrix4::MulPoint(mat, GetSpherePosition(0, -110, radius));
		GetScene()->FindActorByName("PointLight1")->transform.SetWorldPosition(p1);
		GetScene()->FindActorByName("PointLight2")->transform.SetWorldPosition(p2);
		GetScene()->FindActorByName("PointLight3")->transform.SetWorldPosition(p3);
		GetScene()->FindActorByName("PointLight4")->transform.SetWorldPosition(p4);
	}
	GetScene()->Update(p_deltaTime);
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
	// Enable picking pass only when the scene view is hered, not picking, and not operating the camera
	auto& pickingPass = m_renderer->GetPass<Editor::Rendering::PickingRenderPass>("Picking");
	pickingPass.SetEnabled(
		!m_gizmoOperations.IsPicking() &&
		!m_cameraController.IsOperating()
	);
}

Core::SceneSystem::Scene* Editor::Panels::SceneView::GetScene()
{
	return m_sceneManager.GetCurrentScene();
}

void Editor::Panels::SceneView::FitToSelectedActor(const Maths::FVector3& dir)
{
	if (mTargetActor) {
		auto modelRenderer = mTargetActor->GetComponent<::Core::ECS::Components::CModelRenderer>();
		if (modelRenderer) {
			auto model=modelRenderer->GetModel();
			if (model) {
				auto transform=mTargetActor->GetComponent<::Core::ECS::Components::CTransform>();
				auto sphere=modelRenderer->GetModel()->GetBoundingSphere();
				sphere.position=Maths::FMatrix4::MulPoint(transform->GetWorldMatrix(), sphere.position);
				
				auto scale = transform->GetWorldScale();
				sphere.radius*=scale.Max();
				m_camera.ProjectionFitToSphere(sphere,dir);

				float pi = 3.14159265359f;
				Maths::FVector3 forward = dir;
				float angle = Maths::FVector3::AngleBetween(forward, { 0,1,0 });
				Maths::FVector3 up = (angle < FLT_EPSILON || abs(angle - pi) < FLT_EPSILON) ? Maths::FVector3(1, 0, 0) : Maths::FVector3(0, 1, 0);
				Maths::FQuaternion quat=  Maths::FQuaternion::LookAt(forward, up);
				float eff = pi/ 180.0;
				
				if (m_camera.GetProjectionMode() == ::Rendering::Settings::EProjectionMode::ORTHOGRAPHIC) {
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


void Editor::Panels::SceneView::FitToScene(const Maths::FVector3& dir)
{
	//m_camera.ProjectionFitToSphere->the code make no sence
	auto& models = GetScene()->GetFastAccessComponents().modelRenderers;
	if (models.size()>0) {
		::Rendering::Geometry::BoundingSphere sphere=models[0]->GetModel()->GetBoundingSphere();
		for (size_t i = 1; i < models.size(); i++)
		{
			sphere.merge(models[i]->GetModel()->GetBoundingSphere());
		}	
		//m_camera.ProjectionFitToSphere(sphere, dir);
	}

}

void Editor::Panels::SceneView::BuildBvh()
{
	auto& scene = *GetScene();

	//scene.BuildBVH();
}

void Editor::Panels::SceneView::SetGizmoOperation(Editor::Core::EGizmoOperation p_operation)
{
	m_currentOperation = p_operation;
}

Editor::Core::EGizmoOperation Editor::Panels::SceneView::GetGizmoOperation() const
{
	return m_currentOperation;
}

void Editor::Panels::SceneView::ReceiveEvent(QEvent* e)
{
	if (e == nullptr)
		return;
	input.ReceiveEvent(e);
	const QEvent::Type t = e->type();
    if (t == QEvent::MouseButtonPress) {
		QMouseEvent* e2 = static_cast<QMouseEvent*>(e);
		if(e2->button()== Qt::RightButton)
		{ 
			MouseHit(m_roaterCenter);
		}
	}
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
}

bool Editor::Panels::SceneView::MouseHit(Maths::FVector3& out)
{
	auto[x,y]=input.GetMousePosition();
	auto ray = GetCamera()->GetMouseRay(x, y);
	::Core::SceneSystem::HitRes res;
	bool flag=MOON::DebugSettings::instance().getOrDefault<bool>("BvhRayHit",false);
	if (!flag) { 
		if (GetScene()->RayIteratorHit(ray, res)){
			out=res.hitPoint;
			return true;
		}
		return false;
	}
	if (GetScene()->RayHit(ray, res)) {
		out = res.hitPoint;
		return true;
	}
	return false;
}

::Rendering::Geometry::Ray Editor::Panels::SceneView::GetMouseRay()
{
	
	auto [x, y] = input.GetMousePosition();
	return GetCamera()->GetMouseRay(x, y);
}


Core::Rendering::SceneRenderer::SceneDescriptor Editor::Panels::SceneView::CreateSceneDescriptor()
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
	
	if (m_gizmoOperations.IsPicking()&&input.IsMouseButtonReleased(MouseButton::MOUSE_BUTTON_LEFT))
	{
		m_gizmoOperations.StopPicking();
		GetScene()->BuildSceneBvh();
	}

	if (!m_gizmoOperations.IsPicking())
	{
		auto mousePos = input.GetMousePosition();
		int mouseY = GetSafeSize().second - mousePos.second - 1;
		int mouseX = mousePos.first;
		auto& scene = *GetScene();
		auto& actorPickingPass = m_renderer->GetPass<Rendering::PickingRenderPass>("Picking");
		const auto pickingResult = actorPickingPass.ReadbackPickingResult(
			scene,
			static_cast<uint32_t>(mouseX),
			static_cast<uint32_t>(mouseY)
		);


		m_highlightedActor = {};
		m_highlightedGizmoDirection = {};

		if ( pickingResult.has_value())
		{
			if (const auto pval = std::get_if<Tools::Utils::OptRef<::Core::ECS::Actor>>(&pickingResult.value()))
			{
				m_highlightedActor = *pval;
			}
			else if (const auto pval = std::get_if<Editor::Core::GizmoBehaviour::EDirection>(&pickingResult.value()))
			{
				m_highlightedGizmoDirection = *pval;
			}
		}
		else
		{
			m_highlightedActor = {};
			m_highlightedGizmoDirection = {};
		}

		if (input.IsMouseButtonPressed(MouseButton::MOUSE_BUTTON_LEFT) )
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


