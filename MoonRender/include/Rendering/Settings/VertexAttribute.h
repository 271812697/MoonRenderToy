#pragma once
#include <span>
#include <Rendering/Settings/EDataType.h>

namespace Rendering::Settings
{
	/**
	* Structure that holds information about a uniform
	*/
	struct VertexAttribute
	{
		EDataType type = EDataType::FLOAT;
		uint8_t count = 4;
		bool normalized = false;
	};

	using VertexAttributeLayout = std::span<const Settings::VertexAttribute>;
}
