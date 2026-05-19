#include "core/component/TopoShapeActor.h"
#include "renderer/SceneView.h"
#include <Core/ResourceManagement/MaterialManager.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/ECS/Components/CModelRenderer.h>
#include "Core/ECS/Components/CBatchMeshTriangle.h"
#include "Core/ECS/Components/CBatchMeshLine.h"
#include "Core/ResourceManagement/ModelManager.h"
#include "editor/View/sceneview/viewerwidget.h"
#include "core/component/CTopoShape.h"
#include <Core/Global/ServiceLocator.h>
#include <Core/SceneSystem/Scene.h>

namespace Core::ECS {

	TopoActor::TopoActor(Core::SceneSystem::Scene* scene, const std::string&p_name , const std::string& p_tag, bool p_playing) : Actor(scene->GetAvailableID(), p_name, p_tag, p_playing)
	{
		m_scene = scene;
		scene->AddActor(this);
		AddComponent<Core::ECS::Components::CModelRenderer>();
		AddComponent<Core::ECS::Components::CMaterialRenderer>();
		AddComponent<Core::ECS::Components::CBatchMeshTriangle>();
		AddComponent<Core::ECS::Components::CBatchMeshLine>();
		AddComponent<Core::ECS::Components::CTopoShape>();
		
		auto model = new ::Rendering::Resources::Model(p_name + std::string("_Model")+std::to_string(this->GetID()));
		GetComponent<Core::ECS::Components::CModelRenderer>()->SetModel(model);


		Core::Resources::Material* lineMat = new Core::Resources::Material();
		Core::Global::ServiceLocator::Get<Core::ResourceManagement::MaterialManager>().RegisterResource(p_name + std::to_string(this->GetID()) + "_linemat",lineMat);
		Core::Resources::Material* faceMat = new Core::Resources::Material();
		Core::Global::ServiceLocator::Get<Core::ResourceManagement::MaterialManager>().RegisterResource(p_name + std::to_string(this->GetID()) + "_facemat", faceMat);
		GetComponent<Core::ECS::Components::CMaterialRenderer>()->SetMaterialAtIndex(0, *faceMat);	
		GetComponent<Core::ECS::Components::CMaterialRenderer>()->SetMaterialAtIndex(1, *lineMat);
		GetComponent<Core::ECS::Components::CMaterialRenderer>()->UpdateMaterialList();
		{
			auto& renderer=GetSceneView.GetRenderer();;
			
			faceMat->SetBackfaceCulling(false);
			faceMat->SetCastShadows(false);
			faceMat->SetReceiveShadows(false);
			//tempMat->SetBlendable(true);
			//tempMat->SetDepthWriting(false);
			faceMat->SetShader(Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager[":Shaders\\GeomertySurface.ovfx"]);
			faceMat->SetProperty("u_Albedo",Maths::FVector4(1,1,1,1));
			faceMat->SetProperty("u_AlphaClippingThreshold", 0.0f);
			faceMat->SetProperty("u_Roughness", 0.25f);
			faceMat->SetProperty("u_Metallic", 0.75f);
			// Emission
			faceMat->SetProperty("u_EmissiveIntensity", 1.0f);
			faceMat->SetProperty("u_EmissiveColor", Maths::FVector3{ 0.0f, 0.0f, 0.0f });

			faceMat->TrySetProperty("_IrradianceCube", renderer.GetIrradianceCube());
			faceMat->TrySetProperty("_PrefilterCube", renderer.GetPrefilterCube());
			faceMat->TrySetProperty("_BRDFLut", renderer.GetBrdfTexture());

			lineMat->SetShader(Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager[":Shaders\\GeomertyLine.ovfx"]);
			lineMat->SetBackfaceCulling(false);
			lineMat->SetCastShadows(false);
			lineMat->SetReceiveShadows(false);
			lineMat->SetLineWidth(2.0);
			lineMat->AddFeature("BATCHLINE");
		}
		GetService(Core::ResourceManagement::ModelManager).RegisterResource(p_name + std::string("_Model") + std::to_string(this->GetID()), model);
		GetViewerWidget.updateTreeView();
		
	}

	void TopoActor::ClearModel()
	{
		GetComponent<Core::ECS::Components::CModelRenderer>()->GetModel()->ClearMeshes();
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

	void TopoActor::RemoveFromScene()
	{
		if (m_scene) {
			m_scene->RemoveActor(this);
		}		
	}
}