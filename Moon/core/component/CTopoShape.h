#pragma once
#include <vector>
#include <Core/ECS/Components/AComponent.h>
namespace MOON {
	class TopoShape;
}
namespace Core::ECS { class Actor; }
namespace Core::ECS::Components
{
	class CTopoShape : public AComponent
	{
	public:
		CTopoShape(ECS::Actor& p_owner);
		virtual ~CTopoShape()override;
		std::string GetName() override;
		virtual void OnUpdate(float p_deltaTime) override;
		MOON::TopoShape& GetTopoShape();

		virtual void OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override
			;

		virtual void OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)override;
	private:
		class CTopoShapeInternal;
		CTopoShapeInternal* mInternal = nullptr;
	};
}