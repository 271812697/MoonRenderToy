/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Rendering/HAL/None/NoneShaderProgram.h>
#include <Rendering/HAL/None/NoneTexture.h>

template<>
Rendering::HAL::NoneShaderProgram::TShaderProgram() 
{
}

template<>
Rendering::HAL::NoneShaderProgram::~TShaderProgram()
{
}

template<>
void Rendering::HAL::NoneShaderProgram::Bind() const
{
}

template<>
void Rendering::HAL::NoneShaderProgram::Unbind() const
{
}

template<>
uint32_t Rendering::HAL::NoneShaderProgram::GetID() const
{
	return 0;
}

template<>
void Rendering::HAL::NoneShaderProgram::Attach(const NoneShaderStage& p_shader)
{
}

template<>
void Rendering::HAL::NoneShaderProgram::Detach(const NoneShaderStage& p_shader)
{
	
}

template<>
void Rendering::HAL::NoneShaderProgram::DetachAll()
{
}

template<>
Rendering::Settings::ShaderLinkingResult Rendering::HAL::NoneShaderProgram::Link()
{
	return {
		.success = true
	};
}

#define DECLARE_SET_UNIFORM_FUNCTION(type) \
template<> \
template<> \
void Rendering::HAL::NoneShaderProgram::SetUniform<type>(std::string_view p_name, const type& p_value) \
{ \
}

#define DECLARE_GET_UNIFORM_FUNCTION(type) \
template<> \
template<> \
type Rendering::HAL::NoneShaderProgram::GetUniform<type>(std::string_view p_name) \
{ \
	return type{}; \
}

DECLARE_SET_UNIFORM_FUNCTION(int);
DECLARE_SET_UNIFORM_FUNCTION(float);
DECLARE_SET_UNIFORM_FUNCTION(Maths::FVector2);
DECLARE_SET_UNIFORM_FUNCTION(Maths::FVector3);
DECLARE_SET_UNIFORM_FUNCTION(Maths::FVector4);
DECLARE_SET_UNIFORM_FUNCTION(Maths::FMatrix4);

DECLARE_GET_UNIFORM_FUNCTION(int);
DECLARE_GET_UNIFORM_FUNCTION(float);
DECLARE_GET_UNIFORM_FUNCTION(Maths::FVector2);
DECLARE_GET_UNIFORM_FUNCTION(Maths::FVector3);
DECLARE_GET_UNIFORM_FUNCTION(Maths::FVector4);
DECLARE_GET_UNIFORM_FUNCTION(Maths::FMatrix4);

template<>
void Rendering::HAL::NoneShaderProgram::QueryUniforms()
{
}

template<>
Tools::Utils::OptRef<const Rendering::Settings::UniformInfo> Rendering::HAL::NoneShaderProgram::GetUniformInfo(std::string_view p_name) const
{
	return std::nullopt;
}

template<>
std::span<const Rendering::Settings::UniformInfo> Rendering::HAL::NoneShaderProgram::GetUniforms() const
{
	return {};
}