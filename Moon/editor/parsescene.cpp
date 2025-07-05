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
		scene->FindActorByName("Directional Light")->GetComponent<OvCore::ECS::Components::CDirectionalLight>()->SetIntensity(1.0f);
		scene->FindActorByName("Directional Light")->GetComponent<OvCore::ECS::Components::CDirectionalLight>()->GetData().castShadows = true;
		addSphereLight(scene);
		PathTrace::LoadSceneFromFile(PathTrace::GetSceneFilePath(), scene);


	}

}