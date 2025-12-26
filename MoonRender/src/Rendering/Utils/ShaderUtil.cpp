

#include "Rendering/Utils/ShaderUtil.h"

namespace Rendering::Utils
{
	std::string GetShaderTypeName(Settings::EShaderType p_type)
	{
		switch (p_type)
		{
		case Rendering::Settings::EShaderType::VERTEX: return "Vertex";
		case Rendering::Settings::EShaderType::FRAGMENT: return "Fragment";
		case Rendering::Settings::EShaderType::GEOMERTY: return "Geomerty";
		}

		return "None";
	}
}
