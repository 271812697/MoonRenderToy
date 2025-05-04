/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <vector>

#include "Rendering/HAL/VertexArray.h"

namespace Rendering::Resources
{
	/**
	* Interface for any mesh
	*/
	class IMesh
	{
	public:
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual uint32_t GetVertexCount() const = 0;
		virtual uint32_t GetIndexCount() const = 0;
	};
}