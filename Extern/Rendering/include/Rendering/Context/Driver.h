
#pragma once

#include <stdint.h>


#include "Rendering/Settings/DriverSettings.h"

namespace Rendering::Context
{
	/**
	* The Driver represents the OpenGL context
	*/
	class Driver
	{
	public:
		/**
		* Creates the Driver (OpenGL context)
		* @param p_driverSettings
		*/
		Driver(const Settings::DriverSettings& p_driverSettings);

		/**
		* Destroy the driver
		*/
		~Driver() = default;

		/**
		* Returns true if the OpenGL context is active
		*/
		bool IsActive() const;

	private:
		void InitGlad();
		static void __stdcall GLDebugMessageCallback(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int32_t length, const char* message, const void* userParam);

	private:
		bool m_isActive;
	};
}