#pragma once
#include <Maths/FVector3.h>
#include <Maths/FMatrix4.h>
#include <Maths/FQuaternion.h>
#include <Tools/Utils/OptRef.h>

#include "Rendering/Data/Frustum.h"
#include "Rendering/Settings/EProjectionMode.h"
#include "Rendering/Entities/Entity.h"

namespace Rendering::Entities
{

	class Camera : public Rendering::Entities::Entity
	{
	public:

		Camera(Tools::Utils::OptRef<Maths::FTransform> p_transform = std::nullopt);
		void CacheMatrices(uint16_t p_windowWidth, uint16_t p_windowHeight);
		void CacheProjectionMatrix(uint16_t p_windowWidth, uint16_t p_windowHeight);
		void CacheViewMatrix();
		void CacheFrustum(const Maths::FMatrix4& p_view, const Maths::FMatrix4& p_projection);
		const Maths::FVector3& GetPosition() const;
		const Maths::FQuaternion& GetRotation() const;
		float GetFov() const;
		float GetSize() const;
		float GetNear() const;
		float GetFar() const;
		const Maths::FVector3& GetClearColor() const;
		bool GetClearColorBuffer() const;
		bool GetClearDepthBuffer() const;
		bool GetClearStencilBuffer() const;
		const Maths::FMatrix4& GetProjectionMatrix() const;
		const Maths::FMatrix4& GetViewMatrix() const;
		const Maths::FTransform& GetTransform();
		const Rendering::Data::Frustum& GetFrustum() const;
		Tools::Utils::OptRef<const Rendering::Data::Frustum> GetGeometryFrustum() const;
		Tools::Utils::OptRef<const Rendering::Data::Frustum> GetLightFrustum() const;
		bool HasFrustumGeometryCulling() const;
		bool HasFrustumLightCulling() const;
		Rendering::Settings::EProjectionMode GetProjectionMode() const;
		void SetPosition(const Maths::FVector3& p_position);
		void SetRotation(const Maths::FQuaternion& p_rotation);
		void SetFov(float p_value);
		void SetSize(float p_value);
		void SetNear(float p_value);
		void SetFar(float p_value);
		void SetClearColor(const Maths::FVector3& p_clearColor);
		void SetClearColorBuffer(bool p_value);
		void SetClearDepthBuffer(bool p_value);
		void SetClearStencilBuffer(bool p_value);
		void SetFrustumGeometryCulling(bool p_enable);
		void SetFrustumLightCulling(bool p_enable);
		void SetProjectionMode(Rendering::Settings::EProjectionMode p_projectionMode);
		void ProjectionFitToSphere(Rendering::Geometry::BoundingSphere& sphere,const Maths::FVector3& dir);
		void PersertiveZoom(float delta);
		void OrthZoom(float delta, int x, int y);
	private:
		Maths::FMatrix4 CalculateProjectionMatrix(uint16_t p_windowWidth, uint16_t p_windowHeight);
		Maths::FMatrix4 CalculateViewMatrix() const;
	private:
		Rendering::Data::Frustum m_frustum;
		Maths::FMatrix4 m_viewMatrix;
		Maths::FMatrix4 m_projectionMatrix;
		Rendering::Settings::EProjectionMode m_projectionMode;
		float m_ratio;
		int m_windowHeight;
		int m_windowWidth;
		float m_fov;
		float m_size;
		float m_near;
		float m_far;
		bool m_frustumLightCulling;
		bool m_frustumGeometryCulling;
		Maths::FVector3 m_clearColor;
		// Buffer clearing falgs
		bool m_clearColorBuffer;
		bool m_clearDepthBuffer;
		bool m_clearStencilBuffer;
	};
}