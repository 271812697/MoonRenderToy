/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Maths/FMatrix4.h>

namespace Core::Rendering
{
	/**
	* Descriptor for drawable entities that adds a model and a user matrix.
	* This descriptor, when added on a drawable, is read by the EngineBufferRenderFeature
	* and its data is uploaded to the GPU before issuing a draw call.
	*/
	struct EngineDrawableDescriptor
	{
		Maths::FMatrix4 modelMatrix;
		Maths::FMatrix4 userMatrix;
	};
}
