#pragma once
#include <vector>
#include <Core/ECS/Components/AComponent.h>
#include <Core/SceneSystem/Intersection.h>
#include <Rendering/Geometry/bbox.h>
namespace Core::ECS { class Actor; }
namespace Core::ECS::Components
{
	class CBatchMesh : public AComponent
	{
	public:
		CBatchMesh(ECS::Actor& p_owner);
		virtual ~CBatchMesh()override;
		std::string GetName() override;
		virtual void OnUpdate(float p_deltaTime) override;
		void SetColors(const std::vector<Maths::FVector4>& colors);
		void SetColor(const std::vector<int>& index, const Maths::FVector4& color);
		void SetHoverColor(int index, const Maths::FVector4& color);
		void BuildBvh(const std::vector<::Rendering::Geometry::bbox>&boxs,const  std::vector<uint32_t>&subMeshRanges);
		std::vector<Core::SceneSystem::RectPickRes>RectPick(const Maths::FMatrix4& modelMatrix, const Maths::FMatrix4& viewProj, float su, float sv, float eu, float ev);
		virtual void OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;
		virtual void OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;
	private:
		class CBatchMeshInternal;
		CBatchMeshInternal* mInternal = nullptr;
	};
}