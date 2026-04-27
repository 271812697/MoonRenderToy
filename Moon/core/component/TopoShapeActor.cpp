#include "core/component/TopoShapeActor.h"
#include <Core/ResourceManagement/MaterialManager.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/ECS/Components/CModelRenderer.h>
#include "Core/ECS/Components/CBatchMeshTriangle.h"
#include "Core/ECS/Components/CBatchMeshLine.h"
#include "Core/ResourceManagement/ModelManager.h"
#include "core/component/CTopoShape.h"
#include <Core/Global/ServiceLocator.h>
#include <Core/SceneSystem/Scene.h>

namespace Core::ECS {

	TopoActor::TopoActor(Core::SceneSystem::Scene* scene, const std::string&p_name , const std::string& p_tag, bool p_playing) : Actor(scene->GetAvailableID(), p_name, p_tag, p_playing)
	{
		scene->AddActor(this);
		AddComponent<Core::ECS::Components::CModelRenderer>();
		AddComponent<Core::ECS::Components::CMaterialRenderer>();
		AddComponent<Core::ECS::Components::CBatchMeshTriangle>();
		AddComponent<Core::ECS::Components::CBatchMeshLine>();
		AddComponent<Core::ECS::Components::CTopoShape>();
		
		auto model = new ::Rendering::Resources::Model(p_name + std::string("_Model")+std::to_string(this->GetID()));
		GetComponent<Core::ECS::Components::CModelRenderer>()->SetModel(model);
		Core::Global::ServiceLocator::Get<Core::ResourceManagement::ModelManager>().RegisterResource(p_name + std::string("_Model") + std::to_string(this->GetID()), model);
	}

	TopoActor::~TopoActor()
	{
	}

	void TopoActor::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_actorsRoot)
	{
		Actor::OnSerialize(p_doc, p_actorsRoot);
	}

	void TopoActor::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_actorsRoot)
	{
		Actor::OnDeserialize(p_doc, p_actorsRoot);
	}

}