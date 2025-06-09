/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#include <OvCore/ECS/Components/CMaterialRenderer.h>

#include "DebugSceneRenderer.h"
#include "PickingRenderPass.h"

#include "OvCore/Global/ServiceLocator.h"
#include "SceneView.h"
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

	auto headLight = GetScene()->FindActorByName("HeadLight");
	if (!headLight) {
		return;
	}
	headLight->transform.SetWorldPosition(m_camera.GetPosition());
	if (IsSelectActor()) {

		auto& ac = GetSelectedActor();
		//auto bs = ac.GetComponent<::Core::ECS::Components::CPhysicalSphere>();
		auto target = ac.transform.GetWorldPosition();
		auto cp = m_camera.GetPosition();
		float radius = OvMaths::FVector3::Length(target - cp) * 0.5;
		OvMaths::FMatrix4 transMat = OvMaths::FMatrix4::Translation(target - cp);
		OvMaths::FVector3 forward = OvMaths::FVector3::Normalize(cp - target);
		float angle = OvMaths::FVector3::AngleBetween(forward, { 0,1,0 });
		OvMaths::FVector3 worldUp = (angle < FLT_EPSILON || abs(angle - 3.14159274) < FLT_EPSILON) ? OvMaths::FVector3(1, 0, 0) : OvMaths::FVector3(0, 1, 0);
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
	ClearEvents();
	
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
}

OvCore::SceneSystem::Scene* OvEditor::Panels::SceneView::GetScene()
{
	return m_sceneManager.GetCurrentScene();
}

void OvEditor::Panels::SceneView::SetGizmoOperation(OvEditor::Core::EGizmoOperation p_operation)
{
	m_currentOperation = p_operation;
	//EDITOR_EVENT(EditorOperationChanged).Invoke(m_currentOperation);
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

	m_cameraController.ReceiveEvent(e);
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


	//handle pick
	if (t == QEvent::MouseButtonRelease) {
		QMouseEvent* e2 = static_cast<QMouseEvent*>(e);
		if (e2->button() == Qt::LeftButton) {
			m_gizmoOperations.StopPicking();
		}
	}
	std::optional<
		std::variant<OvTools::Utils::OptRef<OvCore::ECS::Actor>,
		OvEditor::Core::GizmoBehaviour::EDirection>
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

OvEditor::Panels::InputState& OvEditor::Panels::SceneView::getInutState()
{
	return input;
}

void OvEditor::Panels::SceneView::ClearEvents()
{
	input.ClearEvents();
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
	auto& pickingPass = m_renderer->GetPass<OvEditor::Rendering::PickingRenderPass>("Picking");

	// Enable picking pass only when the scene view is hovered, not picking, and not operating the camera
	pickingPass.SetEnabled(

		!m_gizmoOperations.IsPicking() &&
		!m_cameraController.IsOperating()
	);

	OvEditor::Panels::AViewControllable::DrawFrame();
	HandleActorPicking();
}

bool IsResizing()
{
	return false;
}

void OvEditor::Panels::SceneView::HandleActorPicking()
{

}

OvEditor::Panels::InputState::InputState()
{
}
namespace OvEditor::Panels {
	KeyState OvEditor::Panels::InputState::GetKeyState(KeyBoard p_key) 
	{
		return m_keyEvents[p_key];
	}

	MouseButtonState InputState::GetMouseButtonState(MouseButton p_button)
	{
		return m_mouseButtonEvents[p_button];
	}

	bool InputState::IsKeyPressed(KeyBoard p_key)
	{
		return m_keyEvents.find(p_key) != m_keyEvents.end() && m_keyEvents.at(p_key) == KeyState::Down;
	}

	bool InputState::IsKeyReleased(KeyBoard p_key) 
	{
		return m_keyEvents.find(p_key) != m_keyEvents.end() && m_keyEvents.at(p_key) == KeyState::Up;
	}

	bool InputState::IsMouseButtonPressed(MouseButton p_button) 
	{
		return m_mouseButtonEvents.find(p_button) != m_mouseButtonEvents.end() && m_mouseButtonEvents.at(p_button) == MouseButtonState::MOUSE_DOWN;
	}

	bool InputState::IsMouseButtonReleased(MouseButton p_button) 
	{
		return m_mouseButtonEvents.find(p_button) != m_mouseButtonEvents.end() && m_mouseButtonEvents.at(p_button) == MouseButtonState::MOUSE_UP;
	}

	std::pair<double, double> InputState::GetMousePosition() 
	{
		return std::pair<double, double>(mouseX,mouseY);
	}

	std::pair<double, double> InputState::GetMouseScroll() 
	{
		return m_scrollData;
	}

	void InputState::ClearEvents()
	{
		m_keyEvents.clear();
		m_mouseButtonEvents.clear();
		m_scrollData = { 0.0, 0.0 };
	}

	void InputState::ReceiveEvent(QEvent* e)
	{
		const QEvent::Type t = e->type();
		if (t == QEvent::KeyPress) {
			QKeyEvent* e2 = static_cast<QKeyEvent*>(e);
			Qt::Key key = static_cast<Qt::Key>(e2->key());
			switch (key)
			{
			case Qt::Key_W: { m_keyEvents[KEYW] = KeyState::Down; }break;
			case Qt::Key_A: { m_keyEvents[KEYA] = KeyState::Down; }break;
			case Qt::Key_S: { m_keyEvents[KEYS] = KeyState::Down; }break;
			case Qt::Key_D: { m_keyEvents[KEYD] = KeyState::Down; }break;
			case Qt::Key_Q: { m_keyEvents[KEYQ] = KeyState::Down; }break;
			case Qt::Key_E: { m_keyEvents[KEYE] = KeyState::Down; }break;
			case Qt::Key_R: { m_keyEvents[KEYR] = KeyState::Down; }break;
			case Qt::Key_F: { m_keyEvents[KEYF] = KeyState::Down; }break;
			case Qt::Key_Alt: { m_keyEvents[ALTA] = KeyState::Down; }break;
			case Qt::Key_Right: { m_keyEvents[RIGHT] = KeyState::Down; }break;
			case Qt::Key_Up: { m_keyEvents[UP] = KeyState::Down; }break;
			case Qt::Key_Down: { m_keyEvents[DOWN] = KeyState::Down; }break;
			case Qt::Key_Left: { m_keyEvents[LEFT] = KeyState::Down; }break;
			case Qt::Key_PageUp: { m_keyEvents[PageUp] = KeyState::Down; }break;
			case Qt::Key_PageDown: { m_keyEvents[PageDown] = KeyState::Down; }break;
			default:
				break;
			}
		}
		else if (t == QEvent::KeyRelease) {
			QKeyEvent* e2 = static_cast<QKeyEvent*>(e);
			Qt::Key key = static_cast<Qt::Key>(e2->key());
			switch (key)
			{
			case Qt::Key_W: { m_keyEvents[KEYW] = KeyState::Up; }break;
			case Qt::Key_A: { m_keyEvents[KEYA] = KeyState::Up; }break;
			case Qt::Key_S: { m_keyEvents[KEYS] = KeyState::Up; }break;
			case Qt::Key_D: { m_keyEvents[KEYD] = KeyState::Up; }break;
			case Qt::Key_Q: { m_keyEvents[KEYQ] = KeyState::Up; }break;
			case Qt::Key_E: { m_keyEvents[KEYE] = KeyState::Up; }break;
			case Qt::Key_R: { m_keyEvents[KEYR] = KeyState::Up; }break;
			case Qt::Key_F: { m_keyEvents[KEYF] = KeyState::Up; }break;
			case Qt::Key_Alt: { m_keyEvents[ALTA] = KeyState::Up; }break;
			case Qt::Key_Right: { m_keyEvents[RIGHT] = KeyState::Up; }break;
			case Qt::Key_Up: { m_keyEvents[UP] = KeyState::Up; }break;
			case Qt::Key_Down: { m_keyEvents[DOWN] = KeyState::Up; }break;
			case Qt::Key_Left: { m_keyEvents[LEFT] = KeyState::Up; }break;
			case Qt::Key_PageUp: { m_keyEvents[PageUp] = KeyState::Up; }break;
			case Qt::Key_PageDown: { m_keyEvents[PageDown] = KeyState::Up; }break;
			default:
				break;
			}
		}
		else if (t == QEvent::MouseMove) {
			QMouseEvent* e2 = static_cast<QMouseEvent*>(e);
			mouseX = e2->x();
			mouseY = e2->y();
		}
		else if (t == QEvent::MouseButtonRelease) {
			QMouseEvent* e2 = static_cast<QMouseEvent*>(e);
			if (e2->button() == Qt::LeftButton) {
				switch (e2->button())
				{
				case Qt::LeftButton: { m_mouseButtonEvents[MOUSE_BUTTON_LEFT] = MouseButtonState::MOUSE_UP; }break;
				case Qt::RightButton: { m_mouseButtonEvents[MOUSE_BUTTON_RIGHT] = MouseButtonState::MOUSE_UP; }break;
				case Qt::MiddleButton: { m_mouseButtonEvents[MOUSE_BUTTON_RIGHT] = MouseButtonState::MOUSE_UP; }break;
				default:
					break;
				}
			}
		}
		else if (t == QEvent::MouseButtonPress) {
			QMouseEvent* e2 = static_cast<QMouseEvent*>(e);
			if (e2->button() == Qt::LeftButton) {
				switch (e2->button())
				{
				case Qt::LeftButton: { m_mouseButtonEvents[MOUSE_BUTTON_LEFT] = MouseButtonState::MOUSE_DOWN; }break;
				case Qt::RightButton: { m_mouseButtonEvents[MOUSE_BUTTON_RIGHT] = MouseButtonState::MOUSE_DOWN; }break;
				case Qt::MiddleButton: { m_mouseButtonEvents[MOUSE_BUTTON_RIGHT] = MouseButtonState::MOUSE_DOWN; }break;
				default:
					break;
				}
			}
		}
		else if (t == QEvent::Wheel) {
			constexpr float kUnitsPerScroll = 1.0f;
			QWheelEvent* e2 = static_cast<QWheelEvent*>(e);
			m_scrollData.second  = 0.002 * static_cast<float>(
				e2->angleDelta().y()
				);
		}
	}

}

