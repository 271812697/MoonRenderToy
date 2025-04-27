
#pragma once

#include <Maths/FVector3.h>

namespace Physics::Settings
{
	/**
	* Data structure to give to the PhysicsEngine constructor to setup its settings
	*/
	struct PhysicsSettings
	{
		Maths::FVector3 gravity = { 0.0f, -9.81f, 0.f };
	};
}