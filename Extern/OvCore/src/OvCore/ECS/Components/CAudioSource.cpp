/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#include <OvAudio/Core/AudioEngine.h>

#include <OvCore/ECS/Components/CAudioSource.h>
#include <OvCore/ECS/Actor.h>
#include <OvCore/Global/ServiceLocator.h>
#include <OvCore/SceneSystem/SceneManager.h>

OvCore::ECS::Components::CAudioSource::CAudioSource(ECS::Actor& p_owner) :
	AComponent(p_owner),
	m_audioSource(OvCore::Global::ServiceLocator::Get<OvAudio::Core::AudioEngine>(), owner.transform.GetFTransform())
{
}

std::string OvCore::ECS::Components::CAudioSource::GetName()
{
	return "Audio Source";
}

void OvCore::ECS::Components::CAudioSource::SetSound(OvAudio::Resources::Sound* p_sound)
{
	m_sound = p_sound;
}

void OvCore::ECS::Components::CAudioSource::SetAutoplay(bool p_autoplay)
{
	m_autoPlay = p_autoplay;
}

void OvCore::ECS::Components::CAudioSource::SetVolume(float p_volume)
{
	m_audioSource.SetVolume(p_volume);
}

void OvCore::ECS::Components::CAudioSource::SetPan(float p_pan)
{
	m_audioSource.SetPan(p_pan);
}

void OvCore::ECS::Components::CAudioSource::SetLooped(bool p_looped)
{
	m_audioSource.SetLooped(p_looped);
}

void OvCore::ECS::Components::CAudioSource::SetPitch(float p_pitch)
{
	m_audioSource.SetPitch(p_pitch);
}

void OvCore::ECS::Components::CAudioSource::SetSpatial(bool p_value)
{
	m_audioSource.SetSpatial(p_value);
}

void OvCore::ECS::Components::CAudioSource::SetAttenuationThreshold(float p_distance)
{
	m_audioSource.SetAttenuationThreshold(p_distance);
}

OvAudio::Resources::Sound* OvCore::ECS::Components::CAudioSource::GetSound() const
{
	return m_sound;
}

bool OvCore::ECS::Components::CAudioSource::IsAutoplayed() const
{
	return m_autoPlay;
}

float OvCore::ECS::Components::CAudioSource::GetVolume() const
{
	return m_audioSource.GetVolume();
}

float OvCore::ECS::Components::CAudioSource::GetPan() const
{
	return m_audioSource.GetPan();
}

bool OvCore::ECS::Components::CAudioSource::IsLooped() const
{
	return m_audioSource.IsLooped();
}

float OvCore::ECS::Components::CAudioSource::GetPitch() const
{
	return m_audioSource.GetPitch();
}

bool OvCore::ECS::Components::CAudioSource::IsPlaying() const
{
	return m_audioSource.IsPlaying();
}

bool OvCore::ECS::Components::CAudioSource::IsSpatial() const
{
	return m_audioSource.IsSpatial();
}

float OvCore::ECS::Components::CAudioSource::GetAttenuationThreshold() const
{
	return m_audioSource.GetAttenuationThreshold();
}

void OvCore::ECS::Components::CAudioSource::Play()
{
	if (owner.IsActive() && m_sound)
		m_audioSource.Play(*m_sound);
}

void OvCore::ECS::Components::CAudioSource::Pause()
{
	if (owner.IsActive())
		m_audioSource.Pause();
}

void OvCore::ECS::Components::CAudioSource::Resume()
{
	if (owner.IsActive())
		m_audioSource.Resume();
}

void OvCore::ECS::Components::CAudioSource::Stop()
{
	if (owner.IsActive())
		m_audioSource.Stop();
}

void OvCore::ECS::Components::CAudioSource::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	using namespace OvCore::Helpers;

	Serializer::SerializeBoolean(p_doc, p_node, "autoplay", m_autoPlay);
	Serializer::SerializeBoolean(p_doc, p_node, "spatial", IsSpatial());
	Serializer::SerializeFloat(p_doc, p_node, "volume", GetVolume());
	Serializer::SerializeFloat(p_doc, p_node, "pan", GetPan());
	Serializer::SerializeBoolean(p_doc, p_node, "looped", IsLooped());
	Serializer::SerializeFloat(p_doc, p_node, "pitch", GetPitch());
	Serializer::SerializeFloat(p_doc, p_node, "attenuation_threshold", GetAttenuationThreshold());
	Serializer::SerializeSound(p_doc, p_node, "audio_clip", m_sound);
}

void OvCore::ECS::Components::CAudioSource::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	using namespace OvCore::Helpers;

	Serializer::DeserializeBoolean(p_doc, p_node, "autoplay", m_autoPlay);
	SetSpatial(Serializer::DeserializeBoolean(p_doc, p_node, "spatial"));
	SetVolume(Serializer::DeserializeFloat(p_doc, p_node, "volume"));
	SetPan(Serializer::DeserializeFloat(p_doc, p_node, "pan"));
	SetLooped(Serializer::DeserializeBoolean(p_doc, p_node, "looped"));
	SetPitch(Serializer::DeserializeFloat(p_doc, p_node, "pitch"));
	SetAttenuationThreshold(Serializer::DeserializeFloat(p_doc, p_node, "attenuation_threshold"));
	Serializer::DeserializeSound(p_doc, p_node, "audio_clip", m_sound);
}



void OvCore::ECS::Components::CAudioSource::OnEnable()
{
	if (m_autoPlay)
		Play();
}

void OvCore::ECS::Components::CAudioSource::OnDisable()
{
	m_audioSource.Stop();
}
