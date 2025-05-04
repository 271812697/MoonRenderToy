/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once


#include <Audio/Entities/AudioSource.h>

#include <Maths/FTransform.h>
#include <Maths/FVector3.h>

namespace Audio::Entities
{
	/**
	* Represents the ears of your application.
	* You can have multiple ones but only the last created will be considered by the AudioEngine
	*/
	class AudioListener
	{
	public:
		/**
		* AudioListener constructor
		* @param p_transform
		*/
		AudioListener(Tools::Utils::OptRef<Maths::FTransform> p_transform = std::nullopt);

		/**
		* AudioListener destructor
		*/
		~AudioListener();

		/**
		* Returns the AudioListener FTransform
		*/
		Maths::FTransform& GetTransform();

		/**
		* Enable or disable the audio listener
		* @param p_enable
		*/
		void SetEnabled(bool p_enable);

		/**
		* Returns true if the audio listener is enabled
		*/
		bool IsEnabled() const;

	private:
		Tools::Utils::ReferenceOrValue<Maths::FTransform> m_transform;
		bool m_enabled = true;

	public:
		static Tools::Eventing::Event<AudioListener&>	CreatedEvent;
		static Tools::Eventing::Event<AudioListener&>	DestroyedEvent;
	};
}