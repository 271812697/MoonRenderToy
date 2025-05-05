/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <algorithm>

#include <glad/glad.h>

#include <Debug/Logger.h>
#include <Rendering/HAL/OpenGL/GLShaderStage.h>
#include <Rendering/HAL/OpenGL/GLTypes.h>
#include <Rendering/Utils/ShaderUtil.h>

template<>
Rendering::HAL::GLShaderStage::TShaderStage(Settings::EShaderType p_type) : m_context{
	.id = static_cast<uint32_t>(glCreateShader(EnumToValue<GLenum>(p_type))),
	.type = p_type,
}
{
}

template<>
Rendering::HAL::GLShaderStage::~TShaderStage()
{
	glDeleteShader(m_context.id);
}

template<>
void Rendering::HAL::GLShaderStage::Upload(const std::string& p_source) const
{
	const char* source = p_source.c_str();
	glShaderSource(m_context.id, 1, &source, nullptr);
}

template<>
Rendering::Settings::ShaderCompilationResult Rendering::HAL::GLShaderStage::Compile() const
{
	glCompileShader(m_context.id);

	GLint compileStatus;
	glGetShaderiv(m_context.id, GL_COMPILE_STATUS, &compileStatus);

	if (compileStatus == GL_FALSE)
	{
		GLint maxLength;
		glGetShaderiv(m_context.id, GL_INFO_LOG_LENGTH, &maxLength);

		std::string errorLog(maxLength, ' ');
		glGetShaderInfoLog(m_context.id, maxLength, &maxLength, errorLog.data());

		std::string shaderTypeStr = Utils::GetShaderTypeName(m_context.type);
		std::transform(shaderTypeStr.begin(), shaderTypeStr.end(), shaderTypeStr.begin(), std::toupper);
		std::string errorHeader = "[" + shaderTypeStr + " SHADER] \"";

		return {
			.success = false,
			.message = errorLog
		};
	}

	return {
		.success = true
	};
}

template<>
uint32_t Rendering::HAL::GLShaderStage::GetID() const
{
	return m_context.id;
}
