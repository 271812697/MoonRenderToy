

#include <irrklang/ik_vec3d.h>

#include "Audio/Core/AudioPlayer.h"
#include "Audio/Entities/AudioSource.h"

Tools::Eventing::Event<Audio::Entities::AudioSource&> Audio::Entities::AudioSource::CreatedEvent;
Tools::Eventing::Event<Audio::Entities::AudioSource&> Audio::Entities::AudioSource::DestroyedEvent;

Audio::Entities::AudioSource::AudioSource(Core::AudioPlayer& p_audioPlayer) :
	m_audioPlayer(p_audioPlayer),
	m_transform(new Maths::FTransform()),
	m_internalTransform(true)
{
	Setup();
}

Audio::Entities::AudioSource::AudioSource(Core::AudioPlayer& p_audioPlayer, Maths::FTransform& p_transform) :
	m_audioPlayer(p_audioPlayer),
	m_transform(&p_transform),
	m_internalTransform(false)
{
	Setup();
}

void Audio::Entities::AudioSource::Setup()
{
	CreatedEvent.Invoke(*this);
}

Audio::Entities::AudioSource::~AudioSource()
{
	DestroyedEvent.Invoke(*this);

	StopAndDestroyTrackedSound();

	if (m_internalTransform)
		delete m_transform;
}

void Audio::Entities::AudioSource::UpdateTrackedSoundPosition()
{
	if (IsTrackingSound())
		m_trackedSound->GetTrack()->setPosition(reinterpret_cast<const irrklang::vec3df&>(m_transform->GetWorldPosition())); // FVector3 and vec3df have the same data layout
}

void Audio::Entities::AudioSource::ApplySourceSettingsToTrackedSound()
{
	m_trackedSound->GetTrack()->setVolume(m_volume);
	m_trackedSound->GetTrack()->setPan(m_pan);
	m_trackedSound->GetTrack()->setIsLooped(m_looped);
	m_trackedSound->GetTrack()->setPlaybackSpeed(m_pitch);
	m_trackedSound->GetTrack()->setMinDistance(m_attenuationThreshold);
}

void Audio::Entities::AudioSource::SetSpatial(bool p_value)
{
	m_spatial = p_value;
}

void Audio::Entities::AudioSource::SetAttenuationThreshold(float p_distance)
{
	m_attenuationThreshold = p_distance;

	if (IsTrackingSound())
		m_trackedSound->GetTrack()->setMinDistance(p_distance);
}

Audio::Tracking::SoundTracker* Audio::Entities::AudioSource::GetTrackedSound() const
{
	return m_trackedSound.get();
}

void Audio::Entities::AudioSource::SetVolume(float p_volume)
{
	m_volume = p_volume;

	if (IsTrackingSound())
		m_trackedSound->GetTrack()->setVolume(p_volume);
}

void Audio::Entities::AudioSource::SetPan(float p_pan)
{
	m_pan = p_pan;

	if (IsTrackingSound())
		m_trackedSound->GetTrack()->setPan(p_pan * -1.0f);
}

void Audio::Entities::AudioSource::SetLooped(bool p_looped)
{
	m_looped = p_looped;

	if (IsTrackingSound())
		m_trackedSound->GetTrack()->setIsLooped(p_looped);
}

void Audio::Entities::AudioSource::SetPitch(float p_pitch)
{
	m_pitch = p_pitch;

	if (IsTrackingSound())
		m_trackedSound->GetTrack()->setPlaybackSpeed(p_pitch < 0.01f ? 0.01f : p_pitch);
}

bool Audio::Entities::AudioSource::IsTrackingSound() const
{
	return m_trackedSound.operator bool();
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

bool Audio::Entities::AudioSource::IsFinished() const
{
	if (IsTrackingSound())
		return m_trackedSound->GetTrack()->isFinished();
	else
		return true;
}

void Audio::Entities::AudioSource::Play(const Resources::Sound& p_sound)
{
	/* Stops and destroy the previous sound (If any) */
	StopAndDestroyTrackedSound();
	
	/* Play the sound and store a tracker to the sound into memory */
	if (m_spatial)
		m_trackedSound = m_audioPlayer.PlaySpatialSound(p_sound, false, m_looped, m_transform->GetWorldPosition(), true);
	else
		m_trackedSound = m_audioPlayer.PlaySound(p_sound, false, m_looped, true);

	/* If the sound tracker is non-null, apply AudioSource settings to the sound (Not every settings because some are already set with AudioPlayer::PlaySound method) */
	if (IsTrackingSound())
	{
		m_trackedSound->GetTrack()->setVolume(m_volume);
		m_trackedSound->GetTrack()->setPan(m_pan);
		m_trackedSound->GetTrack()->setPlaybackSpeed(m_pitch);
		m_trackedSound->GetTrack()->setMinDistance(m_attenuationThreshold);
		m_trackedSound->GetTrack()->setIsPaused(false);
	}
}

void Audio::Entities::AudioSource::Resume()
{
	if (IsTrackingSound())
		m_trackedSound->GetTrack()->setIsPaused(false);
}

void Audio::Entities::AudioSource::Pause()
{
	if (IsTrackingSound())
		m_trackedSound->GetTrack()->setIsPaused(true);
}

void Audio::Entities::AudioSource::Stop()
{
	if (IsTrackingSound())
		m_trackedSound->GetTrack()->stop();
}

void Audio::Entities::AudioSource::StopAndDestroyTrackedSound()
{
	if (IsTrackingSound())
	{
		m_trackedSound->GetTrack()->stop();
		m_trackedSound->GetTrack()->drop();
	}
}
