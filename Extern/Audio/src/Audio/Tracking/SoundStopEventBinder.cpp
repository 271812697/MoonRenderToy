

#include "Audio/Tracking/SoundStopEventBinder.h"

Audio::Tracking::SoundStopEventBinder::SoundStopEventBinder(Tools::Eventing::Event<>& p_soundFinishedEvent) : SoundFinishedEvent(p_soundFinishedEvent)
{
}

void Audio::Tracking::SoundStopEventBinder::OnSoundStopped(irrklang::ISound * sound, irrklang::E_STOP_EVENT_CAUSE reason, void * userData)
{
	SoundFinishedEvent.Invoke();
}
