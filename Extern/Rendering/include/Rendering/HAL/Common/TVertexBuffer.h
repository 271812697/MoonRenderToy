/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TBuffer.h>

namespace Rendering::HAL
{
	/**
	* Represents a vertex buffer, used to store vertex data for the graphics backend to use.
	*/
	template<Settings::EGraphicsBackend Backend, class VertexBufferContext, class BufferContext>
	class TVertexBuffer final : public TBuffer<Backend, BufferContext>
	{
	public:
		/**
		* Creates a vertex buffer.
		*/
		TVertexBuffer();

	private:
		VertexBufferContext m_context;
	};
}
