/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <soloud.h>

#include <Audio/Core/AudioEngine.h>
#include <Audio/Data/SoundInstance.h>
#include <Debug/Assertion.h>
#include <Tools/Utils/EnumMapper.h>

using SoLoudAttenuationType = std::underlying_type<SoLoud::AudioSource::ATTENUATION_MODELS>::type;

template <>
struct Tools::Utils::MappingFor<Audio::Settings::EAttenuationModel, SoLoudAttenuationType>
{
	using enum SoLoud::AudioSource::ATTENUATION_MODELS;
	using EnumType = Audio::Settings::EAttenuationModel;
	using type = std::tuple<
		EnumValuePair<EnumType::NONE, NO_ATTENUATION>,
		EnumValuePair<EnumType::INVERSE_DISTANCE, INVERSE_DISTANCE>,
		EnumValuePair<EnumType::LINEAR_DISTANCE, LINEAR_DISTANCE>,
		EnumValuePair<EnumType::EXPONENTIAL_DISTANCE, EXPONENTIAL_DISTANCE>
	>;
};

namespace
{
	constexpr SoLoudAttenuationType GetAttenuationModelValue(Audio::Settings::EAttenuationModel p_model)
	{
		return Tools::Utils::ToValueImpl<Audio::Settings::EAttenuationModel, SoLoudAttenuationType>(p_model);
	}
}

Audio::Data::SoundInstance::SoundInstance(SoLoud::Soloud& p_backend, SoundHandle p_handle, bool p_spatial) :
	m_backend(p_backend),
	m_handle(p_handle),
	m_spatial(p_spatial)
{
}

void Audio::Data::SoundInstance::SetVolume(float p_volume)
{
	Validate();
	m_backend.setVolume(m_handle, p_volume);
}

void Audio::Data::SoundInstance::SetLooped(bool p_looped)
{
	Validate();
	m_backend.setLooping(m_handle, p_looped);
}

void Audio::Data::SoundInstance::SetPitch(float p_pitch)
{
	Validate();
	m_backend.setRelativePlaySpeed(m_handle, p_pitch);
}

void Audio::Data::SoundInstance::SetPan(float p_pan)
{
	if (!m_spatial)
	{
		Validate();
		m_backend.setPan(m_handle, p_pan);
	}
}

void Audio::Data::SoundInstance::SetAttenuationModel(Audio::Settings::EAttenuationModel p_model, float p_factor)
{
	using namespace Tools::Utils;

	if (m_spatial)
	{
		Validate();
		m_backend.set3dSourceAttenuation(
			m_handle,
			GetAttenuationModelValue(p_model),
			p_factor
		);
	}
}

void Audio::Data::SoundInstance::SetAttenuationThreshold(float p_distance)
{
	// Matches irrKlang default value for compatibility.
	// Don't change this value if you don't know what you are doing:
	// This value causes the sound to stop attenuating after it reaches the max distance.
	constexpr float kMaxDistance = 1000000000.0f;

	if (m_spatial)
	{
		Validate();
		m_backend.set3dSourceMinMaxDistance(m_handle, p_distance, kMaxDistance);
	}
}

void Audio::Data::SoundInstance::SetSpatialParameters(
	const Maths::FVector3& p_position,
	const Maths::FVector3& p_velocity
) const
{
	if (m_spatial)
	{
		Validate();
		m_backend.set3dSourceParameters(
			m_handle,
			p_position.x, p_position.y, p_position.z,
			p_velocity.x, p_velocity.y, p_velocity.z);
	}
}

void Audio::Data::SoundInstance::Play()
{
	Validate();
	m_backend.setPause(m_handle, false);
}

void Audio::Data::SoundInstance::Pause()
{
	Validate();
	m_backend.setPause(m_handle, true);
}

void Audio::Data::SoundInstance::Stop()
{
	Validate();
	m_backend.stop(m_handle);
	m_backend.destroyVoiceGroup(m_handle);
}

bool Audio::Data::SoundInstance::IsValid() const
{
	return m_backend.isValidVoiceHandle(m_handle);
}

bool Audio::Data::SoundInstance::IsPaused() const
{
	Validate();
	return m_backend.getPause(m_handle);
}

bool Audio::Data::SoundInstance::IsSpatial() const
{
	return m_spatial;
}

Audio::Data::SoundHandle Audio::Data::SoundInstance::GetHandle() const
{
	return m_handle;
}

void Audio::Data::SoundInstance::Validate() const
{
	ASSERT(IsValid(), "Sound instance is not valid");
}
