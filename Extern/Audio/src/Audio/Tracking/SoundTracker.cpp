
#include "Audio/Tracking/SoundTracker.h"

Audio::Tracking::SoundTracker::SoundTracker(irrklang::ISound* p_track) :
	m_track(p_track),
	m_soundStopEventBinder(StopEvent)
{
	// m_track->setSoundStopEventReceiver(&m_soundStopEventBinder);
}

Audio::Tracking::SoundTracker::~SoundTracker()
{
	// m_track->setSoundStopEventReceiver(nullptr);
}

irrklang::ISound* Audio::Tracking::SoundTracker::GetTrack() const
{
	return m_track;
}
