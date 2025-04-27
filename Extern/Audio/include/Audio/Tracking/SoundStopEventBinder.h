

#pragma once

#include <irrklang/irrKlang.h>
#include <irrklang/ik_ISoundStopEventReceiver.h>

#include "../../tools/Event.h"

namespace Audio::Tracking
{
	/**
	* This class is an interface between irrklang sound stop event and OvTools event
	*/
	class SoundStopEventBinder : public irrklang::ISoundStopEventReceiver
	{
	public:
		/**
		* Constructor
		* @param p_soundFinishedEvent
		*/
		SoundStopEventBinder(Tools::Eventing::Event<>& p_soundFinishedEvent);

		/**
		* Called when the sound is stopped
		* @param sound
		* @param reason
		* @param userData
		*/
		virtual void OnSoundStopped(irrklang::ISound* sound, irrklang::E_STOP_EVENT_CAUSE reason, void* userData) override;

	private:
		Tools::Eventing::Event<>& SoundFinishedEvent;
	};
}