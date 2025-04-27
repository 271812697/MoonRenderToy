
#include <Audio/Core/AudioPlayer.h>

#include "Core/ECS/Components/CAudioSource.h"
#include "Core/ECS/Actor.h"
#include "Core/Global/ServiceLocator.h"
#include "Core/SceneSystem/SceneManager.h"

Core::ECS::Components::CAudioSource::CAudioSource(ECS::Actor& p_owner) :
	AComponent(p_owner),
	m_audioSource(Core::Global::ServiceLocator::Get<Audio::Core::AudioPlayer>(), owner.transform.GetFTransform())
{
}

std::string Core::ECS::Components::CAudioSource::GetName()
{
	return "Audio Source";
}

void Core::ECS::Components::CAudioSource::SetSound(Audio::Resources::Sound* p_sound)
{
	m_sound = p_sound;
}

void Core::ECS::Components::CAudioSource::SetAutoplay(bool p_autoplay)
{
	m_autoPlay = p_autoplay;
}

void Core::ECS::Components::CAudioSource::SetVolume(float p_volume)
{
	m_audioSource.SetVolume(p_volume);
}

void Core::ECS::Components::CAudioSource::SetPan(float p_pan)
{
	m_audioSource.SetPan(p_pan);
}

void Core::ECS::Components::CAudioSource::SetLooped(bool p_looped)
{
	m_audioSource.SetLooped(p_looped);
}

void Core::ECS::Components::CAudioSource::SetPitch(float p_pitch)
{
	m_audioSource.SetPitch(p_pitch);
}

void Core::ECS::Components::CAudioSource::SetSpatial(bool p_value)
{
	m_audioSource.SetSpatial(p_value);
}

void Core::ECS::Components::CAudioSource::SetAttenuationThreshold(float p_distance)
{
	m_audioSource.SetAttenuationThreshold(p_distance);
}

Audio::Resources::Sound* Core::ECS::Components::CAudioSource::GetSound() const
{
	return m_sound;
}

bool Core::ECS::Components::CAudioSource::IsAutoplayed() const
{
	return m_autoPlay;
}

float Core::ECS::Components::CAudioSource::GetVolume() const
{
	return m_audioSource.GetVolume();
}

float Core::ECS::Components::CAudioSource::GetPan() const
{
	return m_audioSource.GetPan();
}

bool Core::ECS::Components::CAudioSource::IsLooped() const
{
	return m_audioSource.IsLooped();
}

float Core::ECS::Components::CAudioSource::GetPitch() const
{
	return m_audioSource.GetPitch();
}

bool Core::ECS::Components::CAudioSource::IsFinished() const
{
	return m_audioSource.IsFinished();
}

bool Core::ECS::Components::CAudioSource::IsSpatial() const
{
	return m_audioSource.IsSpatial();
}

float Core::ECS::Components::CAudioSource::GetAttenuationThreshold() const
{
	return m_audioSource.GetAttenuationThreshold();
}

void Core::ECS::Components::CAudioSource::Play()
{
	if (owner.IsActive() && m_sound)
		m_audioSource.Play(*m_sound);
}

void Core::ECS::Components::CAudioSource::Pause()
{
	if (owner.IsActive())
		m_audioSource.Pause();
}

void Core::ECS::Components::CAudioSource::Resume()
{
	if (owner.IsActive())
		m_audioSource.Resume();
}

void Core::ECS::Components::CAudioSource::Stop()
{
	if (owner.IsActive())
		m_audioSource.Stop();
}

void Core::ECS::Components::CAudioSource::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	using namespace Core::Helpers;

	Serializer::SerializeBoolean(p_doc, p_node, "autoplay", m_autoPlay);
	Serializer::SerializeBoolean(p_doc, p_node, "spatial", IsSpatial());
	Serializer::SerializeFloat(p_doc, p_node, "volume", GetVolume());
	Serializer::SerializeFloat(p_doc, p_node, "pan", GetPan());
	Serializer::SerializeBoolean(p_doc, p_node, "looped", IsLooped());
	Serializer::SerializeFloat(p_doc, p_node, "pitch", GetPitch());
	Serializer::SerializeFloat(p_doc, p_node, "attenuation_threshold", GetAttenuationThreshold());
	Serializer::SerializeSound(p_doc, p_node, "audio_clip", m_sound);
}

void Core::ECS::Components::CAudioSource::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	using namespace Core::Helpers;

	Serializer::DeserializeBoolean(p_doc, p_node, "autoplay", m_autoPlay);
	SetSpatial(Serializer::DeserializeBoolean(p_doc, p_node, "spatial"));
	SetVolume(Serializer::DeserializeFloat(p_doc, p_node, "volume"));
	SetPan(Serializer::DeserializeFloat(p_doc, p_node, "pan"));
	SetLooped(Serializer::DeserializeBoolean(p_doc, p_node, "looped"));
	SetPitch(Serializer::DeserializeFloat(p_doc, p_node, "pitch"));
	SetAttenuationThreshold(Serializer::DeserializeFloat(p_doc, p_node, "attenuation_threshold"));
	Serializer::DeserializeSound(p_doc, p_node, "audio_clip", m_sound);
}



void Core::ECS::Components::CAudioSource::OnEnable()
{
	if (m_autoPlay)
		Play();
}

void Core::ECS::Components::CAudioSource::OnDisable()
{
	m_audioSource.Stop();
}
