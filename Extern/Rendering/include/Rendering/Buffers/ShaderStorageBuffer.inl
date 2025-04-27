

#pragma once

#include "Rendering/Buffers/ShaderStorageBuffer.h"
#include "UniformBuffer.h"

namespace Rendering::Buffers
{
	template<typename T>
	inline void ShaderStorageBuffer::SendBlocks(T* p_data, size_t p_size)
	{
		UpLoadData(p_data,p_size);
	}
}