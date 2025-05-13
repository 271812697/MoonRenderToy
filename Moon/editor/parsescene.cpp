#include "parsescene.h"
#include "renderer/Context.h"

#include "Core/Global/ServiceLocator.h"
#include "pathtrace/Scene.h"
#include "pathtrace/PathTrace.h"
#include "Core/ECS/Components/CMaterialRenderer.h"
#include "core/ECS/Components/CPointLight.h"
#include "Core/Global/ServiceLocator.h"



namespace MOON {



	void ParseScene::ParsePathTraceScene() {
		PathTrace::Scene* sce = PathTrace::GetScene();
		OVSERVICE(::Editor::Core::Context).sceneManager.LoadEmptyScene();
		::Core::SceneSystem::Scene* scene = OVSERVICE(::Editor::Core::Context).sceneManager.GetCurrentScene();
		if (sce == nullptr || scene == nullptr) {
			return;
		}
		scene->AddDefaultLights();
		auto& pointLight = scene->CreateActor("Ambient Light").AddComponent<::Core::ECS::Components::CPointLight>();
		pointLight.SetIntensity(10.0);


		auto& mesh = sce->meshes;
		auto& instance = sce->meshInstances;
		auto& material = sce->materials;
		for (auto& mi : instance) {
			::Core::Resources::Material* tempMat = new ::Core::Resources::Material();
			::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::MaterialManager>().RegisterResource(mi.name, tempMat);
			tempMat->SetShader(::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().shaderManager[":Shaders\\Standard.ovfx"]);
			auto& color = material[mi.materialID].baseColor;
			tempMat->SetProperty("u_Diffuse", Maths::FVector4{ color.x,  color.y, color.z, 1.0f });
			tempMat->SetBackfaceCulling(false);;
			tempMat->SetCastShadows(false);
			tempMat->SetReceiveShadows(false);
			auto& actor = scene->CreateActor();
			actor.SetName(mi.name);
			auto m = ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::ModelManager>().GetResource("#" + mesh[mi.meshID]->name);
			actor.AddComponent<::Core::ECS::Components::CModelRenderer>().SetModel(m);
			actor.GetComponent<::Core::ECS::Components::CTransform>()->SetMatrix(mi.transform.data);
			auto& materilaRener = actor.AddComponent<::Core::ECS::Components::CMaterialRenderer>();
			materilaRener.SetMaterialAtIndex(0, *tempMat);
			materilaRener.UpdateMaterialList();
		}
	}

}