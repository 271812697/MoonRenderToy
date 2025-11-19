

#include "GizmoBehaviour.h"
//#include "Editor/Core/EditorActions.h"
//#include "Editor/Settings/EditorSettings.h"

float SnapValue(float p_value, float p_step)
{
	return p_value - std::fmod(p_value, p_step);
}

Maths::FVector3 SnapValue(Maths::FVector3 p_value, float p_step)
{
	Maths::FVector3 result;
	result.x = std::round(p_value.x / p_step) * p_step;
	result.y = std::round(p_value.y / p_step) * p_step;
	result.z = std::round(p_value.z / p_step) * p_step;
	return result;
}

bool Editor::Core::GizmoBehaviour::IsSnappedBehaviourEnabled() const
{

	return false;
}

void Editor::Core::GizmoBehaviour::StartPicking(::Core::ECS::Actor& p_target, const Maths::FVector3& p_cameraPosition, EGizmoOperation p_operation, EDirection p_direction)
{
	m_target = &p_target;
	m_firstMouse = true;
	m_firstPick = true;
	m_originalTransform = p_target.transform.GetFTransform();
	m_distanceToActor = Maths::FVector3::Distance(p_cameraPosition, m_target->transform.GetWorldPosition());
	m_currentOperation = p_operation;
	m_direction = p_direction;
}

void Editor::Core::GizmoBehaviour::StopPicking()
{
	m_target = nullptr;
}

Maths::FVector3 Editor::Core::GizmoBehaviour::GetFakeDirection() const
{
	auto result = Maths::FVector3();

	switch (m_direction)
	{
	case Editor::Core::GizmoBehaviour::EDirection::X:
		result = Maths::FVector3::Right;
		break;
	case Editor::Core::GizmoBehaviour::EDirection::Y:
		result = Maths::FVector3::Up;
		break;
	case Editor::Core::GizmoBehaviour::EDirection::Z:
		result = Maths::FVector3::Forward;
		break;
	}

	return result;
}

Maths::FVector3 Editor::Core::GizmoBehaviour::GetRealDirection(bool p_relative) const
{
	auto result = Maths::FVector3();

	switch (m_direction)
	{
	case Editor::Core::GizmoBehaviour::EDirection::X:
		result = p_relative ? m_originalTransform.GetWorldRight() : m_originalTransform.GetLocalRight();
		break;
	case Editor::Core::GizmoBehaviour::EDirection::Y:
		result = p_relative ? m_originalTransform.GetWorldUp() : m_originalTransform.GetLocalUp();
		break;
	case Editor::Core::GizmoBehaviour::EDirection::Z:
		result = p_relative ? m_originalTransform.GetWorldForward() : m_originalTransform.GetLocalForward();
		break;
	}

	return result;
}

Maths::FVector2 Editor::Core::GizmoBehaviour::GetScreenDirection(const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector2& p_viewSize) const
{
	auto start = m_originalTransform.GetWorldPosition();
	auto end = m_originalTransform.GetWorldPosition() + GetRealDirection(true) * 0.01f;

	auto start2D = Maths::FVector2();
	{
		auto clipSpacePos = p_projectionMatrix * (p_viewMatrix * Maths::FVector4{ start.x, start.y, start.z, 1.0f });
		auto ndcSpacePos = Maths::FVector3{ clipSpacePos.x, clipSpacePos.y, clipSpacePos.z } / clipSpacePos.w;
		auto windowSpacePos = ((Maths::FVector2{ ndcSpacePos.x, ndcSpacePos.y } + 1.0f) / 2.0f);
		windowSpacePos.x *= p_viewSize.x;
		windowSpacePos.y *= p_viewSize.y;
		start2D = windowSpacePos;
	}

	auto end2D = Maths::FVector2();
	{
		auto clipSpacePos = p_projectionMatrix * (p_viewMatrix * Maths::FVector4{ end.x, end.y, end.z, 1.0f });
		auto ndcSpacePos = Maths::FVector3{ clipSpacePos.x, clipSpacePos.y, clipSpacePos.z } / clipSpacePos.w;
		auto windowSpacePos = ((Maths::FVector2{ ndcSpacePos.x, ndcSpacePos.y } + 1.0f) / 2.0f);
		windowSpacePos.x *= p_viewSize.x;
		windowSpacePos.y *= p_viewSize.y;
		end2D = windowSpacePos;
	}

	auto result = end2D - start2D;

	result.y *= -1; // Screen coordinates are reversed, so we inverse the Y

	return Maths::FVector2::Normalize(result);
}

void Editor::Core::GizmoBehaviour::ApplyTranslation(const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector3& p_cameraPosition, const Maths::FVector2& p_viewSize)
{
	auto ray = GetMouseRay(m_currentMouse, p_viewMatrix, p_projectionMatrix, p_viewSize);

	const Maths::FVector3 planeTangent = Maths::FVector3::Cross(GetRealDirection(true), m_target->transform.GetWorldPosition() - p_cameraPosition);
	const Maths::FVector3 planeNormal = Maths::FVector3::Cross(GetRealDirection(true), planeTangent);

	Maths::FVector3 direction = GetRealDirection(true);

	Maths::FVector3 planePoint = m_originalTransform.GetWorldPosition();

	const float denom = Maths::FVector3::Dot(ray, planeNormal);

	if (std::abs(denom) <= 0.001f)
		return;

	const float t = Maths::FVector3::Dot(planePoint - p_cameraPosition, planeNormal) / denom;

	if (t <= 0.001f)
		return;

	Maths::FVector3 point = p_cameraPosition + ray * t;

	if (m_firstPick)
	{
		m_initialOffset = m_originalTransform.GetWorldPosition() - point;
		m_firstPick = false;
	}

	auto translationVector = point - planePoint + m_initialOffset;

	if (IsSnappedBehaviourEnabled())
	{
		translationVector = { 3,3,3 };
	}

	Maths::FVector3 projectedPoint = planePoint + direction * Maths::FVector3::Dot(translationVector, direction);

	m_target->transform.SetWorldPosition(projectedPoint);
}

void Editor::Core::GizmoBehaviour::ApplyRotation(const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector2& p_viewSize) const
{
	auto unitsPerPixel = 0.2f;
	auto originRotation = m_originalTransform.GetWorldRotation();

	auto screenDirection = GetScreenDirection(p_viewMatrix, p_projectionMatrix, p_viewSize);
	screenDirection = Maths::FVector2(-screenDirection.y, screenDirection.x);

	auto totalDisplacement = m_currentMouse - m_originMouse;
	auto rotationCoefficient = Maths::FVector2::Dot(totalDisplacement, screenDirection) * unitsPerPixel;

	if (IsSnappedBehaviourEnabled())
	{
		rotationCoefficient = 3.0f;
	}

	auto rotationToApply = Maths::FQuaternion(Maths::FVector3(GetFakeDirection() * rotationCoefficient));
	m_target->transform.SetWorldRotation(originRotation * rotationToApply);
}

void Editor::Core::GizmoBehaviour::ApplyScale(const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector3& p_cameraPosition, const Maths::FVector2& p_viewSize)
{
	if (!m_target)
		return;

	const auto ray = GetMouseRay(m_currentMouse, p_viewMatrix, p_projectionMatrix, p_viewSize);

	const Maths::FVector3 gizmoAxisDirection = GetRealDirection(true);

	Maths::FVector3 vectorToCamera = m_target->transform.GetWorldPosition() - p_cameraPosition;
	if (Maths::FVector3::Dot(vectorToCamera, vectorToCamera) < 0.0001f)
	{
		vectorToCamera = m_originalTransform.GetWorldUp();
		if (std::abs(Maths::FVector3::Dot(gizmoAxisDirection, vectorToCamera)) > 0.99f)
		{
			vectorToCamera = m_originalTransform.GetWorldRight();
		}
	}

	const Maths::FVector3 planeTangent = Maths::FVector3::Cross(gizmoAxisDirection, vectorToCamera);

	Maths::FVector3 tempPlaneNormal = Maths::FVector3::Cross(gizmoAxisDirection, planeTangent);
	if (Maths::FVector3::Dot(tempPlaneNormal, tempPlaneNormal) < 0.0001f)
	{
		Maths::FVector3 nonCollinearVector = Maths::FVector3::Up;
		if (std::abs(Maths::FVector3::Dot(gizmoAxisDirection, nonCollinearVector)) > 0.99f)
		{
			nonCollinearVector = Maths::FVector3::Right;
		}
		tempPlaneNormal = Maths::FVector3::Cross(gizmoAxisDirection, nonCollinearVector);
	}
	const Maths::FVector3 planeNormal = Maths::FVector3::Normalize(tempPlaneNormal);


	const Maths::FVector3 planePoint = m_originalTransform.GetWorldPosition();

	const float denom = Maths::FVector3::Dot(ray, planeNormal);

	if (std::abs(denom) <= 0.001f)
		return;

	const float t = Maths::FVector3::Dot(planePoint - p_cameraPosition, planeNormal) / denom;

	if (t <= 0.001f)
		return;

	const Maths::FVector3 pointOnPlane = p_cameraPosition + ray * t * 2.0f;

	Maths::FVector3 displacementOnPlane;
	if (m_firstPick)
	{
		m_initialOffset = m_originalTransform.GetWorldPosition() - pointOnPlane;
		m_firstPick = false;
		displacementOnPlane = Maths::FVector3::Zero;
	}
	else
	{
		displacementOnPlane = pointOnPlane - m_originalTransform.GetWorldPosition() + m_initialOffset;
	}

	float scaleDeltaCoefficient = Maths::FVector3::Dot(displacementOnPlane, gizmoAxisDirection);

	if (IsSnappedBehaviourEnabled())
	{
		scaleDeltaCoefficient = 3.0f;
	}

	const auto originScale = m_originalTransform.GetWorldScale();

	auto newScale = originScale + GetFakeDirection() * scaleDeltaCoefficient;

	/* Prevent scale from being negative*/
	newScale.x = std::max(newScale.x, 0.0001f);
	newScale.y = std::max(newScale.y, 0.0001f);
	newScale.z = std::max(newScale.z, 0.0001f);

	m_target->transform.SetWorldScale(newScale);
}

void Editor::Core::GizmoBehaviour::ApplyOperation(const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector3& p_cameraPosition, const Maths::FVector2& p_viewSize)
{
	switch (m_currentOperation)
	{
	case EGizmoOperation::TRANSLATE:
		ApplyTranslation(p_viewMatrix, p_projectionMatrix, p_cameraPosition, p_viewSize);
		break;

	case EGizmoOperation::ROTATE:
		ApplyRotation(p_viewMatrix, p_projectionMatrix, p_viewSize);
		break;

	case EGizmoOperation::SCALE:
		ApplyScale(p_viewMatrix, p_projectionMatrix, p_cameraPosition, p_viewSize);
		break;
	}
}

void Editor::Core::GizmoBehaviour::SetCurrentMouse(const Maths::FVector2& p_mousePosition)
{
	if (m_firstMouse)
	{
		m_currentMouse = m_originMouse = p_mousePosition;
		m_firstMouse = false;
	}
	else
	{
		m_currentMouse = p_mousePosition;
	}
}

bool Editor::Core::GizmoBehaviour::IsPicking() const
{
	return m_target;
}

Editor::Core::GizmoBehaviour::EDirection Editor::Core::GizmoBehaviour::GetDirection() const
{
	return m_direction;
}

Maths::FVector3 Editor::Core::GizmoBehaviour::GetMouseRay(const Maths::FVector2& p_mousePos, const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector2& p_viewSize)
{
	float x = 2.0f * (p_mousePos.x / p_viewSize.x) - 1.0f;
	float y = 1.0f - 2.0f * (p_mousePos.y / p_viewSize.y);

	Maths::FMatrix4 inverseView = Maths::FMatrix4::Inverse(p_viewMatrix);
	Maths::FMatrix4 inverseProjection = Maths::FMatrix4::Inverse(p_projectionMatrix);

	Maths::FMatrix4 inverseViewProjection = inverseView * inverseProjection;

	Maths::FVector4 nearestPoint = inverseViewProjection * Maths::FVector4(x, y, -1.0f, 1.0f);
	Maths::FVector4 farthestPoint = inverseViewProjection * Maths::FVector4(x, y, 1.0f, 1.0f);

	return Maths::FVector3(farthestPoint.x, farthestPoint.y, farthestPoint.z) * nearestPoint.w - Maths::FVector3(nearestPoint.x, nearestPoint.y, nearestPoint.z) * farthestPoint.w; ;
}
