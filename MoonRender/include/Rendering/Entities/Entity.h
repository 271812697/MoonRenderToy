#pragma once
#include <memory>
#include <Maths/FTransform.h>
#include <Tools/Utils/ReferenceOrValue.h>

namespace Rendering::Entities
{
	/**
	* Represents an entity with a transformation in space
	*/
	struct Entity
	{
		Tools::Utils::ReferenceOrValue<Maths::FTransform> transform;
	};
}
