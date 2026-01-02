#include <tinyxml2.h>
#include <Core/ECS/Actor.h>
#include <Core/ECS/Components/CColorBar.h>
#include <Core/Global/ServiceLocator.h>
#include <Core/ResourceManagement/MaterialManager.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
Core::ECS::Components::ColorBar::ColorBar(ECS::Actor& p_owner) : AComponent(p_owner)
{
}

std::string Core::ECS::Components::ColorBar::GetName()
{
	return "ColorBar";
}

void Core::ECS::Components::ColorBar::OnUpdate(float p_deltaTime)
{
	if (colorChange) {
		colorChange = false;
		auto mat = owner.GetComponent<Core::ECS::Components::CMaterialRenderer>()->GetMaterialAtIndex(0);	
		if (mat) {
			const ::Rendering::Data::MaterialProperty prop = mat->GetProperty("domainColorTex").value();
			::Rendering::HAL::GLTexture* triangleInfoTex = nullptr;
			auto tex = std::get<::Rendering::HAL::TextureHandle*>(prop.value);
			
			if (tex) {
				std::vector<Maths::FVector4> colors = m_colors;
				::Rendering::Settings::TextureDesc desc;
				desc.isTextureBuffer = true;
				desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA32F;
				desc.buffetLen = colors.size() * sizeof(Maths::FVector4);
				desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
					.data = colors.data()
				};
				static_cast<::Rendering::HAL::GLTexture*>(tex)->Allocate(desc);
			}
		}
	}
}

void Core::ECS::Components::ColorBar::SetColors(const std::vector<Maths::FVector4>& colors)
{
	m_defaultColors = colors;
}

void Core::ECS::Components::ColorBar::SetColor(const std::vector<int>& index, const Maths::FVector4& color)
{
	m_colors = m_defaultColors;
	for (int i = 0;i < index.size();i++) {
		int idx = index[i];
		if (idx >= 0 && idx < m_defaultColors.size()) {
			colorChange = true;
			m_colors[idx] = color;
		}
	}
}

void Core::ECS::Components::ColorBar::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
}

void Core::ECS::Components::ColorBar::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
}
