#pragma once
#include "AView.h"
#include "CameraController.h"

namespace OvEditor {
	namespace Panels
	{
		class AViewControllable : public OvEditor::Panels::AView
		{
		public:
			AViewControllable(
				const std::string& p_title);

			/**
			* Update the controllable view (Handle inputs)
			* @param p_deltaTime
			*/
			virtual void Update(float p_deltaTime) override;

			/**
			* Prepare the renderer for rendering
			*/
			virtual void InitFrame() override;

			/**
			* Reset the camera transform to its initial value
			*/
			virtual void ResetCameraTransform();

			/**
			* Returns the camera controller of the controllable view
			*/
			OvEditor::Core::CameraController& GetCameraController();

			/**
			* Returns the camera used by the camera controller
			*/
			virtual OvRendering::Entities::Camera* GetCamera();

			/**
			* Returns the grid color of the view
			*/
			const OvMaths::FVector3& GetGridColor() const;

			/**
			* Defines the grid color of the view
			* @param p_color
			*/
			void SetGridColor(const OvMaths::FVector3& p_color);

			/**
			* Reset the grid color to its initial value
			*/
			void ResetGridColor();

			/**
			* Set the camera clear color
			*/
			void ResetClearColor();

			void setCameraMode(OvRendering::Settings::EProjectionMode mode);

		protected:
			OvMaths::FVector3 m_gridColor;
			OvRendering::Entities::Camera m_camera;
			OvEditor::Core::CameraController m_cameraController;
		};
	}
}