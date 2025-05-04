/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/
#include <Core/ECS/Components/CPostProcessStack.h>

::Core::ECS::Components::CPostProcessStack::CPostProcessStack(ECS::Actor& p_owner) : AComponent(p_owner)
{
}

std::string Core::ECS::Components::CPostProcessStack::GetName()
{
	return "Post Process Stack";
}

void Core::ECS::Components::CPostProcessStack::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	auto& bloomSettings = m_settings.Get<Rendering::PostProcess::BloomEffect, Rendering::PostProcess::BloomSettings>();
	auto& autoExposureSettings = m_settings.Get<Rendering::PostProcess::AutoExposureEffect, Rendering::PostProcess::AutoExposureSettings>();
	auto& fxaaSettings = m_settings.Get<Rendering::PostProcess::FXAAEffect, Rendering::PostProcess::FXAASettings>();
	auto& tonemappingSettings = m_settings.Get<Rendering::PostProcess::TonemappingEffect, Rendering::PostProcess::TonemappingSettings>();

	Helpers::Serializer::SerializeBoolean(p_doc, p_node, "bloom_enabled", bloomSettings.enabled);
	Helpers::Serializer::SerializeFloat(p_doc, p_node, "bloom_threshold", bloomSettings.threshold);
	Helpers::Serializer::SerializeFloat(p_doc, p_node, "bloom_radius", bloomSettings.radius);
	Helpers::Serializer::SerializeInt(p_doc, p_node, "bloom_kernel_size", bloomSettings.kernelSize);
	Helpers::Serializer::SerializeFloat(p_doc, p_node, "bloom_intensity", bloomSettings.intensity);

	Helpers::Serializer::SerializeBoolean(p_doc, p_node, "auto_exposure_enabled", autoExposureSettings.enabled);
	Helpers::Serializer::SerializeFloat(p_doc, p_node, "auto_exposure_center_weight_bias", autoExposureSettings.centerWeightBias);
	Helpers::Serializer::SerializeFloat(p_doc, p_node, "auto_exposure_luminance_min", autoExposureSettings.minLuminanceEV);
	Helpers::Serializer::SerializeFloat(p_doc, p_node, "auto_exposure_luminance_max", autoExposureSettings.maxLuminanceEV);
	Helpers::Serializer::SerializeFloat(p_doc, p_node, "auto_exposure_exposure_compensation", autoExposureSettings.exposureCompensationEV);
	Helpers::Serializer::SerializeBoolean(p_doc, p_node, "auto_exposure_progressive", autoExposureSettings.progressive);
	Helpers::Serializer::SerializeFloat(p_doc, p_node, "auto_exposure_speed_up", autoExposureSettings.speedUp);
	Helpers::Serializer::SerializeFloat(p_doc, p_node, "auto_exposure_speed_down", autoExposureSettings.speedDown);

	Helpers::Serializer::SerializeBoolean(p_doc, p_node, "tonemapping_enabled", tonemappingSettings.enabled);
	Helpers::Serializer::SerializeFloat(p_doc, p_node, "tonemapping_exposure", tonemappingSettings.exposure);
	Helpers::Serializer::SerializeInt(p_doc, p_node, "tonemapping_mode", static_cast<int>(tonemappingSettings.mode));
	Helpers::Serializer::SerializeBoolean(p_doc, p_node, "tonemapping_gamma_correction", tonemappingSettings.gammaCorrection);

	Helpers::Serializer::SerializeBoolean(p_doc, p_node, "fxaa_enabled", fxaaSettings.enabled);
}

void Core::ECS::Components::CPostProcessStack::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	auto& bloomSettings = m_settings.Get<Rendering::PostProcess::BloomEffect, Rendering::PostProcess::BloomSettings>();
	auto& autoExposureSettings = m_settings.Get<Rendering::PostProcess::AutoExposureEffect, Rendering::PostProcess::AutoExposureSettings>();
	auto& fxaaSettings = m_settings.Get<Rendering::PostProcess::FXAAEffect, Rendering::PostProcess::FXAASettings>();
	auto& tonemappingSettings = m_settings.Get<Rendering::PostProcess::TonemappingEffect, Rendering::PostProcess::TonemappingSettings>();

	Helpers::Serializer::DeserializeBoolean(p_doc, p_node, "bloom_enabled", bloomSettings.enabled);
	Helpers::Serializer::DeserializeFloat(p_doc, p_node, "bloom_threshold", bloomSettings.threshold);
	Helpers::Serializer::DeserializeFloat(p_doc, p_node, "bloom_radius", bloomSettings.radius);
	Helpers::Serializer::DeserializeInt(p_doc, p_node, "bloom_kernel_size", bloomSettings.kernelSize);
	Helpers::Serializer::DeserializeFloat(p_doc, p_node, "bloom_intensity", bloomSettings.intensity);

	Helpers::Serializer::DeserializeBoolean(p_doc, p_node, "auto_exposure_enabled", autoExposureSettings.enabled);
	Helpers::Serializer::DeserializeFloat(p_doc, p_node, "auto_exposure_center_weight_bias", autoExposureSettings.centerWeightBias);
	Helpers::Serializer::DeserializeFloat(p_doc, p_node, "auto_exposure_min_luminance", autoExposureSettings.minLuminanceEV);
	Helpers::Serializer::DeserializeFloat(p_doc, p_node, "auto_exposure_max_luminance", autoExposureSettings.maxLuminanceEV);
	Helpers::Serializer::DeserializeFloat(p_doc, p_node, "auto_exposure_exposure_compensation", autoExposureSettings.exposureCompensationEV);
	Helpers::Serializer::DeserializeBoolean(p_doc, p_node, "auto_exposure_progressive", autoExposureSettings.progressive);
	Helpers::Serializer::DeserializeFloat(p_doc, p_node, "auto_exposure_speed_up", autoExposureSettings.speedUp);
	Helpers::Serializer::DeserializeFloat(p_doc, p_node, "auto_exposure_speed_down", autoExposureSettings.speedDown);

	Helpers::Serializer::DeserializeBoolean(p_doc, p_node, "tonemapping_enabled", tonemappingSettings.enabled);
	Helpers::Serializer::DeserializeFloat(p_doc, p_node, "tonemapping_exposure", tonemappingSettings.exposure);
	Helpers::Serializer::DeserializeInt(p_doc, p_node, "tonemapping_mode", reinterpret_cast<int&>(tonemappingSettings.mode));
	Helpers::Serializer::DeserializeBoolean(p_doc, p_node, "tonemapping_gamma_correction", tonemappingSettings.gammaCorrection);

	Helpers::Serializer::DeserializeBoolean(p_doc, p_node, "fxaa_enabled", fxaaSettings.enabled);
}


const Core::Rendering::PostProcess::PostProcessStack& Core::ECS::Components::CPostProcessStack::GetStack()
{
	return m_settings;
}

const Core::Rendering::PostProcess::BloomSettings& Core::ECS::Components::CPostProcessStack::GetBloomSettings() const
{
	return m_settings.Get<Rendering::PostProcess::BloomEffect, Rendering::PostProcess::BloomSettings>();
}

const Core::Rendering::PostProcess::AutoExposureSettings& Core::ECS::Components::CPostProcessStack::GetAutoExposureSettings() const
{
	return m_settings.Get<Rendering::PostProcess::AutoExposureEffect, Rendering::PostProcess::AutoExposureSettings>();
}

const Core::Rendering::PostProcess::TonemappingSettings& Core::ECS::Components::CPostProcessStack::GetTonemappingSettings() const
{
	return m_settings.Get<Rendering::PostProcess::TonemappingEffect, Rendering::PostProcess::TonemappingSettings>();
}

const Core::Rendering::PostProcess::FXAASettings& Core::ECS::Components::CPostProcessStack::GetFXAASettings() const
{
	return m_settings.Get<Rendering::PostProcess::FXAAEffect, Rendering::PostProcess::FXAASettings>();
}

void Core::ECS::Components::CPostProcessStack::SetBloomSettings(const Core::Rendering::PostProcess::BloomSettings& p_settings)
{
	m_settings.Set<Rendering::PostProcess::BloomEffect>(p_settings);
}

void Core::ECS::Components::CPostProcessStack::SetAutoExposureSettings(const Core::Rendering::PostProcess::AutoExposureSettings& p_settings)
{
	m_settings.Set<Rendering::PostProcess::AutoExposureSettings>(p_settings);
}

void Core::ECS::Components::CPostProcessStack::SetTonemappingSettings(const Core::Rendering::PostProcess::TonemappingSettings& p_settings)
{
	m_settings.Set<Rendering::PostProcess::TonemappingEffect>(p_settings);
}

void Core::ECS::Components::CPostProcessStack::SetFXAASettings(const Core::Rendering::PostProcess::FXAASettings& p_settings)
{
	m_settings.Set<Rendering::PostProcess::FXAAEffect>(p_settings);
}
