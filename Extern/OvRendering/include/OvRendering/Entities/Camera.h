#pragma once
#include <OvMaths/FVector3.h>
#include <OvMaths/FMatrix4.h>
#include <OvMaths/FQuaternion.h>
#include <OvTools/Utils/OptRef.h>

#include "OvRendering/Data/Frustum.h"
#include "OvRendering/Settings/EProjectionMode.h"
#include "OvRendering/Entities/Entity.h"

namespace OvRendering::Entities
{

	class Camera : public OvRendering::Entities::Entity
	{
	public:

		Camera(OvTools::Utils::OptRef<OvMaths::FTransform> p_transform = std::nullopt);
		void CacheMatrices(uint16_t p_windowWidth, uint16_t p_windowHeight);
		void CacheProjectionMatrix(uint16_t p_windowWidth, uint16_t p_windowHeight);
		void CacheViewMatrix();
		void CacheFrustum(const OvMaths::FMatrix4& p_view, const OvMaths::FMatrix4& p_projection);
		const OvMaths::FVector3& GetPosition() const;
		const OvMaths::FQuaternion& GetRotation() const;
		float GetFov() const;
		float GetSize() const;
		float GetNear() const;
		float GetFar() const;
		const OvMaths::FVector3& GetClearColor() const;
		bool GetClearColorBuffer() const;
		bool GetClearDepthBuffer() const;
		bool GetClearStencilBuffer() const;
		const OvMaths::FMatrix4& GetProjectionMatrix() const;
		const OvMaths::FMatrix4& GetViewMatrix() const;
		const OvMaths::FTransform& GetTransform();
		const OvRendering::Data::Frustum& GetFrustum() const;
		OvTools::Utils::OptRef<const OvRendering::Data::Frustum> GetGeometryFrustum() const;
		OvTools::Utils::OptRef<const OvRendering::Data::Frustum> GetLightFrustum() const;
		bool HasFrustumGeometryCulling() const;
		bool HasFrustumLightCulling() const;
		OvRendering::Settings::EProjectionMode GetProjectionMode() const;
		void SetPosition(const OvMaths::FVector3& p_position);
		void SetRotation(const OvMaths::FQuaternion& p_rotation);
		void SetFov(float p_value);
		void SetSize(float p_value);
		void SetNear(float p_value);
		void SetFar(float p_value);
		void SetClearColor(const OvMaths::FVector3& p_clearColor);
		void SetClearColorBuffer(bool p_value);
		void SetClearDepthBuffer(bool p_value);
		void SetClearStencilBuffer(bool p_value);
		void SetFrustumGeometryCulling(bool p_enable);
		void SetFrustumLightCulling(bool p_enable);
		void SetProjectionMode(OvRendering::Settings::EProjectionMode p_projectionMode);
		void ProjectionFitToSphere(OvRendering::Geometry::BoundingSphere& sphere,const OvMaths::FVector3& dir);
		void PersertiveZoom(float delta);
		void OrthZoom(float delta, int x, int y);
	private:
		OvMaths::FMatrix4 CalculateProjectionMatrix(uint16_t p_windowWidth, uint16_t p_windowHeight);
		OvMaths::FMatrix4 CalculateViewMatrix() const;
	private:
		OvRendering::Data::Frustum m_frustum;
		OvMaths::FMatrix4 m_viewMatrix;
		OvMaths::FMatrix4 m_projectionMatrix;
		OvRendering::Settings::EProjectionMode m_projectionMode;
		float m_ratio;
		int m_windowHeight;
		int m_windowWidth;
		float m_fov;
		float m_size;
		float m_near;
		float m_far;
		bool m_frustumLightCulling;
		bool m_frustumGeometryCulling;
		OvMaths::FVector3 m_clearColor;
		// Buffer clearing falgs
		bool m_clearColorBuffer;
		bool m_clearDepthBuffer;
		bool m_clearStencilBuffer;
	};
}