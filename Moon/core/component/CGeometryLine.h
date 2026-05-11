#pragma once
#include <vector>
#include <Core/ECS/Components/AComponent.h>
namespace Part {
	class  Geometry;
}
namespace Core::ECS { class Actor; }
namespace Core::ECS::Components
{
	class CGeometryLine : public AComponent
	{
	public:
		CGeometryLine(ECS::Actor& p_owner);
		virtual ~CGeometryLine()override;
		std::string GetName() override;
		virtual void OnUpdate(float p_deltaTime) override;
		Part::Geometry* GetGeometry();
		void setGeometry(Part::Geometry* geometry);
		void discretizationShape(int plane);
		
		virtual void OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;
		virtual void OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)override;
	private:
		
		void buildLines(int plane);
		class CGeometryLineInternal;
		CGeometryLineInternal* mInternal = nullptr;
	};
}