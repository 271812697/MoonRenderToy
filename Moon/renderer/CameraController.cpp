/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/


#include <OvCore/ECS/Components/CAmbientBoxLight.h>
#include <OvCore/ECS/Components/CAmbientSphereLight.h>
#include <OvCore/ECS/Components/CPhysicalBox.h>
#include <OvCore/ECS/Components/CPhysicalCapsule.h>
#include <OvCore/ECS/Components/CPhysicalSphere.h>

#include "CameraController.h"
#include <QMouseEvent>

OvEditor::Core::CameraController::CameraController(
	OvEditor::Panels::AView& p_view,
	OvRendering::Entities::Camera& p_camera
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

float GetActorFocusDist(OvCore::ECS::Actor& p_actor)
{
	float distance = 4.0f;

	if (p_actor.IsActive())
	{
		if (auto pb = p_actor.GetComponent<OvCore::ECS::Components::CPhysicalBox>())
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

		if (auto ps = p_actor.GetComponent<OvCore::ECS::Components::CPhysicalSphere>())
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

		if (auto pc = p_actor.GetComponent<OvCore::ECS::Components::CPhysicalCapsule>())
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

		if (auto modelRenderer = p_actor.GetComponent<OvCore::ECS::Components::CModelRenderer>())
		{
			const bool hasCustomBoundingSphere = modelRenderer->GetFrustumBehaviour() == OvCore::ECS::Components::CModelRenderer::EFrustumBehaviour::CUSTOM_BOUNDS;
			const bool hasModel = modelRenderer->GetModel();
			const auto boundingSphere = hasCustomBoundingSphere ? &modelRenderer->GetCustomBoundingSphere() : hasModel ? &modelRenderer->GetModel()->GetBoundingSphere() : nullptr;
			const auto& actorPosition = p_actor.transform.GetWorldPosition();
			const auto& actorScale = p_actor.transform.GetWorldScale();
			const auto scaleFactor = std::max(std::max(actorScale.x, actorScale.y), actorScale.z);

			distance = std::max(distance, boundingSphere ? (boundingSphere->radius + OvMaths::FVector3::Length(boundingSphere->position)) * scaleFactor * 2.0f : 10.0f);
		}

		for (auto child : p_actor.GetChildren())
			distance = std::max(distance, GetActorFocusDist(*child));
	}

	return distance;
}

void OvEditor::Core::CameraController::HandleInputs(float p_deltaTime)
{
	if (!m_cameraDestinations.empty())
	{
		m_currentMovementSpeed = 0.0f;

		while (m_cameraDestinations.size() != 1)
			m_cameraDestinations.pop();

		auto& [destPos, destRotation] = m_cameraDestinations.front();

		float t = m_focusLerpCoefficient * p_deltaTime;

		if (OvMaths::FVector3::Distance(m_camera.GetPosition(), destPos) <= 0.03f)
		{
			m_camera.SetPosition(destPos);
			m_camera.SetRotation(destRotation);
			m_cameraDestinations.pop();
		}
		else
		{
			m_camera.SetPosition(OvMaths::FVector3::Lerp(m_camera.GetPosition(), destPos, t));
			m_camera.SetRotation(OvMaths::FQuaternion::Lerp(m_camera.GetRotation(), destRotation, t));
		}
	}

}

void OvEditor::Core::CameraController::MoveToTarget(OvCore::ECS::Actor& p_target)
{
	m_cameraDestinations.push({
		p_target.transform.GetWorldPosition() -
		m_camera.GetRotation() *
		OvMaths::FVector3::Forward *
		GetActorFocusDist(p_target),
		m_camera.GetRotation()
		});
}

void OvEditor::Core::CameraController::SetSpeed(float p_speed)
{
	m_cameraMoveSpeed = p_speed;
}

float OvEditor::Core::CameraController::GetSpeed() const
{
	return m_cameraMoveSpeed;
}

void OvEditor::Core::CameraController::SetPosition(const OvMaths::FVector3& p_position)
{
	m_camera.SetPosition(p_position);
}

void OvEditor::Core::CameraController::SetRotation(const OvMaths::FQuaternion& p_rotation)
{
	m_camera.SetRotation(p_rotation);
}

const OvMaths::FVector3& OvEditor::Core::CameraController::GetPosition() const
{
	return m_camera.GetPosition();
}

const OvMaths::FQuaternion& OvEditor::Core::CameraController::GetRotation() const
{
	return m_camera.GetRotation();
}

bool OvEditor::Core::CameraController::IsRightMousePressed() const
{
	return m_rightMousePressed;
}

bool OvEditor::Core::CameraController::IsOperating() const
{
	return m_rightMousePressed || m_middleMousePressed;
}

void OvEditor::Core::CameraController::LockTargetActor(OvCore::ECS::Actor& p_actor)
{
	m_lockedActor = p_actor;
}

void OvEditor::Core::CameraController::UnlockTargetActor()
{
	m_lockedActor = std::nullopt;
}
void OvEditor::Core::CameraController::ReceiveEvent(QEvent* e)
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
		else if (key == Qt::Key_Right || key == Qt::Key_Left || key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_PageUp || key == Qt::Key_PageDown) {
			if (m_view.IsSelectActor()) {
				auto& target = m_view.GetSelectedActor();
				auto targetPos = target.transform.GetWorldPosition();
				float dist = GetActorFocusDist(target);
				MoveToTarget(target);
				auto focusObjectFromAngle = [this, &targetPos, &dist](const OvMaths::FVector3& offset)
					{
						auto camPos = targetPos + offset * dist;
						auto direction = OvMaths::FVector3::Normalize(targetPos - camPos);
						m_camera.SetRotation(OvMaths::FQuaternion::LookAt(direction, abs(direction.y) == 1.0f ? OvMaths::FVector3::Right : OvMaths::FVector3::Up));
						m_cameraDestinations.push({ camPos, m_camera.GetRotation() });
					};
				if (key == Qt::Key_Up)			focusObjectFromAngle(OvMaths::FVector3::Up);
				if (key == Qt::Key_Down)		focusObjectFromAngle(-OvMaths::FVector3::Up);
				if (key == Qt::Key_Right)		focusObjectFromAngle(OvMaths::FVector3::Right);
				if (key == Qt::Key_Left)		focusObjectFromAngle(-OvMaths::FVector3::Right);
				if (key == Qt::Key_PageUp)	    focusObjectFromAngle(OvMaths::FVector3::Forward);
				if (key == Qt::Key_PageDown)	focusObjectFromAngle(-OvMaths::FVector3::Forward);

			}
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

			OvMaths::FVector2 mouseOffset
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

	HandleCameraFPSKeyboard(0.016);
}

std::optional<std::reference_wrapper<OvCore::ECS::Actor>> OvEditor::Core::CameraController::GetTargetActor() const
{
	if (m_lockedActor.has_value())
	{
		return m_lockedActor;
	}

	return std::nullopt;
}

void OvEditor::Core::CameraController::HandleCameraPanning(const OvMaths::FVector2& p_mouseOffset, bool p_firstMouset)
{

	auto mouseOffset = p_mouseOffset * m_cameraDragSpeed;

	m_camera.SetPosition(m_camera.GetPosition() + m_camera.transform->GetWorldRight() * mouseOffset.x);
	m_camera.SetPosition(m_camera.GetPosition() - m_camera.transform->GetWorldUp() * mouseOffset.y);
}

OvMaths::FVector3 RemoveRoll(const OvMaths::FVector3& p_ypr)
{
	OvMaths::FVector3 result = p_ypr;

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

void OvEditor::Core::CameraController::HandleCameraOrbit(
	OvCore::ECS::Actor& p_target,
	const OvMaths::FVector2& p_mouseOffset,
	bool p_firstMouse
)
{
	auto mouseOffset = p_mouseOffset * m_cameraOrbitSpeed;

	if (p_firstMouse)
	{
		m_ypr = OvMaths::FQuaternion::EulerAngles(m_camera.GetRotation());
		m_ypr = RemoveRoll(m_ypr);
		m_orbitTarget = &p_target.transform.GetFTransform();
		m_orbitStartOffset = -OvMaths::FVector3::Forward * OvMaths::FVector3::Distance(m_orbitTarget->GetWorldPosition(), m_camera.GetPosition());
	}

	m_ypr.y += -mouseOffset.x;
	m_ypr.x += -mouseOffset.y;
	m_ypr.x = std::max(std::min(m_ypr.x, 90.0f), -90.0f);

	auto& target = p_target.transform.GetFTransform();
	OvMaths::FTransform pivotTransform(target.GetWorldPosition());
	OvMaths::FTransform cameraTransform(m_orbitStartOffset);
	cameraTransform.SetParent(pivotTransform);
	pivotTransform.RotateLocal(OvMaths::FQuaternion(m_ypr));
	m_camera.SetPosition(cameraTransform.GetWorldPosition());
	m_camera.SetRotation(cameraTransform.GetWorldRotation());
}

void OvEditor::Core::CameraController::HandleCameraZoom()
{
	constexpr float kUnitsPerScroll = 1.0f;

	const auto verticalScroll = 1.0;

	m_camera.SetPosition(
		m_camera.GetPosition() +
		m_camera.transform->GetWorldForward() * kUnitsPerScroll * verticalScroll
	);
}

void OvEditor::Core::CameraController::HandleCameraFPSMouse(const OvMaths::FVector2& p_mouseOffset, bool p_firstMouse)
{
	auto mouseOffset = p_mouseOffset * m_mouseSensitivity;

	if (p_firstMouse)
	{
		m_ypr = OvMaths::FQuaternion::EulerAngles(m_camera.GetRotation());
		m_ypr = RemoveRoll(m_ypr);
	}

	m_ypr.y -= mouseOffset.x;
	m_ypr.x += -mouseOffset.y;
	m_ypr.x = std::max(std::min(m_ypr.x, 90.0f), -90.0f);

	m_camera.SetRotation(OvMaths::FQuaternion(m_ypr));
}

void OvEditor::Core::CameraController::HandleCameraFPSKeyboard(float p_deltaTime)
{
	m_targetSpeed = OvMaths::FVector3(0.f, 0.f, 0.f);

	if (m_rightMousePressed)
	{
		m_targetSpeed = OvMaths::FVector3(0.f, 0.f, 0.f);

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

		m_currentMovementSpeed = OvMaths::FVector3::Lerp(m_currentMovementSpeed, m_targetSpeed, 10.0f * p_deltaTime);
		m_camera.SetPosition(m_camera.GetPosition() + m_currentMovementSpeed);
	}

	m_currentMovementSpeed = OvMaths::FVector3::Lerp(m_currentMovementSpeed, m_targetSpeed, 10.0f * p_deltaTime);
	m_camera.SetPosition(m_camera.GetPosition() + m_currentMovementSpeed);
}

void OvEditor::Core::CameraController::HandleMousePressed()
{

}

void OvEditor::Core::CameraController::HandleMouseReleased()
{

}
