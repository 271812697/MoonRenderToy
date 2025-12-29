#pragma once
#include <vector>
#include <Core/ECS/Components/AComponent.h>
namespace Core::ECS { class Actor; }
namespace Core::ECS::Components
{
	class ColorBar : public AComponent
	{
	public:
		ColorBar(ECS::Actor& p_owner);
		std::string GetName() override;
		virtual void OnUpdate(float p_deltaTime) override;
		void SetColors(const std::vector<Maths::FVector4>& colors);
		void SetColor(int index, const Maths::FVector4& color);
		virtual void OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;
		virtual void OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;
	private:
		bool colorChange = false;
		std::vector<Maths::FVector4> m_colors;
		std::vector<Maths::FVector4> m_defaultColors;
	};
}