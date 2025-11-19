#include "AViewControllable.h"
#include "DebugSceneRenderer.h"
#include "GridRenderPass.h"

const Maths::FVector3 kDefaultGridColor{ 1.0f, 1.0f, 1.0f };
const Maths::FVector3 kDefaultClearColor{ 0.0f, 0.0f, 0.0f };
const Maths::FVector3 kDefaultCameraPosition{ -10.0f, 3.0f, 10.0f };
const Maths::FQuaternion kDefaultCameraRotation({ 0.0f, 135.0f, 0.0f });

Editor::Panels::AViewControllable::AViewControllable(
	const std::string& p_title) :
	AView(p_title),
	m_cameraController(*this, m_camera)
{
	ResetCameraTransform();
	ResetGridColor();
	ResetClearColor();
}

void Editor::Panels::AViewControllable::Update(float p_deltaTime)
{
	m_cameraController.HandleInputs(p_deltaTime);
	AView::Update(p_deltaTime);
}

void Editor::Panels::AViewControllable::InitFrame()
{
	m_camera.SetFrustumGeometryCulling(false);
	m_camera.SetFrustumLightCulling(false);
	AView::InitFrame();
	m_renderer->AddDescriptor<Rendering::GridRenderPass::GridDescriptor>({
		m_gridColor,
		m_camera.GetPosition()
		});
}

void Editor::Panels::AViewControllable::ResetCameraTransform()
{
	m_camera.transform->SetWorldPosition(kDefaultCameraPosition);
	m_camera.transform->SetWorldRotation(kDefaultCameraRotation);
}

Editor::Core::CameraController& Editor::Panels::AViewControllable::GetCameraController()
{
	return m_cameraController;
}

Rendering::Entities::Camera* Editor::Panels::AViewControllable::GetCamera()
{
	return &m_camera;
}

const Maths::FVector3& Editor::Panels::AViewControllable::GetGridColor() const
{
	return m_gridColor;
}

void Editor::Panels::AViewControllable::SetGridColor(const Maths::FVector3& p_color)
{
	m_gridColor = p_color;
}

void Editor::Panels::AViewControllable::ResetGridColor()
{
	m_gridColor = kDefaultGridColor;
}

void Editor::Panels::AViewControllable::ResetClearColor()
{
	m_camera.SetClearColor(kDefaultClearColor);
}

void Editor::Panels::AViewControllable::setCameraMode(::Rendering::Settings::EProjectionMode mode)
{
	m_camera.SetProjectionMode(mode);
}
