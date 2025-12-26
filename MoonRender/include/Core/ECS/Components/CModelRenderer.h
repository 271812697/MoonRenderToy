#pragma once
#include <Rendering/Geometry/Vertex.h>
#include <Rendering/Resources/Model.h>
#include "Core/ECS/Components/AComponent.h"
namespace Core::ECS { class Actor; }
namespace Core::ECS::Components
{
	class CModelRenderer : public AComponent
	{
	public:
		enum class EFrustumBehaviour
		{
			DISABLED = 0,
			DEPRECATED_MODEL_BOUNDS = 1, // This is not used anymore, but the enum value is kept for compatibility
			MESH_BOUNDS = 2,
			CUSTOM_BOUNDS = 3
		};

		CModelRenderer(ECS::Actor& p_owner);
		std::string GetName() override;
		void SetModel(::Rendering::Resources::Model* p_model);
		::Rendering::Resources::Model* GetModel() const;
		void SetFrustumBehaviour(EFrustumBehaviour p_boundingMode);
		EFrustumBehaviour GetFrustumBehaviour() const;
		const ::Rendering::Geometry::BoundingSphere& GetCustomBoundingSphere() const;
		const ::Rendering::Geometry::bbox GetBoundingBox() const;
	
		void SetCustomBoundingSphere(const ::Rendering::Geometry::BoundingSphere& p_boundingSphere);
		virtual void OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;
		virtual void OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;


	private:
		::Rendering::Resources::Model* m_model = nullptr;

		::Rendering::Geometry::BoundingSphere m_customBoundingSphere = { {}, 1.0f };
		EFrustumBehaviour m_frustumBehaviour = EFrustumBehaviour::MESH_BOUNDS;
	};
}