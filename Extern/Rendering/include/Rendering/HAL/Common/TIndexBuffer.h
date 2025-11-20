#pragma once
#include <Rendering/HAL/Common/TBuffer.h>

namespace Rendering::HAL
{
	/**
	* Represents an index buffer, used to store index data for the graphics backend to use.
	*/
	template<Settings::EGraphicsBackend Backend, class IndexBufferContext, class BufferContext>
	class TIndexBuffer : public TBuffer<Backend, BufferContext>
	{
	public:
		/**
		* Creates an index buffer.
		*/
		TIndexBuffer();

	private:
		IndexBufferContext m_context;
	};
}
