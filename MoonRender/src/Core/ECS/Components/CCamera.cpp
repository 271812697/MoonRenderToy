#include <tinyxml2.h>
#include <Core/ECS/Actor.h>
#include <Core/ECS/Components/CCamera.h>


Core::ECS::Components::CCamera::CCamera(ECS::Actor& p_owner)
	: AComponent(p_owner), m_camera{ p_owner.transform.GetFTransform() }
{
	/* Default clear color for the CCamera (Different from Camera default clear color) */
	SetClearColor({ 0.1921569f, 0.3019608f, 0.4745098f });
}

std::string Core::ECS::Components::CCamera::GetName()
{
	return "Camera";
}

void Core::ECS::Components::CCamera::SetFov(float p_value)
{
	m_camera.SetFov(p_value);
}

void Core::ECS::Components::CCamera::SetSize(float p_value)
{
	m_camera.SetSize(p_value);
}

void Core::ECS::Components::CCamera::SetNear(float p_value)
{
	m_camera.SetNear(p_value);
}

void Core::ECS::Components::CCamera::SetFar(float p_value)
{
	m_camera.SetFar(p_value);
}

void Core::ECS::Components::CCamera::SetFrustumGeometryCulling(bool p_enable)
{
	m_camera.SetFrustumGeometryCulling(p_enable);
}

void Core::ECS::Components::CCamera::SetFrustumLightCulling(bool p_enable)
{
	m_camera.SetFrustumLightCulling(p_enable);
}

void Core::ECS::Components::CCamera::SetProjectionMode(Rendering::Settings::EProjectionMode p_projectionMode)
{
	m_camera.SetProjectionMode(p_projectionMode);
}

float Core::ECS::Components::CCamera::GetFov() const
{
	return m_camera.GetFov();
}

float Core::ECS::Components::CCamera::GetSize() const
{
	return m_camera.GetSize();
}

float Core::ECS::Components::CCamera::GetNear() const
{
	return m_camera.GetNear();
}

float Core::ECS::Components::CCamera::GetFar() const
{
	return m_camera.GetFar();
}

const Maths::FVector3& Core::ECS::Components::CCamera::GetClearColor() const
{
	return m_camera.GetClearColor();
}

bool Core::ECS::Components::CCamera::HasFrustumGeometryCulling() const
{
	return m_camera.HasFrustumGeometryCulling();
}

void Core::ECS::Components::CCamera::SetClearColor(const Maths::FVector3& p_clearColor)
{
	m_camera.SetClearColor(p_clearColor);
}

bool Core::ECS::Components::CCamera::HasFrustumLightCulling() const
{
	return m_camera.HasFrustumLightCulling();
}

Rendering::Settings::EProjectionMode Core::ECS::Components::CCamera::GetProjectionMode() const
{
	return m_camera.GetProjectionMode();
}

Rendering::Entities::Camera& Core::ECS::Components::CCamera::GetCamera()
{
	return m_camera;
}

void Core::ECS::Components::CCamera::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	Core::Helpers::Serializer::SerializeFloat(p_doc, p_node, "f", m_camera.GetFov());
	Core::Helpers::Serializer::SerializeFloat(p_doc, p_node, "size", m_camera.GetSize());
	Core::Helpers::Serializer::SerializeFloat(p_doc, p_node, "near", m_camera.GetNear());
	Core::Helpers::Serializer::SerializeFloat(p_doc, p_node, "far", m_camera.GetFar());
	Core::Helpers::Serializer::SerializeVec3(p_doc, p_node, "clear_color", m_camera.GetClearColor());
	Core::Helpers::Serializer::SerializeBoolean(p_doc, p_node, "frustum_geometry_culling", m_camera.HasFrustumGeometryCulling());
	Core::Helpers::Serializer::SerializeBoolean(p_doc, p_node, "frustum_light_culling", m_camera.HasFrustumLightCulling());
	Core::Helpers::Serializer::SerializeInt(p_doc, p_node, "projection_mode", static_cast<int>(m_camera.GetProjectionMode()));
}

void Core::ECS::Components::CCamera::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	m_camera.SetFov(Core::Helpers::Serializer::DeserializeFloat(p_doc, p_node, "f"));
	m_camera.SetSize(Core::Helpers::Serializer::DeserializeFloat(p_doc, p_node, "size"));
	m_camera.SetNear(Core::Helpers::Serializer::DeserializeFloat(p_doc, p_node, "near"));
	m_camera.SetFar(Core::Helpers::Serializer::DeserializeFloat(p_doc, p_node, "far"));
	m_camera.SetClearColor(Core::Helpers::Serializer::DeserializeVec3(p_doc, p_node, "clear_color"));
	m_camera.SetFrustumGeometryCulling(Core::Helpers::Serializer::DeserializeBoolean(p_doc, p_node, "frustum_geometry_culling"));
	m_camera.SetFrustumLightCulling(Core::Helpers::Serializer::DeserializeBoolean(p_doc, p_node, "frustum_light_culling"));

	// We have to make sure the "projection_mode" exists in the serialized component, otherwise we do not want to modify the default setting (Perspective).
	// This is a bad practice to have each components calling setters in `OnDeserialize` even if no XML node hasn't been found for a given property.
	// We should rework this system later. As it is out of the scope of the orthographic projection scope, this will be left as is for now.
	if (p_node->FirstChildElement("projection_mode"))
	{
		m_camera.SetProjectionMode(static_cast<Rendering::Settings::EProjectionMode>(Core::Helpers::Serializer::DeserializeInt(p_doc, p_node, "projection_mode")));
	}
}

