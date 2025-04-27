

#include <cmath>

#include "Rendering/LowRenderer/Camera.h"
#include "Maths/FMatrix4.h"

Rendering::LowRenderer::Camera::Camera() :
    m_projectionMode(Settings::EProjectionMode::PERSPECTIVE),
	m_fov(45.0f),
    m_size(5.0f),
	m_near(0.1f),
	m_far(100.f),
	m_clearColor(0.f, 0.f, 0.f),
	m_frustumGeometryCulling(false),
	m_frustumLightCulling(false)
{
}

void Rendering::LowRenderer::Camera::CacheMatrices(uint16_t p_windowWidth, uint16_t p_windowHeight, const Maths::FVector3& p_position, const Maths::FQuaternion& p_rotation)
{
	CacheProjectionMatrix(p_windowWidth, p_windowHeight);
	CacheViewMatrix(p_position, p_rotation);
	CacheFrustum(m_viewMatrix, m_projectionMatrix);
}

void Rendering::LowRenderer::Camera::CacheProjectionMatrix(uint16_t p_windowWidth, uint16_t p_windowHeight)
{
	m_projectionMatrix = CalculateProjectionMatrix(p_windowWidth, p_windowHeight);
}

void Rendering::LowRenderer::Camera::CacheViewMatrix(const Maths::FVector3& p_position, const Maths::FQuaternion& p_rotation)
{
	m_viewMatrix = CalculateViewMatrix(p_position, p_rotation);
}

void Rendering::LowRenderer::Camera::CacheFrustum(const Maths::FMatrix4& p_view, const Maths::FMatrix4& p_projection)
{
	m_frustum.CalculateFrustum(p_projection * p_view);
}

float Rendering::LowRenderer::Camera::GetFov() const
{
	return m_fov;
}

float Rendering::LowRenderer::Camera::GetSize() const
{
    return m_size;
}

float Rendering::LowRenderer::Camera::GetNear() const
{
	return m_near;
}

float Rendering::LowRenderer::Camera::GetFar() const
{
	return m_far;
}

const Maths::FVector3 & Rendering::LowRenderer::Camera::GetClearColor() const
{
	return m_clearColor;
}

const Maths::FMatrix4& Rendering::LowRenderer::Camera::GetProjectionMatrix() const
{
	return m_projectionMatrix;
}

const Maths::FMatrix4& Rendering::LowRenderer::Camera::GetViewMatrix() const
{
	return m_viewMatrix;
}

const Rendering::Data::Frustum& Rendering::LowRenderer::Camera::GetFrustum() const
{
	return m_frustum;
}

bool Rendering::LowRenderer::Camera::HasFrustumGeometryCulling() const
{
	return m_frustumGeometryCulling;
}

bool Rendering::LowRenderer::Camera::HasFrustumLightCulling() const
{
	return m_frustumLightCulling;
}

Rendering::Settings::EProjectionMode Rendering::LowRenderer::Camera::GetProjectionMode() const
{
    return m_projectionMode;
}

void Rendering::LowRenderer::Camera::SetFov(float p_value)
{
	m_fov = p_value;
}

void Rendering::LowRenderer::Camera::SetSize(float p_value)
{
    m_size = p_value;
}

void Rendering::LowRenderer::Camera::SetNear(float p_value)
{
	m_near = p_value;
}

void Rendering::LowRenderer::Camera::SetFar(float p_value)
{
	m_far = p_value;
}

void Rendering::LowRenderer::Camera::SetClearColor(const Maths::FVector3 & p_clearColor)
{
	m_clearColor = p_clearColor;
}

void Rendering::LowRenderer::Camera::SetFrustumGeometryCulling(bool p_enable)
{
	m_frustumGeometryCulling = p_enable;
}

void Rendering::LowRenderer::Camera::SetFrustumLightCulling(bool p_enable)
{
	m_frustumLightCulling = p_enable;
}

void Rendering::LowRenderer::Camera::SetProjectionMode(Rendering::Settings::EProjectionMode p_projectionMode)
{
    m_projectionMode = p_projectionMode;
}

Maths::FMatrix4 Rendering::LowRenderer::Camera::CalculateProjectionMatrix(uint16_t p_windowWidth, uint16_t p_windowHeight) const
{
    using namespace Maths;
    using namespace Rendering::Settings;

    const auto ratio = p_windowWidth / static_cast<float>(p_windowHeight);

    switch (m_projectionMode)
    {
    case EProjectionMode::ORTHOGRAPHIC:
        return FMatrix4::CreateOrthographic(m_size, ratio, m_near, m_far);

    case EProjectionMode::PERSPECTIVE: 
        return FMatrix4::CreatePerspective(m_fov, ratio, m_near, m_far);

    default:
        return FMatrix4::Identity;
    }
}

Maths::FMatrix4 Rendering::LowRenderer::Camera::CalculateViewMatrix(const Maths::FVector3& p_position, const Maths::FQuaternion& p_rotation) const
{
	const auto& up = p_rotation * Maths::FVector3::Up;
	const auto& forward = p_rotation * Maths::FVector3::Forward;

	return Maths::FMatrix4::CreateView
	(
		p_position.x, p_position.y, p_position.z,												// Position
		p_position.x + forward.x, p_position.y + forward.y, p_position.z + forward.z,			// LookAt (Position + Forward)
		up.x, up.y, up.z																		// Up Vector
	);
}