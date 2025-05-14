#include "parsescene.h"
#include "renderer/Context.h"

#include "Core/Global/ServiceLocator.h"
#include "pathtrace/Scene.h"
#include "pathtrace/PathTrace.h"
#include "Core/ECS/Components/CMaterialRenderer.h"
#include "core/ECS/Components/CPointLight.h"
#include "Core/Global/ServiceLocator.h"



namespace MOON {
	Maths::FVector3 GetSpherePosition(float a,float b,float radius) {

		float elevation = a / 180.0 * 3.14159265;
		float azimuth = b / 180.0 * 3.14159265;
		return Maths::FVector3(cos(elevation)*sin(azimuth),sin(elevation),cos(elevation) * cos(azimuth))*radius;
	}


	void addSphereLight(::Core::SceneSystem::Scene* scene) {

		auto ambient=scene->FindActorByName("Ambient Light");
		auto& ac1=scene->CreateActor("PointLight1");
		auto& pointLight = ac1.AddComponent<::Core::ECS::Components::CPointLight>();
		pointLight.SetIntensity(0.75);
		pointLight.SetConstant(1.0);
		//pointLight.SetLinear(0.0);

		pointLight.SetQuadratic(0.0);

		ac1.SetParent(*ambient);
		ac1.transform.SetLocalPosition(GetSpherePosition(50,10,999));

		auto& ac2 = scene->CreateActor("PointLight2");
		auto& pointLight2 = ac2.AddComponent<::Core::ECS::Components::CPointLight>();
		pointLight2.SetIntensity(0.25);
		pointLight2.SetConstant(1.0);
		//pointLight.SetLinear(0.0);

		pointLight2.SetQuadratic(0.0);

		ac2.SetParent(*ambient);
		ac2.transform.SetLocalPosition(GetSpherePosition(-75, 10, 999));

		auto& ac3 = scene->CreateActor("PointLight3");
		auto& pointLight3 = ac3.AddComponent<::Core::ECS::Components::CPointLight>();
		pointLight3.SetIntensity(0.214);
		pointLight3.SetConstant(1.0);
		//pointLight.SetLinear(0.0);

		pointLight3.SetQuadratic(0.0);

		ac3.SetParent(*ambient);
		ac3.transform.SetLocalPosition(GetSpherePosition(0, 110, 999));
		
		auto& ac4 = scene->CreateActor("PointLight4");
		auto& pointLight4 = ac4.AddComponent<::Core::ECS::Components::CPointLight>();
		pointLight4.SetIntensity(0.214);
		pointLight4.SetConstant(1.0);
		//pointLight.SetLinear(0.0);

		pointLight4.SetQuadratic(0.0);

		ac4.SetParent(*ambient);
		ac4.transform.SetLocalPosition(GetSpherePosition(0, -110, 999));

		auto& ac5 = scene->CreateActor("HeadLight");
		auto& pointLight5 = ac5.AddComponent<::Core::ECS::Components::CPointLight>();
		pointLight5.SetIntensity(0.25);
		pointLight5.SetConstant(1.0);
		//pointLight.SetLinear(0.0);

		pointLight5.SetQuadratic(0.0);
	}

	void ParseScene::ParsePathTraceScene() {
		PathTrace::Scene* sce = PathTrace::GetScene();
		OVSERVICE(::Editor::Core::Context).sceneManager.LoadEmptyScene();
		::Core::SceneSystem::Scene* scene = OVSERVICE(::Editor::Core::Context).sceneManager.GetCurrentScene();
		if (sce == nullptr || scene == nullptr) {
			return;
		}
		scene->AddDefaultLights();
		addSphereLight(scene);
		//addSphereLight(scene);
		//addSphereLight(scene);

		//addSphereLight(scene);

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