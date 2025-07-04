#include "parsescene.h"
#include "renderer/Context.h"

#include "OvCore/Global/ServiceLocator.h"
#include "pathtrace/Scene.h"
#include "pathtrace/PathTrace.h"
#include "OvCore/ECS/Components/CMaterialRenderer.h"
#include "OvCore/ECS/Components/CPointLight.h"
#include "OvCore/ECS/Components/CDirectionalLight.h"
#include "OvCore/ECS/Components/CAmbientSphereLight.h"
#include "OvCore/ECS/Components/CPostProcessStack.h"
#include "pathtrace/LoadScene.h"



namespace MOON {
	OvMaths::FVector3 GetSpherePosition(float a, float b, float radius) {

		float elevation = a / 180.0 * 3.14159265;
		float azimuth = b / 180.0 * 3.14159265;
		return OvMaths::FVector3(cos(elevation) * sin(azimuth), sin(elevation), cos(elevation) * cos(azimuth)) * radius;
	}


	void addSphereLight(OvCore::SceneSystem::Scene* scene) {

		auto ambient = scene->FindActorByName("Ambient Light");
		auto& ac1 = scene->CreateActor("PointLight1");
		auto& pointLight1 = ac1.AddComponent<OvCore::ECS::Components::CPointLight>();
		float kI = 0.50;
		float kB = kI / 3.0;
		float kC = kI / 3.5;

		pointLight1.SetIntensity(kI);
		pointLight1.SetConstant(1.0);
		//pointLight.SetLinear(0.0);

		pointLight1.SetQuadratic(0.0);

		//ac1.SetParent(*ambient);
		//ac1.transform.SetLocalPosition(GetSpherePosition(50, 10, 999));

		auto& ac2 = scene->CreateActor("PointLight2");
		auto& pointLight2 = ac2.AddComponent<OvCore::ECS::Components::CPointLight>();
		pointLight2.SetIntensity(kB);
		pointLight2.SetConstant(1.0);
		//pointLight.SetLinear(0.0);

		pointLight2.SetQuadratic(0.0);

		//ac2.SetParent(*ambient);
		//ac2.transform.SetLocalPosition(GetSpherePosition(-75, 10, 999));

		auto& ac3 = scene->CreateActor("PointLight3");
		auto& pointLight3 = ac3.AddComponent<OvCore::ECS::Components::CPointLight>();
		pointLight3.SetIntensity(kC);
		pointLight3.SetConstant(1.0);
		//pointLight.SetLinear(0.0);

		pointLight3.SetQuadratic(0.0);

		//ac3.SetParent(*ambient);
		//ac3.transform.SetLocalPosition(GetSpherePosition(0, 110, 999));

		auto& ac4 = scene->CreateActor("PointLight4");
		auto& pointLight4 = ac4.AddComponent<OvCore::ECS::Components::CPointLight>();

		pointLight4.SetIntensity(kC);
		pointLight4.SetConstant(1.0);
		//pointLight.SetLinear(0.0);

		pointLight4.SetQuadratic(0.0);

		//ac4.SetParent(*ambient);
		//ac4.transform.SetLocalPosition(GetSpherePosition(0, -110, 999));

		auto& ac5 = scene->CreateActor("HeadLight");
		auto& pointLight5 = ac5.AddComponent<OvCore::ECS::Components::CPointLight>();
		pointLight5.SetIntensity(kB);
		pointLight5.SetConstant(1.0);
		//pointLight.SetLinear(0.0);

		pointLight5.SetQuadratic(0.0);
	}

	void ParseScene::ParsePathTraceScene() {
		PathTrace::Scene* sce = PathTrace::GetScene();
		OVSERVICE(OvEditor::Core::Context).sceneManager.LoadDefaultScene();
		OvCore::SceneSystem::Scene* scene = OVSERVICE(OvEditor::Core::Context).sceneManager.GetCurrentScene();
		if (sce == nullptr || scene == nullptr) {
			return;
		}
		//scene->AddDefaultLights();
		//scene->AddDefaultPostProcessStack();
		//auto& tonesettings = scene->FindActorByName("Post Process Stack")->GetComponent<OvCore::ECS::Components::CPostProcessStack>()->GetTonemappingSettings();
		//tonesettings.gammaCorrection = false;
		scene->FindActorByName("Directional Light")->GetComponent<OvCore::ECS::Components::CDirectionalLight>()->SetIntensity(1.0f);
		addSphereLight(scene);
		PathTrace::LoadSceneFromFile(PathTrace::GetSceneFilePath(),scene);

		//auto& mesh = sce->meshes;
		//auto& instance = sce->meshInstances;
		//auto& material = sce->materials;
		//for (int i = 0; i < mesh.size(); i++) {
		//	auto m = OvCore::Global::ServiceLocator::Get<OvCore::ResourceManagement::ModelManager>().LoadFromMemory(mesh[i]->PackData(), {});
		//	std::string name = "Scene::Mesh" + std::to_string(i);
		//	OvCore::Global::ServiceLocator::Get<OvCore::ResourceManagement::ModelManager>().RegisterResource(name, m);
		//}
		//for (auto& mi : instance) {
		//	OvCore::Resources::Material* tempMat = new OvCore::Resources::Material();
		//	OvCore::Global::ServiceLocator::Get<OvCore::ResourceManagement::MaterialManager>().RegisterResource(mi.name, tempMat);
		//	tempMat->SetShader(OvCore::Global::ServiceLocator::Get<OvEditor::Core::Context>().shaderManager[":Shaders\\Standard.ovfx"]);
		//	auto& color = material[mi.materialID].baseColor;
		//	tempMat->SetProperty("u_Albedo", OvMaths::FVector4{ color.x,  color.y, color.z, 1.0f });
		//	tempMat->SetProperty("u_Metallic", 0.3f);
		//	tempMat->SetProperty("u_Roughness", 0.3f);
		//	//tempMat->SetProperty("u_Diffuse", Maths::FVector4{ 1.0,  1.0, 1.0, 1.0f });
		//	tempMat->SetBackfaceCulling(false);;
		//	tempMat->SetCastShadows(false);
		//	tempMat->SetReceiveShadows(false);
		//	auto& actor = scene->CreateActor();
		//	actor.SetName(mi.name);
		//	auto m = OvCore::Global::ServiceLocator::Get<OvCore::ResourceManagement::ModelManager>().GetResource("Scene::Mesh" + std::to_string(mi.meshID));
		//	//auto m = ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::ModelManager>().GetResource("#" + mesh[mi.meshID]->name);
		//	actor.AddComponent<OvCore::ECS::Components::CModelRenderer>().SetModel(m);
		//	actor.GetComponent<OvCore::ECS::Components::CTransform>()->SetMatrix(mi.transform.data);
		//	auto& materilaRener = actor.AddComponent<OvCore::ECS::Components::CMaterialRenderer>();
		//	materilaRener.SetMaterialAtIndex(0, *tempMat);
		//	materilaRener.UpdateMaterialList();
		//}
	}

}