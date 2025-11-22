#include <Core/ECS/Components/CAmbientBoxLight.h>
#include <Core/ECS/Components/CAmbientSphereLight.h>
#include "CameraController.h"
#include <iostream>


Editor::Core::CameraController::CameraController(
	Editor::Panels::AView& p_view,
	Rendering::Entities::Camera& p_camera
) :
	m_view(p_view),
	m_camera(p_camera)
{
	m_camera.SetFov(60.0f);

}

float GetActorFocusDist(Core::ECS::Actor& p_actor)
{
	float distance = 4.0f;

	if (p_actor.IsActive())
	{
		if (auto modelRenderer = p_actor.GetComponent<Core::ECS::Components::CModelRenderer>())
		{
			const bool hasCustomBoundingSphere = modelRenderer->GetFrustumBehaviour() == Core::ECS::Components::CModelRenderer::EFrustumBehaviour::CUSTOM_BOUNDS;
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
	HandleMouseReleased();
	HandleMousePressed();
	auto& input = m_view.getInutState();
	if (m_view.IsSelectActor()) {
		auto& target = m_view.GetSelectedActor();
		auto targetPos = target.transform.GetWorldPosition();
		float dist = GetActorFocusDist(target);

		if (input.IsKeyPressed(Editor::Panels::KEYF))
		{
			MoveToTarget(target);
		}

		auto focusObjectFromAngle = [this, &targetPos, &dist](const Maths::FVector3& offset)
			{
				auto camPos = targetPos + offset * dist;
				auto direction = Maths::FVector3::Normalize(targetPos - camPos);
				m_camera.SetRotation(Maths::FQuaternion::LookAt(direction, abs(direction.y) == 1.0f ? Maths::FVector3::Right : Maths::FVector3::Up));
				m_cameraDestinations.push({ camPos, m_camera.GetRotation() });
			};

		if (input.IsKeyPressed(Editor::Panels::UP))		focusObjectFromAngle(Maths::FVector3::Up);
		if (input.IsKeyPressed(Editor::Panels::DOWN))		focusObjectFromAngle(-Maths::FVector3::Up);
		if (input.IsKeyPressed(Editor::Panels::RIGHT))	focusObjectFromAngle(Maths::FVector3::Right);
		if (input.IsKeyPressed(Editor::Panels::LEFT))		focusObjectFromAngle(-Maths::FVector3::Right);
		if (input.IsKeyPressed(Editor::Panels::PageUp))	focusObjectFromAngle(Maths::FVector3::Forward);
		if (input.IsKeyPressed(Editor::Panels::PageDown))	focusObjectFromAngle(-Maths::FVector3::Forward);
	}

	if (!m_cameraDestinations.empty())
	{
		m_currentMovementSpeed = 0.0f;

		while (m_cameraDestinations.size() != 1)
			m_cameraDestinations.pop();

		auto& [destPos, destRotation] = m_cameraDestinations.front();

		float t = m_focusLerpCoefficient * p_deltaTime;

		if (Maths::FVector3::Distance(m_camera.GetPosition(), destPos) <= 0.03f)
		{
			m_camera.SetPosition(destPos);
			m_camera.SetRotation(destRotation);
			m_cameraDestinations.pop();
		}
		else
		{
			m_camera.SetPosition(Maths::FVector3::Lerp(m_camera.GetPosition(), destPos, t));
			m_camera.SetRotation(Maths::FQuaternion::Lerp(m_camera.GetRotation(), destRotation, t));
		}
	}
	else
	{   
		auto [xPos, yPos] = input.GetMousePosition();
		if (m_rightMousePressed || m_middleMousePressed || m_leftMousePressed)
		{

			bool wasFirstMouse = m_firstMouse;
			if (m_firstMouse)
			{
				m_lastMousePosX = xPos;
				m_lastMousePosY = yPos;
				
				HandleFirstMouse();
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
				HandleCameraOrbit(m_view.GetRoaterCenter(), mouseOffset, wasFirstMouse);
			}
			else if (m_middleMousePressed)
			{
                HandleCameraPanning(mouseOffset, wasFirstMouse);
			}
			else
			{
				//HandleCameraFPSMouse(mouseOffset, wasFirstMouse);
			}
		}
		HandleCameraZoom();
		HandleCameraFPSKeyboard(p_deltaTime);
	}
}
Maths::FVector3 RemoveRoll(const Maths::FVector3& p_ypr);
void Editor::Core::CameraController::HandleFirstMouse()
{
	m_ypr = Maths::FQuaternion::EulerAngles(m_camera.GetRotation());
	m_ypr = RemoveRoll(m_ypr);
	m_orbitStartOffset = -Maths::FVector3::Forward * Maths::FVector3::Distance(m_view.GetRoaterCenter(), m_camera.GetPosition());
	
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

void Editor::Core::CameraController::MoveToPose(const Maths::FVector3& pos, const Maths::FQuaternion& quat)
{
	m_cameraDestinations.push({pos,quat});
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

bool Editor::Core::CameraController::IsOperating() const
{
	return m_rightMousePressed || m_middleMousePressed;
}

void Editor::Core::CameraController::LockTargetActor(::Core::ECS::Actor& p_actor)
{
	m_lockedActor = p_actor;
}

void Editor::Core::CameraController::UnlockTargetActor()
{
	m_lockedActor = std::nullopt;
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

	m_camera.HandleCameraPanning(p_mouseOffset, m_cameraDragSpeed);
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
	const Maths::FVector3& center,
	const Maths::FVector2& p_mouseOffset,
	bool p_firstMouse
)
{
	auto mouseOffset = p_mouseOffset * m_cameraOrbitSpeed;
	float oldpry = m_ypr.y;
	float oldprx = m_ypr.x;
	m_ypr.y +=-mouseOffset.x;
	m_ypr.x +=-mouseOffset.y;
	//m_ypr.x = std::max(std::min(m_ypr.x, 90.0f), -90.0f);
	float offsetY =  m_ypr.x - oldprx;
	float offsetX =  m_ypr.y - oldpry;
	//Maths::FTransform pivotTransform(center);
	//Maths::FTransform cameraTransform(m_orbitStartOffset);
	//cameraTransform.SetParent(pivotTransform);
	//pivotTransform.RotateLocal(Maths::FQuaternion(m_ypr));
	//m_camera.SetPosition(cameraTransform.GetWorldPosition());
	//m_camera.SetRotation(cameraTransform.GetWorldRotation());
	Maths::FTransform pivotTransform(center);
	Maths::FTransform cameraTransform=m_camera.GetTransform();
	cameraTransform.SetParent(pivotTransform);
	cameraTransform.SetLocalPosition(m_camera.GetPosition()-center);
	auto yAxis =  m_camera.GetTransform().GetWorldUp();
	auto xAxis = m_camera.GetTransform().GetWorldRight();
	float ratio = m_camera.GetRatio();
	//There's no need to correct the aspect ratio.
	auto rAXis = yAxis * offsetX + xAxis*offsetY ;
	float angleRad = rAXis.Length() / 180.0*3.14157;// -mouseOffset.x / 180.0f * 3.14157;// rAXis.Length() / 100.0;
	rAXis = rAXis.Normalize();
	auto quat = Maths::FQuaternion(rAXis, angleRad);
	pivotTransform.RotateWorld(quat);
	m_camera.SetPosition(cameraTransform.GetWorldPosition());
	m_camera.SetRotation(cameraTransform.GetWorldRotation());
}

void Editor::Core::CameraController::HandleCameraZoom()
{
	const auto verticalScroll = m_view.getInutState().GetMouseScroll().second;
	auto& input = m_view.getInutState();
	auto [x, y] = input.GetMousePosition();

	if (m_camera.GetProjectionMode() == Rendering::Settings::EProjectionMode::PERSPECTIVE)
	{
		
		m_camera.PersertiveZoom(verticalScroll);
	}
	else
	{
		m_camera.OrthZoom(verticalScroll,x,y);
	}

}

void Editor::Core::CameraController::HandleCameraFPSMouse(const Maths::FVector2& p_mouseOffset, bool p_firstMouse)
{
	//auto mouseOffset = p_mouseOffset * m_mouseSensitivity;


	//m_ypr.y -= mouseOffset.x;
	//m_ypr.x += -mouseOffset.y;
	//m_ypr.x = std::max(std::min(m_ypr.x, 90.0f), -90.0f);

	//m_camera.SetRotation(Maths::FQuaternion(m_ypr));
}

void Editor::Core::CameraController::HandleCameraFPSKeyboard(float p_deltaTime)
{
	m_targetSpeed = Maths::FVector3(0.f, 0.f, 0.f);
	auto& input = m_view.getInutState();
	if (m_rightMousePressed)
	{
		float velocity = m_cameraMoveSpeed * p_deltaTime * 2.0f;

		if (input.IsKeyPressed(Editor::Panels::KEYW))
			m_targetSpeed += m_camera.transform->GetWorldForward() * velocity;
		if (input.IsKeyPressed(Editor::Panels::KEYS))
			m_targetSpeed += m_camera.transform->GetWorldForward() * -velocity;
		if (input.IsKeyPressed(Editor::Panels::KEYA))
			m_targetSpeed += m_camera.transform->GetWorldRight() * velocity;
		if (input.IsKeyPressed(Editor::Panels::KEYD))
			m_targetSpeed += m_camera.transform->GetWorldRight() * -velocity;
		if (input.IsKeyPressed(Editor::Panels::KEYE))
			m_targetSpeed += {0.0f, velocity, 0.0f};
		if (input.IsKeyPressed(Editor::Panels::KEYQ))
			m_targetSpeed += {0.0f, -velocity, 0.0f};

	}

	m_currentMovementSpeed = Maths::FVector3::Lerp(m_currentMovementSpeed, m_targetSpeed, 10.0f * p_deltaTime);
	m_camera.SetPosition(m_camera.GetPosition() + m_currentMovementSpeed);
}

void Editor::Core::CameraController::HandleMousePressed()
{
	auto& input = m_view.getInutState();
	if (input.IsMouseButtonPressed(Editor::Panels::MouseButton::MOUSE_BUTTON_LEFT))
	{
		m_leftMousePressed = true;
	}

	if (input.IsMouseButtonPressed(Editor::Panels::MouseButton::MOUSE_BUTTON_MIDDLE))
	{
		m_middleMousePressed = true;
	}

	if (input.IsMouseButtonPressed(Editor::Panels::MouseButton::MOUSE_BUTTON_RIGHT))
	{
		m_rightMousePressed = true;
	}
}

void Editor::Core::CameraController::HandleMouseReleased()
{
	auto& input = m_view.getInutState();
	if (m_leftMousePressed && input.IsMouseButtonReleased(Editor::Panels::MouseButton::MOUSE_BUTTON_LEFT))
	{
		m_leftMousePressed = false;
		m_firstMouse = true;
	}

	if (m_middleMousePressed && input.IsMouseButtonReleased(Editor::Panels::MouseButton::MOUSE_BUTTON_MIDDLE))
	{
		m_middleMousePressed = false;
		m_firstMouse = true;
	}

	if (m_rightMousePressed && input.IsMouseButtonReleased(Editor::Panels::MouseButton::MOUSE_BUTTON_RIGHT))
	{
		m_rightMousePressed = false;
		m_firstMouse = true;

	}
}
