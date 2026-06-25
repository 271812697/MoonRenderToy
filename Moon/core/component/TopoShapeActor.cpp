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
		Core::ECS::Actor& faceChild=scene->CreateActor("Face");
		Core::ECS::Actor& edgeChild = scene->CreateActor("Edge");
		faceChild.SetParent(*this);
		edgeChild.SetParent(*this);
		AddComponent<Core::ECS::Components::CTopoShape>();
		faceChild.AddComponent<Core::ECS::Components::CModelRenderer>();
		faceChild.AddComponent<Core::ECS::Components::CMaterialRenderer>();
		faceChild.AddComponent<Core::ECS::Components::CBatchMeshTriangle>();
		edgeChild.AddComponent<Core::ECS::Components::CModelRenderer>();
		edgeChild.AddComponent<Core::ECS::Components::CMaterialRenderer>();
		edgeChild.AddComponent<Core::ECS::Components::CBatchMeshLine>();
		
		
		auto faceModel = new ::Rendering::Resources::Model(p_name + std::string("_faceModel")+std::to_string(this->GetID()));
		faceChild.GetComponent<Core::ECS::Components::CModelRenderer>()->SetModel(faceModel);
		GetService(Core::ResourceManagement::ModelManager).RegisterResource(p_name + std::string("_faceModel") + std::to_string(this->GetID()), faceModel);
		auto edgeModel = new ::Rendering::Resources::Model(p_name + std::string("_edgeModel") + std::to_string(this->GetID()));
		edgeChild.GetComponent<Core::ECS::Components::CModelRenderer>()->SetModel(edgeModel);
		GetService(Core::ResourceManagement::ModelManager).RegisterResource(p_name + std::string("_edgeModel") + std::to_string(this->GetID()), edgeModel);

		Core::Resources::Material* lineMat = new Core::Resources::Material();
		Core::Global::ServiceLocator::Get<Core::ResourceManagement::MaterialManager>().RegisterResource(p_name + std::to_string(this->GetID()) + "_linemat",lineMat);
		Core::Resources::Material* faceMat = new Core::Resources::Material();
		Core::Global::ServiceLocator::Get<Core::ResourceManagement::MaterialManager>().RegisterResource(p_name + std::to_string(this->GetID()) + "_facemat", faceMat);
		Core::Resources::Material* faceTransparentMat = new Core::Resources::Material();
		Core::Global::ServiceLocator::Get<Core::ResourceManagement::MaterialManager>().RegisterResource(p_name + std::to_string(this->GetID()) + "_faceTransparentMat", faceTransparentMat);
		faceChild.GetComponent<Core::ECS::Components::CMaterialRenderer>()->SetMaterialAtIndex(0, *faceMat);
		faceChild.GetComponent<Core::ECS::Components::CMaterialRenderer>()->SetMaterialAtIndex(1, *faceTransparentMat);		
		edgeChild.GetComponent<Core::ECS::Components::CMaterialRenderer>()->SetMaterialAtIndex(0, *lineMat);
		faceChild.GetComponent<Core::ECS::Components::CMaterialRenderer>()->UpdateMaterialList();
		edgeChild.GetComponent<Core::ECS::Components::CMaterialRenderer>()->UpdateMaterialList();
		{
			auto& renderer=GetSceneView.GetRenderer();;
			{
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


				faceTransparentMat->SetBackfaceCulling(false);
				faceTransparentMat->SetCastShadows(false);
				faceTransparentMat->SetReceiveShadows(false);
				//tempMat->SetBlendable(true);
				//tempMat->SetDepthWriting(false);
				faceTransparentMat->SetShader(Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager[":Shaders\\GeomertySurface.ovfx"]);
				faceTransparentMat->SetProperty("u_Albedo", Maths::FVector4(1, 1, 1, 0.5));
				faceTransparentMat->SetProperty("u_AlphaClippingThreshold", 0.0f);
				faceTransparentMat->SetProperty("u_Roughness", 0.25f);
				faceTransparentMat->SetProperty("u_Metallic", 0.75f);
				// Emission
				faceTransparentMat->SetProperty("u_EmissiveIntensity", 1.0f);
				faceTransparentMat->SetProperty("u_EmissiveColor", Maths::FVector3{ 0.0f, 0.0f, 0.0f });

				faceTransparentMat->TrySetProperty("_IrradianceCube", renderer.GetIrradianceCube());
				faceTransparentMat->TrySetProperty("_PrefilterCube", renderer.GetPrefilterCube());
				faceTransparentMat->TrySetProperty("_BRDFLut", renderer.GetBrdfTexture());
				faceTransparentMat->SetTransparent(true);
				faceTransparentMat->SetDepthWriting(true);
			}
			{
				lineMat->SetShader(Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager[":Shaders\\GeomertyLine.ovfx"]);
				lineMat->SetBackfaceCulling(false);
				lineMat->SetCastShadows(false);
				lineMat->SetReceiveShadows(false);
				lineMat->SetLineWidth(2.0);
				lineMat->AddFeature("BATCHLINE");
			}

		}
		//GetViewerWidget.updateTreeView();
	}

	void TopoActor::ClearModel()
	{
		GetChild("Face")->GetComponent<Core::ECS::Components::CModelRenderer>()->GetModel()->ClearMeshes();
		GetChild("Edge")->GetComponent<Core::ECS::Components::CModelRenderer>()->GetModel()->ClearMeshes();
	}

	TopoActor::~TopoActor()
	{
		//m_scene->

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