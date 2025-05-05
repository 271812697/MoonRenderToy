/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <soloud.h>
#include <soloud_wav.h>

#include <Audio/Core/AudioEngine.h>
#include <Audio/Entities/AudioSource.h>
#include <Debug/Assertion.h>

Tools::Eventing::Event<Audio::Entities::AudioSource&> Audio::Entities::AudioSource::CreatedEvent;
Tools::Eventing::Event<Audio::Entities::AudioSource&> Audio::Entities::AudioSource::DestroyedEvent;

Audio::Entities::AudioSource::AudioSource(Core::AudioEngine& p_engine, Tools::Utils::OptRef<Maths::FTransform> p_transform) :
	m_engine(p_engine),
	m_transform(p_transform)
{
	CreatedEvent.Invoke(*this);
}

Audio::Entities::AudioSource::~AudioSource()
{
	Stop();
	DestroyedEvent.Invoke(*this);
}

const Maths::FTransform& Audio::Entities::AudioSource::GetTransform()
{
	return m_transform;
}

void Audio::Entities::AudioSource::ApplySourceSettingsToTrackedSound()
{
	m_instance->SetVolume(m_volume);
	m_instance->SetPan(m_pan);
	m_instance->SetLooped(m_looped);
	m_instance->SetPitch(m_pitch);
	m_instance->SetAttenuationThreshold(m_attenuationThreshold);
}

bool Audio::Entities::AudioSource::HasSound() const
{
	return m_instance && m_instance->IsValid();
}

bool Audio::Entities::AudioSource::IsPlaying() const
{
	return HasSound() && !IsPaused();
}

void Audio::Entities::AudioSource::SetSpatial(bool p_value)
{
	m_spatial = p_value;
	// TODO: Cannot currently change the spatialization mode of a sound instance
}

void Audio::Entities::AudioSource::SetAttenuationThreshold(float p_distance)
{
	m_attenuationThreshold = p_distance;

	if (HasSound())
	{
		m_instance->SetAttenuationThreshold(m_attenuationThreshold);
	}
}

void Audio::Entities::AudioSource::SetVolume(float p_volume)
{
	m_volume = p_volume;

	if (HasSound())
	{
		m_instance->SetVolume(m_volume);
	}
}

void Audio::Entities::AudioSource::SetPan(float p_pan)
{
	m_pan = p_pan;

	if (HasSound())
	{
		m_instance->SetPan(m_pan);
	}
}

void Audio::Entities::AudioSource::SetLooped(bool p_looped)
{
	m_looped = p_looped;

	if (HasSound())
	{
		m_instance->SetLooped(p_looped);
	}
}

void Audio::Entities::AudioSource::SetPitch(float p_pitch)
{
	m_pitch = p_pitch;

	if (HasSound())
	{
		m_instance->SetPitch(p_pitch);
	}
}

bool Audio::Entities::AudioSource::IsPaused() const
{
	ASSERT(HasSound(), "Cannot check if the sound is paused if no sound is currently being tracked");
	return m_instance->IsPaused();
}

std::weak_ptr<Audio::Data::SoundInstance> Audio::Entities::AudioSource::GetSoundInstance() const
{
	return m_instance;
}

bool Audio::Entities::AudioSource::IsSpatial() const
{
	return m_spatial;
}

float Audio::Entities::AudioSource::GetAttenuationThreshold() const
{
	return m_attenuationThreshold;
}

float Audio::Entities::AudioSource::GetVolume() const
{
	return m_volume;
}

float Audio::Entities::AudioSource::GetPan() const
{
	return m_pan;
}

bool Audio::Entities::AudioSource::IsLooped() const
{
	return m_looped;
}

float Audio::Entities::AudioSource::GetPitch() const
{
	return m_pitch;
}

void Audio::Entities::AudioSource::Play(const Resources::Sound& p_sound)
{
	Stop();

	if (m_spatial)
	{
		m_instance = m_engine.Play3D(
			p_sound,
			m_transform->GetWorldPosition(),
			Maths::FVector3::Zero, // TODO: Add support for non-zero velocity
			m_volume,
			true
		);

		if (m_instance)
		{
			m_instance->SetAttenuationThreshold(m_attenuationThreshold);
			m_instance->SetAttenuationModel(
				Settings::EAttenuationModel::EXPONENTIAL_DISTANCE // TODO: Expose attenuation model
			);

			// Potentially expensive? But necessary so that the settings set above are applied.
			// Otherwise some spatialized sounds may play without attenuation until the next update.
			// This isn't ideal, but I couldn't find a better way to do it (no `update3dAudio` for a single sound instance).
			m_engine.GetBackend().update3dAudio();
		}
	}
	else
	{
		m_instance = m_engine.Play2D(p_sound, m_pan, m_volume, true);
	}

	if (m_instance)
	{
		m_instance->SetLooped(m_looped);
		m_instance->SetPitch(m_pitch);
		m_instance->Play();
	}
}

void Audio::Entities::AudioSource::Resume()
{
	if (HasSound())
	{
		m_instance->Play();
	}
}

void Audio::Entities::AudioSource::Pause()
{
	if (HasSound())
	{
		m_instance->Pause();
	}
}

void Audio::Entities::AudioSource::Stop()
{
	if (HasSound())
	{
		m_instance->Stop();
	}
}

void Audio::Entities::AudioSource::Update()
{
	if (HasSound() && m_instance->IsSpatial())
	{
		m_instance->SetSpatialParameters(
			m_transform->GetWorldPosition(),
			Maths::FVector3::Zero // TODO: Add support for non-zero velocity
		);
	}
}
