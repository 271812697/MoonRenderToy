/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Rendering/HAL/None/NoneShaderStage.h>

template<>
Rendering::HAL::NoneShaderStage::TShaderStage(Settings::EShaderType p_type)
{
}

template<>
Rendering::HAL::NoneShaderStage::~TShaderStage()
{
}

template<>
void Rendering::HAL::NoneShaderStage::Upload(const std::string& p_source) const
{
}

template<>
Rendering::Settings::ShaderCompilationResult Rendering::HAL::NoneShaderStage::Compile() const
{
	return {
		.success = true
	};
}

template<>
uint32_t Rendering::HAL::NoneShaderStage::GetID() const
{
	return 0;
}
