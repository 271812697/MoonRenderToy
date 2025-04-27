

#include "Audio/Entities/AudioListener.h"

Tools::Eventing::Event<Audio::Entities::AudioListener&>	Audio::Entities::AudioListener::CreatedEvent;
Tools::Eventing::Event<Audio::Entities::AudioListener&>	Audio::Entities::AudioListener::DestroyedEvent;

Audio::Entities::AudioListener::AudioListener() :
	m_transform(new Maths::FTransform()),
	m_internalTransform(true)
{
	Setup();
}

Audio::Entities::AudioListener::AudioListener(Maths::FTransform& p_transform) :
	m_transform(&p_transform),
	m_internalTransform(false)
{
	Setup();
}

Audio::Entities::AudioListener::~AudioListener()
{
	DestroyedEvent.Invoke(*this);
}

Maths::FTransform& Audio::Entities::AudioListener::GetTransform()
{
	return *m_transform;
}

void Audio::Entities::AudioListener::SetEnabled(bool p_enable)
{
	m_enabled = p_enable;
}

bool Audio::Entities::AudioListener::IsEnabled() const
{
	return m_enabled;
}

void Audio::Entities::AudioListener::Setup()
{
	CreatedEvent.Invoke(*this);
}
