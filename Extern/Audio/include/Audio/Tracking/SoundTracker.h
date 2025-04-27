

#pragma once

#include <irrklang/irrKlang.h>


#include "Audio/Tracking/SoundStopEventBinder.h"

namespace Audio::Tracking
{
	/**
	* Track a playing sound and allow the modification of its settings
	*/
	class SoundTracker
	{
	public:
		/**
		* Constructor
		* @param p_track
		*/
		SoundTracker(irrklang::ISound* p_track);

		/**
		* Destructor
		*/
		~SoundTracker();

		/**
		* Returns the tracked sound instance
		*/
		irrklang::ISound* GetTrack() const;

	public:
		/**
		* FinishedEvent is called when the track get stopped/finished
		*/
		Tools::Eventing::Event<> StopEvent;

	private:
		irrklang::ISound* const m_track = nullptr;

	private:
		SoundStopEventBinder m_soundStopEventBinder;
	};
}