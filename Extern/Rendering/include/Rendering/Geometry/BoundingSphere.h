/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Maths/FVector3.h>

namespace Rendering::Geometry
{
	/**
	* Data structure that defines a bounding sphere (Position + radius)
	*/
	struct BoundingSphere
	{
		Maths::FVector3 position;
		float radius;
	};
}