#pragma once
#include <vector>
#include <Core/ECS/Components/AComponent.h>
namespace Part {
	class TopoShape;
}
namespace Core::ECS { class Actor; }
namespace Core::ECS::Components
{
	struct HighLightOption
	{
		enum Mode
		{
			Color=0,
			Transparent=1
		};
		Mode mode{ Transparent };
		Maths::FVector4 hoverColor = { 1,1,0,1 };
		Maths::FVector4 selectColor = {1,1,1.0,1};
	};
	class CTopoShape : public AComponent
	{
	public:
		CTopoShape(ECS::Actor& p_owner);
		virtual ~CTopoShape()override;
		std::string GetName() override;
		HighLightOption& getHightLightOption();
		void switchHighLightMode(HighLightOption::Mode mode);
		virtual void OnUpdate(float p_deltaTime) override;
		void updateChildBuffer();
		std::vector<std::pair<int, int>>GetChildMeshInfo();
		void setChildsMeshTransParent(const std::vector<int>& childs);
		Part::TopoShape& GetTopoShape();
		Part::TopoShape GetTopoFace(int childFaceId);
		void hoverChild(int childId);
		void hoverChildLine(int childId);

		void clearHover();
		void clearHoverLine();
		void discretizationFaceShape();
		void discretizationEdgeShape();
		void discretizationShape();
		virtual void OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;
		virtual void OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)override;
	private:
		void updateChildMesh();
		class CTopoShapeInternal;
		CTopoShapeInternal* mInternal = nullptr;
	};
}