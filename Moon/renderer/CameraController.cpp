/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/


#include <Core/ECS/Components/CAmbientBoxLight.h>
#include <Core/ECS/Components/CAmbientSphereLight.h>
#include <Core/ECS/Components/CPhysicalBox.h>
#include <Core/ECS/Components/CPhysicalCapsule.h>
#include <Core/ECS/Components/CPhysicalSphere.h>
#include "CameraController.h"
#include <QMouseEvent>


::Editor::Core::CameraController::CameraController(
	Editor::Panels::AView& p_view,
	Rendering::Entities::Camera& p_camera
) :

	m_view(p_view),
	m_camera(p_camera)
{
	m_camera.SetFov(60.0f);
	mKeyState[KEYW] = Up;
	mKeyState[KEYA] = Up;
	mKeyState[KEYS] = Up;
	mKeyState[KEYD] = Up;
	mKeyState[ALTA] = Up;
	mKeyState[KEYQ] = Up;
	mKeyState[KEYE] = Up;
}

float GetActorFocusDist(Core::ECS::Actor& p_actor)
{
	float distance = 4.0f;

	if (p_actor.IsActive())
	{
		if (auto pb = p_actor.GetComponent<Core::ECS::Components::CPhysicalBox>())
		{
			distance = std::max(distance, std::max
			(
				std::max
				(
					pb->GetSize().x * p_actor.transform.GetWorldScale().x,
					pb->GetSize().y * p_actor.transform.GetWorldScale().y
				),
				pb->GetSize().z * p_actor.transform.GetWorldScale().z
			) * 1.5f);
		}

		if (auto ps = p_actor.GetComponent<Core::ECS::Components::CPhysicalSphere>())
		{
			distance = std::max(distance, std::max
			(
				std::max
				(
					ps->GetRadius() * p_actor.transform.GetWorldScale().x,
					ps->GetRadius() * p_actor.transform.GetWorldScale().y
				),
				ps->GetRadius() * p_actor.transform.GetWorldScale().z
			) * 1.5f);
		}

		if (auto pc = p_actor.GetComponent<Core::ECS::Components::CPhysicalCapsule>())
		{
			distance = std::max(distance, std::max
			(
				std::max
				(
					pc->GetRadius() * p_actor.transform.GetWorldScale().x,
					pc->GetHeight() * p_actor.transform.GetWorldScale().y
				),
				pc->GetRadius() * p_actor.transform.GetWorldScale().z
			) * 1.5f);
		}

		if (auto modelRenderer = p_actor.GetComponent<Core::ECS::Components::CModelRenderer>())
		{
			const bool hasCustomBoundingSphere = modelRenderer->GetFrustumBehaviour() == Core::ECS::Components::CModelRenderer::EFrustumBehaviour::CULL_CUSTOM;
			const bool hasModel = modelRenderer->GetModel();
			const auto boundingSphere = hasCustomBoundingSphere ? &modelRenderer->GetCustomBoundingSphere() : hasModel ? &modelRenderer->GetModel()->GetBoundingSphere() : nullptr;
			const auto& actorPosition = p_actor.transform.GetWorldPosition();
			const auto& actorScale = p_actor.transform.GetWorldScale();
			const auto scaleFactor = std::max(std::max(actorScale.x, actorScale.y), actorScale.z);

			distance = std::max(distance, boundingSphere ? (boundingSphere->radius + Maths::FVector3::Length(boundingSphere->position)) * scaleFactor * 2.0f : 10.0f);
		}

		for (auto child : p_actor.GetChildren())
			distance = std::max(distance, GetActorFocusDist(*child));
	}

	return distance;
}

void Editor::Core::CameraController::HandleInputs(float p_deltaTime)
{

}

void Editor::Core::CameraController::MoveToTarget(::Core::ECS::Actor& p_target)
{
	m_cameraDestinations.push({
		p_target.transform.GetWorldPosition() -
		m_camera.GetRotation() *
		Maths::FVector3::Forward *
		GetActorFocusDist(p_target),
		m_camera.GetRotation()
		});
}

void Editor::Core::CameraController::SetSpeed(float p_speed)
{
	m_cameraMoveSpeed = p_speed;
}

float Editor::Core::CameraController::GetSpeed() const
{
	return m_cameraMoveSpeed;
}

void Editor::Core::CameraController::SetPosition(const Maths::FVector3& p_position)
{
	m_camera.SetPosition(p_position);
}

void Editor::Core::CameraController::SetRotation(const Maths::FQuaternion& p_rotation)
{
	m_camera.SetRotation(p_rotation);
}

const Maths::FVector3& Editor::Core::CameraController::GetPosition() const
{
	return m_camera.GetPosition();
}

const Maths::FQuaternion& Editor::Core::CameraController::GetRotation() const
{
	return m_camera.GetRotation();
}

bool Editor::Core::CameraController::IsRightMousePressed() const
{
	return m_rightMousePressed;
}

void Editor::Core::CameraController::LockTargetActor(::Core::ECS::Actor& p_actor)
{
	m_lockedActor = p_actor;
}

void Editor::Core::CameraController::UnlockTargetActor()
{
	m_lockedActor = std::nullopt;
}

void Editor::Core::CameraController::ReceiveEvent(QEvent* e)
{
	if (e == nullptr)
		return;

	const QEvent::Type t = e->type();
	if (t == QEvent::KeyPress) {
		QKeyEvent* e2 = static_cast<QKeyEvent*>(e);
		Qt::Key key = static_cast<Qt::Key>(e2->key());
		if (key == Qt::Key_W) {
			mKeyState[KEYW] = Down;
		}
		else if (key == Qt::Key_A) {
			mKeyState[KEYA] = Down;
		}
		else if (key == Qt::Key_S) {
			mKeyState[KEYS] = Down;
		}
		else if (key == Qt::Key_D) {
			mKeyState[KEYD] = Down;
		}
		else if (key == Qt::Key_Alt) {
			mKeyState[ALTA] = Down;
		}
		else if (key == Qt::Key_Q) {
			mKeyState[KEYQ] = Down;
		}
		else if (key == Qt::Key_E) {
			mKeyState[KEYE] = Down;
		}
	}
	else if (t == QEvent::KeyRelease) {
		QKeyEvent* e2 = static_cast<QKeyEvent*>(e);
		Qt::Key key = static_cast<Qt::Key>(e2->key());
		if (key == Qt::Key_W) {
			mKeyState[KEYW] = Up;
		}
		else if (key == Qt::Key_A) {
			mKeyState[KEYA] = Up;
		}
		else if (key == Qt::Key_S) {
			mKeyState[KEYS] = Up;
		}
		else if (key == Qt::Key_D) {
			mKeyState[KEYD] = Up;
		}
		else if (key == Qt::Key_Alt) {
			mKeyState[ALTA] = Up;
		}
		else if (key == Qt::Key_Q) {
			mKeyState[KEYQ] = Up;
		}
		else if (key == Qt::Key_E) {
			mKeyState[KEYE] = Up;
		}
	}


	if (t == QEvent::MouseButtonRelease) {
		QMouseEvent* e2 = static_cast<QMouseEvent*>(e);
		switch (e2->button())
		{
		case Qt::LeftButton:
			m_leftMousePressed = false;
			m_firstMouse = true;
			break;

		case Qt::MiddleButton:
			m_middleMousePressed = false;
			m_firstMouse = true;
			break;

		case Qt::RightButton:
			m_rightMousePressed = false;
			m_firstMouse = true;
			break;

		default:
			break;
		}
	}
	else if (t == QEvent::MouseButtonPress)
	{
		QMouseEvent* e2 = static_cast<QMouseEvent*>(e);
		switch (e2->button())
		{
		case Qt::LeftButton:
			m_leftMousePressed = true;

			break;

		case Qt::MiddleButton:
			m_middleMousePressed = true;
			break;
		case Qt::RightButton:
			m_rightMousePressed = true;
			break;

		default:
			break;
		}
	}
	else if (t == QEvent::MouseMove) {
		QMouseEvent* e2 = static_cast<QMouseEvent*>(e);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		auto xPos = e2->x();
		auto yPos = e2->y();
#else
		auto x = e2->position().x();
		auto y = e2->position().y();
#endif

		if (m_rightMousePressed || m_middleMousePressed || m_leftMousePressed) {
			bool wasFirstMouse = m_firstMouse;

			if (m_firstMouse)
			{
				m_lastMousePosX = xPos;
				m_lastMousePosY = yPos;
				m_firstMouse = false;
			}

			Maths::FVector2 mouseOffset
			{
				static_cast<float>(xPos - m_lastMousePosX),
				static_cast<float>(m_lastMousePosY - yPos)
			};

			m_lastMousePosX = xPos;
			m_lastMousePosY = yPos;
			if (m_rightMousePressed)
			{
				HandleCameraFPSMouse(mouseOffset, wasFirstMouse);
			}
			else
			{
				if (m_middleMousePressed)
				{
					if (mKeyState[ALTA] == Down && m_view.IsSelectActor()) {
						auto& target = m_view.GetSelectedActor();
						HandleCameraOrbit(target, mouseOffset, wasFirstMouse);
					}
					else {
						HandleCameraPanning(mouseOffset, wasFirstMouse);
					}

				}

			}
		}

	}
	else if (t == QEvent::Wheel) {
		constexpr float kUnitsPerScroll = 1.0f;
		QWheelEvent* e2 = static_cast<QWheelEvent*>(e);
		const auto verticalScroll = 0.002 * static_cast<float>(
			e2->angleDelta().y()
			);

		m_camera.SetPosition(
			m_camera.GetPosition() +
			m_camera.transform->GetWorldForward() * kUnitsPerScroll * verticalScroll
		);
	}
	HandleCameraFPSKeyboard(0.016, e);
}

std::optional<std::reference_wrapper<Core::ECS::Actor>> Editor::Core::CameraController::GetTargetActor() const
{
	if (m_lockedActor.has_value())
	{
		return m_lockedActor;
	}


	return std::nullopt;
}

void Editor::Core::CameraController::HandleCameraPanning(const Maths::FVector2& p_mouseOffset, bool p_firstMouset)
{


	auto mouseOffset = p_mouseOffset * m_cameraDragSpeed;

	m_camera.SetPosition(m_camera.GetPosition() + m_camera.transform->GetWorldRight() * mouseOffset.x);
	m_camera.SetPosition(m_camera.GetPosition() - m_camera.transform->GetWorldUp() * mouseOffset.y);
}

Maths::FVector3 RemoveRoll(const Maths::FVector3& p_ypr)
{
	Maths::FVector3 result = p_ypr;

	if (result.z >= 179.0f || result.z <= -179.0f)
	{
		result.x += result.z;
		result.y = 180.0f - result.y;
		result.z = 0.0f;
	}

	if (result.x > 180.0f) result.x -= 360.0f;
	if (result.x < -180.0f) result.x += 360.0f;

	return result;
}

void Editor::Core::CameraController::HandleCameraOrbit(
	::Core::ECS::Actor& p_target,
	const Maths::FVector2& p_mouseOffset,
	bool p_firstMouse
)
{
	auto mouseOffset = p_mouseOffset * m_cameraOrbitSpeed;

	if (p_firstMouse)
	{
		m_ypr = Maths::FQuaternion::EulerAngles(m_camera.GetRotation());
		m_ypr = RemoveRoll(m_ypr);
		m_orbitTarget = &p_target.transform.GetFTransform();
		m_orbitStartOffset = -Maths::FVector3::Forward * Maths::FVector3::Distance(m_orbitTarget->GetWorldPosition(), m_camera.GetPosition());
	}

	m_ypr.y += -mouseOffset.x;
	m_ypr.x += -mouseOffset.y;
	m_ypr.x = std::max(std::min(m_ypr.x, 90.0f), -90.0f);

	auto& target = p_target.transform.GetFTransform();
	Maths::FTransform pivotTransform(target.GetWorldPosition());
	Maths::FTransform cameraTransform(m_orbitStartOffset);
	cameraTransform.SetParent(pivotTransform);
	pivotTransform.RotateLocal(Maths::FQuaternion(m_ypr));
	m_camera.SetPosition(cameraTransform.GetWorldPosition());
	m_camera.SetRotation(cameraTransform.GetWorldRotation());
}

void Editor::Core::CameraController::HandleCameraZoom()
{
	constexpr float kUnitsPerScroll = 1.0f;

	const auto verticalScroll = 1.0f;
	m_camera.SetPosition(
		m_camera.GetPosition() +
		m_camera.transform->GetWorldForward() * kUnitsPerScroll * verticalScroll
	);
}

void Editor::Core::CameraController::HandleCameraFPSMouse(const Maths::FVector2& p_mouseOffset, bool p_firstMouse)
{
	auto mouseOffset = p_mouseOffset * m_mouseSensitivity;

	if (p_firstMouse)
	{
		m_ypr = Maths::FQuaternion::EulerAngles(m_camera.GetRotation());
		m_ypr = RemoveRoll(m_ypr);
	}

	m_ypr.y -= mouseOffset.x;
	m_ypr.x += -mouseOffset.y;
	m_ypr.x = std::max(std::min(m_ypr.x, 90.0f), -90.0f);

	m_camera.SetRotation(Maths::FQuaternion(m_ypr));
}

void Editor::Core::CameraController::HandleCameraFPSKeyboard(float p_deltaTime, QEvent* e)
{


	if (m_rightMousePressed)
	{
		m_targetSpeed = Maths::FVector3(0.f, 0.f, 0.f);

		float velocity = m_cameraMoveSpeed * p_deltaTime * 0.1f;

		if (mKeyState[KEYW] == Down)
			m_targetSpeed += m_camera.transform->GetWorldForward() * velocity;
		if (mKeyState[KEYS] == Down)
			m_targetSpeed += m_camera.transform->GetWorldForward() * -velocity;
		if (mKeyState[KEYA] == Down)
			m_targetSpeed += m_camera.transform->GetWorldRight() * velocity;
		if (mKeyState[KEYD] == Down)
			m_targetSpeed += m_camera.transform->GetWorldRight() * -velocity;
		if (mKeyState[KEYE] == Down)
			m_targetSpeed += {0.0f, velocity, 0.0f};
		if (mKeyState[KEYQ] == Down)
			m_targetSpeed += {0.0f, -velocity, 0.0f};

		m_currentMovementSpeed = Maths::FVector3::Lerp(m_currentMovementSpeed, m_targetSpeed, 10.0f * p_deltaTime);
		m_camera.SetPosition(m_camera.GetPosition() + m_currentMovementSpeed);

	}

}

void Editor::Core::CameraController::HandleMousePressed()
{

}

void Editor::Core::CameraController::HandleMouseReleased()
{

}
