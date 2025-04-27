

#include "Core/Resources/Material.h"
#include "Core/Global/ServiceLocator.h"
#include "Core/ResourceManagement/TextureManager.h"

#include <Rendering/Buffers/UniformBuffer.h>
#include <Rendering/Resources/Texture.h>
#include <glad/glad.h>

void Core::Resources::Material::SetShader(Rendering::Resources::Shader* p_shader)
{
	m_shader = p_shader;
	if (m_shader)
	{
		Rendering::Buffers::UniformBuffer::BindBlockToShader(*m_shader, "EngineUBO");
		FillUniform();
	}
	else
	{
		m_uniformsData.clear();
	}
}

void Core::Resources::Material::FillUniform()
{
	m_uniformsData.clear();

	for (const Rendering::Resources::UniformInfo& element : m_shader->uniforms)
		m_uniformsData.emplace(element.name, element.defaultValue);
}

void Core::Resources::Material::Bind(Rendering::Resources::Texture* p_emptyTexture)
{
	if (HasShader())
	{
		using namespace Maths;
		using namespace Rendering::Resources;

		m_shader->Bind();

		int textureSlot = 10;

		for (auto&[name, value] : m_uniformsData)
		{
			auto uniformData = m_shader->GetUniformInfo(name);

			if (uniformData)
			{
                switch (uniformData->type)
                {
                case Rendering::Resources::UniformType::UNIFORM_BOOL:			if (value.type() == typeid(bool))		  m_shader->SetUniformInt(name, std::any_cast<bool>(value));			break;
                case Rendering::Resources::UniformType::UNIFORM_INT:			if (value.type() == typeid(int))		  m_shader->SetUniformInt(name, std::any_cast<int>(value));			    break;
                case Rendering::Resources::UniformType::UNIFORM_UINT:			if (value.type() == typeid(unsigned int)) m_shader->SetUniformUint(name, std::any_cast<unsigned int>(value));   break;
                case Rendering::Resources::UniformType::UNIFORM_FLOAT:		    if (value.type() == typeid(float))		  m_shader->SetUniformFloat(name, std::any_cast<float>(value));		    break;
                case Rendering::Resources::UniformType::UNIFORM_FLOAT_VEC2:	    if (value.type() == typeid(FVector2))	  m_shader->SetUniformVec2(name, std::any_cast<FVector2>(value));		break;
                case Rendering::Resources::UniformType::UNIFORM_FLOAT_VEC3:	    if (value.type() == typeid(FVector3))	  m_shader->SetUniformVec3(name, std::any_cast<FVector3>(value));		break;
                case Rendering::Resources::UniformType::UNIFORM_FLOAT_VEC4:	    if (value.type() == typeid(FVector4))	  m_shader->SetUniformVec4(name, std::any_cast<FVector4>(value));		break;
                case Rendering::Resources::UniformType::UNIFORM_FLOAT_MAT4:	    if (value.type() == typeid(FMatrix4))	  m_shader->SetUniformMat4(name, std::any_cast<FMatrix4>(value));		break;
                case Rendering::Resources::UniformType::UNIFORM_SAMPLER_2D:
                {
                    if (value.type() == typeid(Texture*))
                    {
                        if (auto tex = std::any_cast<Texture*>(value); tex)
                        {
                            glBindTextureUnit(textureSlot, tex->id);
                            m_shader->SetUniformInt(uniformData->name, textureSlot++);
                        }
                        else if (p_emptyTexture)
                        {
                            glBindTextureUnit(textureSlot, p_emptyTexture->id);
                            m_shader->SetUniformInt(uniformData->name, textureSlot++);
                        }
                    }
                }; break;
                
                }
   
			}
		}
		
	}
}

void Core::Resources::Material::UnBind()
{
	if (HasShader())
		m_shader->Unbind();
}

Rendering::Resources::Shader*& Core::Resources::Material::GetShader()
{
	return m_shader;
}

bool Core::Resources::Material::HasShader() const
{
	return m_shader;
}

void Core::Resources::Material::SetBlendable(bool p_transparent)
{
	m_blendable = p_transparent;
}

void Core::Resources::Material::SetBackfaceCulling(bool p_backfaceCulling)
{
	m_backfaceCulling = p_backfaceCulling;
}

void Core::Resources::Material::SetFrontfaceCulling(bool p_frontfaceCulling)
{
	m_frontfaceCulling = p_frontfaceCulling;
}

void Core::Resources::Material::SetDepthTest(bool p_depthTest)
{
	m_depthTest = p_depthTest;
}

void Core::Resources::Material::SetDepthWriting(bool p_depthWriting)
{
	m_depthWriting = p_depthWriting;
}

void Core::Resources::Material::SetColorWriting(bool p_colorWriting)
{
	m_colorWriting = p_colorWriting;
}

void Core::Resources::Material::SetGPUInstances(int p_instances)
{
	m_gpuInstances = p_instances;
}

bool Core::Resources::Material::IsBlendable() const
{
	return m_blendable;
}

bool Core::Resources::Material::HasBackfaceCulling() const
{
	return m_backfaceCulling;
}

bool Core::Resources::Material::HasFrontfaceCulling() const
{
	return m_frontfaceCulling;
}

bool Core::Resources::Material::HasDepthTest() const
{
	return m_depthTest;
}

bool Core::Resources::Material::HasDepthWriting() const
{
	return m_depthWriting;
}

bool Core::Resources::Material::HasColorWriting() const
{
	return m_colorWriting;
}

int Core::Resources::Material::GetGPUInstances() const
{
	return m_gpuInstances;
}

uint8_t Core::Resources::Material::GenerateStateMask() const
{
	uint8_t result = 0;

	if (m_depthWriting)								result |= 0b0000'0001;
	if (m_colorWriting)								result |= 0b0000'0010;
	if (m_blendable)								result |= 0b0000'0100;
	if (m_backfaceCulling || m_frontfaceCulling)	result |= 0b0000'1000;
	if (m_depthTest)								result |= 0b0001'0000;
	if (m_backfaceCulling)							result |= 0b0010'0000;
	if (m_frontfaceCulling)							result |= 0b0100'0000;

	return result;
}

std::map<std::string, std::any>& Core::Resources::Material::GetUniformsData()
{
	return m_uniformsData;
}

void Core::Resources::Material::OnSerialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_node)
{
	using namespace Core::Helpers;
	using namespace Rendering::Resources;
	using namespace Maths;

	Serializer::SerializeShader(p_doc, p_node, "shader", m_shader);

	tinyxml2::XMLNode* settingsNode = p_doc.NewElement("settings");
	p_node->InsertEndChild(settingsNode);

	Serializer::SerializeBoolean(p_doc, settingsNode, "blendable", m_blendable);
	Serializer::SerializeBoolean(p_doc, settingsNode, "backface_culling", m_backfaceCulling);
	Serializer::SerializeBoolean(p_doc, settingsNode, "frontface_culling", m_frontfaceCulling);
	Serializer::SerializeBoolean(p_doc, settingsNode, "depth_test", m_depthTest);
	Serializer::SerializeBoolean(p_doc, settingsNode, "depth_writing", m_depthWriting);
	Serializer::SerializeBoolean(p_doc, settingsNode, "color_writing", m_colorWriting);
	Serializer::SerializeInt(p_doc, settingsNode, "gpu_instances", m_gpuInstances);

	// Create "Uniforms" (Every uniform will be attached to "Uniforms")
	tinyxml2::XMLNode* uniformsNode = p_doc.NewElement("uniforms");
	p_node->InsertEndChild(uniformsNode);

	for (const auto&[uniformName, uniformValue] : m_uniformsData)
	{
		tinyxml2::XMLNode* uniform = p_doc.NewElement("uniform");
		uniformsNode->InsertEndChild(uniform); // Instead of p_node, use uniformNode (To create)

		const Rendering::Resources::UniformInfo* uniformInfo = m_shader->GetUniformInfo(uniformName);

		Serializer::SerializeString(p_doc, uniform, "name", uniformName);

		if (uniformInfo && uniformValue.has_value())
		{
			switch (uniformInfo->type)
			{
			case UniformType::UNIFORM_BOOL:
				if (uniformValue.type() == typeid(bool)) Serializer::SerializeInt(p_doc, uniform, "value", std::any_cast<bool>(uniformValue));
				break;

			case UniformType::UNIFORM_INT:
				if (uniformValue.type() == typeid(int)) Serializer::SerializeInt(p_doc, uniform, "value", std::any_cast<int>(uniformValue));
				break;

            case UniformType::UNIFORM_UINT:
                if (uniformValue.type() == typeid(unsigned int)) Serializer::SerializeUint(p_doc, uniform, "value", std::any_cast<unsigned int>(uniformValue));
                break;

			case UniformType::UNIFORM_FLOAT:
				if (uniformValue.type() == typeid(float)) Serializer::SerializeFloat(p_doc, uniform, "value", std::any_cast<float>(uniformValue));
				break;

			case UniformType::UNIFORM_FLOAT_VEC2:
				if (uniformValue.type() == typeid(FVector2)) Serializer::SerializeVec2(p_doc, uniform, "value", std::any_cast<FVector2>(uniformValue));
				break;

			case UniformType::UNIFORM_FLOAT_VEC3:
				if (uniformValue.type() == typeid(FVector3)) Serializer::SerializeVec3(p_doc, uniform, "value", std::any_cast<FVector3>(uniformValue));
				break;

			case UniformType::UNIFORM_FLOAT_VEC4:
				if (uniformValue.type() == typeid(FVector4)) Serializer::SerializeVec4(p_doc, uniform, "value", std::any_cast<FVector4>(uniformValue));
				break;
            case UniformType::UNIFORM_FLOAT_MAT4:
                if (uniformValue.type() == typeid(FMatrix4)) Serializer::SerializeFmatrix4(p_doc, uniform, "value", std::any_cast<FMatrix4>(uniformValue));
                break;

			case UniformType::UNIFORM_SAMPLER_2D:
				if (uniformValue.type() == typeid(Texture*)) Serializer::SerializeTexture(p_doc, uniform, "value", std::any_cast<Texture*>(uniformValue));
				break;
			}
		}
	}
}

void Core::Resources::Material::OnDeserialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_node)
{
	using namespace Core::Helpers;

	tinyxml2::XMLNode* settingsNode = p_node->FirstChildElement("settings");
	
	if (settingsNode)
	{
		Serializer::DeserializeBoolean(p_doc, settingsNode, "blendable", m_blendable);
		Serializer::DeserializeBoolean(p_doc, settingsNode, "backface_culling", m_backfaceCulling);
		Serializer::DeserializeBoolean(p_doc, settingsNode, "frontface_culling", m_frontfaceCulling);
		Serializer::DeserializeBoolean(p_doc, settingsNode, "depth_test", m_depthTest);
		Serializer::DeserializeBoolean(p_doc, settingsNode, "depth_writing", m_depthWriting);
		Serializer::DeserializeBoolean(p_doc, settingsNode, "color_writing", m_colorWriting);
		Serializer::DeserializeInt(p_doc, settingsNode, "gpu_instances", m_gpuInstances);
	}

	/* We get the shader with Deserialize method */
	Rendering::Resources::Shader* deserializedShader = Core::Helpers::Serializer::DeserializeShader(p_doc, p_node, "shader");

	/* We verify that the shader is valid (Not null) */
	if (deserializedShader)
	{
		/* If the shader is valid, we set it to the material (Modify m_shader pointer + Query + FillUniforms) */
		SetShader(deserializedShader);

		tinyxml2::XMLNode* uniformsNode = p_node->FirstChildElement("uniforms"); // Access to "Uniforms" (Every uniform will be attached to "Uniforms")		

		if (uniformsNode)
		{
			/* We iterate over every <uniform>...</uniform> */
			for (auto uniform = uniformsNode->FirstChildElement("uniform"); uniform; uniform = uniform->NextSiblingElement("uniform"))
			{
				/* Verify that the uniform node contains a "name" element */
				if (auto uniformNameElement = uniform->FirstChildElement("name"); uniformNameElement)
				{
					const std::string uniformName = uniformNameElement->GetText();

					/* We collect information about the uniform (The uniform is identified in the shader by its name) */
					const Rendering::Resources::UniformInfo* uniformInfo = m_shader->GetUniformInfo(uniformName);

					/* We verify that the uniform is existant is the current shader */
					if (uniformInfo)
					{
						/* Deserialization of the uniform value depending on the uniform type (Deserialization result to std::any) */
						switch (uniformInfo->type)
						{
						case Rendering::Resources::UniformType::UNIFORM_BOOL:
							m_uniformsData[uniformInfo->name] = Core::Helpers::Serializer::DeserializeBoolean(p_doc, uniform, "value");
							break;

						case Rendering::Resources::UniformType::UNIFORM_INT:
							m_uniformsData[uniformInfo->name] = Core::Helpers::Serializer::DeserializeInt(p_doc, uniform, "value");
							break;

                        case Rendering::Resources::UniformType::UNIFORM_UINT:
                            m_uniformsData[uniformInfo->name] = Core::Helpers::Serializer::DeserializeUint(p_doc, uniform, "value");
                            break;

						case Rendering::Resources::UniformType::UNIFORM_FLOAT:
							m_uniformsData[uniformInfo->name] = Core::Helpers::Serializer::DeserializeFloat(p_doc, uniform, "value");
							break;

						case Rendering::Resources::UniformType::UNIFORM_FLOAT_VEC2:
							m_uniformsData[uniformInfo->name] = Core::Helpers::Serializer::DeserializeVec2(p_doc, uniform, "value");
							break;

						case Rendering::Resources::UniformType::UNIFORM_FLOAT_VEC3:
							m_uniformsData[uniformInfo->name] = Core::Helpers::Serializer::DeserializeVec3(p_doc, uniform, "value");
							break;

						case Rendering::Resources::UniformType::UNIFORM_FLOAT_VEC4:
							m_uniformsData[uniformInfo->name] = Core::Helpers::Serializer::DeserializeVec4(p_doc, uniform, "value");
							break;
                        case Rendering::Resources::UniformType::UNIFORM_FLOAT_MAT4:
                            m_uniformsData[uniformInfo->name] = Core::Helpers::Serializer::DeserializeFmatrix4(p_doc, uniform, "value");
                            break;

						case Rendering::Resources::UniformType::UNIFORM_SAMPLER_2D:
							m_uniformsData[uniformInfo->name] = Core::Helpers::Serializer::DeserializeTexture(p_doc, uniform, "value");
							break;
						}
					}
				}
			}
		}
	}
}