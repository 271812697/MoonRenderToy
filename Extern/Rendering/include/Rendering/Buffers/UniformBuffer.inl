#pragma once
#include "Rendering/Buffers/UniformBuffer.h"

namespace Rendering::Buffers
{
	template<typename T>
	inline void UniformBuffer::SetSubData(const T& p_data, size_t p_offsetInOut)
	{
		Bind();
		UploadData(p_offsetInOut, sizeof(T), std::addressof(p_data));
		Unbind();
	}

	template<typename T>
	inline void UniformBuffer::SetSubData(const T& p_data, std::reference_wrapper<size_t> p_offsetInOut)
	{
		Bind();
		size_t dataSize = sizeof(T);
		UploadData(p_offsetInOut.get(), dataSize, std::addressof(p_data));
		
		p_offsetInOut.get() += dataSize;
		Unbind();
	}
}