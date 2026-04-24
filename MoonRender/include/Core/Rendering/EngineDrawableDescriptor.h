#pragma once
#include <Maths/FMatrix4.h>

namespace Core::Rendering
{
	struct EngineDrawableDescriptor
	{
		Maths::FMatrix4 modelMatrix;
		Maths::FMatrix4 userMatrix;
	};
}
