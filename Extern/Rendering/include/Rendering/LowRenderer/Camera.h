

#pragma once

#include <Maths/FVector3.h>
#include <Maths/FMatrix4.h>
#include <Maths/FQuaternion.h>


#include"../../tools/ReferenceOrValue.h"

#include "Rendering/Data/Frustum.h"
#include "Rendering/Settings/EProjectionMode.h"

namespace Rendering::LowRenderer
{
	/**
	* Represents a camera. Wraps projection and view calculation based on applied rotation and the given positions
	*/
	class Camera
	{
	public:
		/**
		* Constructor
		*/
		Camera();

		/**
		* Cache the projection, view and frustum matrices
		* @param p_windowWidth
		* @param p_windowHeight
		* @param p_position
		* @param p_rotation
		*/
		void CacheMatrices(uint16_t p_windowWidth, uint16_t p_windowHeight, const Maths::FVector3& p_position, const Maths::FQuaternion& p_rotation);

		/**
		* Calculate and cache the result projection matrix
		* @param p_windowWidth
		* @param p_windowHeight
		*/
		void CacheProjectionMatrix(uint16_t p_windowWidth, uint16_t p_windowHeight);

		/**
		* Calculate and cache the result view matrix
		* @param p_position
		* @param p_rotation
		*/
		void CacheViewMatrix(const Maths::FVector3& p_position, const Maths::FQuaternion& p_rotation);

		/**
		* Calculate and cache the result frustum.
		* This method should be called after projection and view matrices are cached.
		* @param p_view
		* @param p_projection
		*/
		void CacheFrustum(const Maths::FMatrix4& p_view, const Maths::FMatrix4& p_projection);

		/**
		* Returns the fov of the camera
		*/
		float GetFov() const;

        /**
        * Returns the size of the camera
        */
        float GetSize() const;

		/**
		* Returns the near of the camera
		*/
		float GetNear() const;

		/**
		* Returns the far of the camera
		*/
		float GetFar() const;

		/**
		* Returns the clear color of the camera
		*/
		const Maths::FVector3& GetClearColor() const;

		/**
		* Returns the cached projection matrix
		*/
		const Maths::FMatrix4& GetProjectionMatrix() const;

		/**
		* Returns the cached view matrix
		*/
		const Maths::FMatrix4& GetViewMatrix() const;

		/**
		* Retursn the cached frustum
		*/
		const Rendering::Data::Frustum& GetFrustum() const;

		/**
		* Returns true if the frustum culling for geometry is enabled
		*/
		bool HasFrustumGeometryCulling() const;

		/**
		* Returns true if the frustum culling for lights is enabled
		*/
		bool HasFrustumLightCulling() const;

        /**
        * Returns the current projection mode
        */
        Rendering::Settings::EProjectionMode GetProjectionMode() const;

		/**
		* Sets the fov of the camera to the given value
		* @param p_value
		*/
		void SetFov(float p_value);

        /**
        * Sets the size of the camera to the given value
        * @param p_value
        */
        void SetSize(float p_value);

		/**
		* Sets the near of the camera to the given value
		* @param p_value
		*/
		void SetNear(float p_value);

		/**
		* Sets the far of the camera to the given value
		* @param p_value
		*/
		void SetFar(float p_value);

		/**
		* Sets the clear color of the camera to the given value
		* @param p_value
		*/
		void SetClearColor(const Maths::FVector3& p_clearColor);

		/**
		* Defines if the camera should apply frustum culling to geometry while rendering
		* @param p_enable
		*/
		void SetFrustumGeometryCulling(bool p_enable);

		/**
		* Defines if the camera should apply frustum culling to lights while rendering
		* @param p_enable
		*/
		void SetFrustumLightCulling(bool p_enable);

        /**
        * Defines the projection mode the camera should adopt
        * @param p_projectionMode
        */
        void SetProjectionMode(Rendering::Settings::EProjectionMode p_projectionMode);

	private:
		Maths::FMatrix4 CalculateProjectionMatrix(uint16_t p_windowWidth, uint16_t p_windowHeight) const;
		Maths::FMatrix4 CalculateViewMatrix(const Maths::FVector3& p_position, const Maths::FQuaternion& p_rotation) const;

	private:
		Rendering::Data::Frustum m_frustum;
		Maths::FMatrix4 m_viewMatrix;
		Maths::FMatrix4 m_projectionMatrix;
        Rendering::Settings::EProjectionMode m_projectionMode;

		float m_fov;
        float m_size;
		float m_near;
		float m_far;

		Maths::FVector3 m_clearColor;

		bool m_frustumGeometryCulling;
		bool m_frustumLightCulling;
	};
}