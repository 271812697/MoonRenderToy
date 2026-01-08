#pragma once
#include <vector>
#include <Core/ECS/Components/AComponent.h>
#include <Core/SceneSystem/Intersection.h>
#include <Rendering/Geometry/bbox.h>
namespace Core::ECS { class Actor; }
namespace Core::ECS::Components
{
	class CBatchMeshLine : public AComponent
	{
	public:
		CBatchMeshLine(ECS::Actor& p_owner);
		virtual ~CBatchMeshLine()override;
		std::string GetName() override;
		virtual void OnUpdate(float p_deltaTime) override;
		void SetColors(const std::vector<Maths::FVector4>& colors);
		void SetColor(const std::vector<int>& index, const Maths::FVector4& color);
		void SetHoverColor(int index, const Maths::FVector4& color);
		std::vector<Maths::FVector3> getLineSeg(int index);
		void BuildBvh(const  std::vector<uint32_t>&subMeshRanges);
		bool PointPick(const Maths::FMatrix4& viewPortMatrix, int x, int y, float tolerance, Core::SceneSystem::PointPickRes& out);
		virtual void OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;
		virtual void OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;
	private:
		class CBatchMeshLineInternal;
		CBatchMeshLineInternal* mInternal = nullptr;
	};
}