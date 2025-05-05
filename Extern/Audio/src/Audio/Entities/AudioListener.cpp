/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Audio/Entities/AudioListener.h>

Tools::Eventing::Event<Audio::Entities::AudioListener&>	Audio::Entities::AudioListener::CreatedEvent;
Tools::Eventing::Event<Audio::Entities::AudioListener&>	Audio::Entities::AudioListener::DestroyedEvent;

Audio::Entities::AudioListener::AudioListener(Tools::Utils::OptRef<Maths::FTransform> p_transform) :
	m_transform(p_transform)
{
	CreatedEvent.Invoke(*this);
}

Audio::Entities::AudioListener::~AudioListener()
{
	DestroyedEvent.Invoke(*this);
}

Maths::FTransform& Audio::Entities::AudioListener::GetTransform()
{
	return m_transform;
}

void Audio::Entities::AudioListener::SetEnabled(bool p_enable)
{
	m_enabled = p_enable;
}

bool Audio::Entities::AudioListener::IsEnabled() const
{
	return m_enabled;
}
