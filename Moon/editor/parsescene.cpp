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
#include "Settings/DebugSetting.h"
#include "io/io_occ_step.h"


namespace MOON {
	OvMaths::FVector3 GetSpherePosition(float a, float b, float radius) {

		float elevation = a / 180.0 * 3.14159265;
		float azimuth = b / 180.0 * 3.14159265;
		return OvMaths::FVector3(cos(elevation) * sin(azimuth), sin(elevation), cos(elevation) * cos(azimuth)) * radius;
	}


	void addSphereLight(OvCore::SceneSystem::Scene* scene) {
		auto node = DebugSettings::instance().getNode("showLight");
		DebugSettings::instance().addCallBack("showLight", [=]() {
			bool value = node->getData<bool>();
			scene->FindActorByName("PointLight1")->SetActive(value);
			scene->FindActorByName("PointLight2")->SetActive(value);
			scene->FindActorByName("PointLight3")->SetActive(value);
			scene->FindActorByName("PointLight4")->SetActive(value);
			});
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



	void ParseScene::ParsePathTraceScene(const std::string& path) {

		GetService(OvEditor::Core::Context).sceneManager.LoadDefaultScene();
		OvCore::SceneSystem::Scene* scene = GetService(OvEditor::Core::Context).sceneManager.GetCurrentScene();
		if (scene == nullptr) {
			return;
		}
		scene->FindActorByName("Directional Light")->GetComponent<OvCore::ECS::Components::CDirectionalLight>()->SetIntensity(1.0f);
		scene->FindActorByName("Directional Light")->GetComponent<OvCore::ECS::Components::CDirectionalLight>()->GetData().castShadows = true;
		addSphereLight(scene);
		std::string sceneName = path;
		std::string ext = sceneName.substr(sceneName.find_last_of(".") + 1);
		Mat4 xform;
		if (ext == "scene")
			PathTrace::LoadSceneFromFile(sceneName, scene);
		else if (ext == "gltf")
			PathTrace::LoadGLTF(sceneName, scene, xform, false);
		else if (ext == "glb")
			PathTrace::LoadGLTF(sceneName, scene, xform, true);
		else if (ext == "STEP"|| ext== "stp" ) {
			IO::ReadSTEP(sceneName.c_str(), scene);
		}
		else
		{
			auto model = OvCore::Global::ServiceLocator::Get<OvCore::ResourceManagement::ModelManager>().LoadResource(sceneName);


			OvCore::Resources::Material* tempMat = new OvCore::Resources::Material();
			OvCore::Global::ServiceLocator::Get<OvCore::ResourceManagement::MaterialManager>().RegisterResource(sceneName, tempMat);
			tempMat->SetBackfaceCulling(false);;
			tempMat->SetCastShadows(false);
			tempMat->SetReceiveShadows(false);

			tempMat->SetShader(OvCore::Global::ServiceLocator::Get<OvEditor::Core::Context>().shaderManager[":Shaders\\Standard.ovfx"]);
			tempMat->SetProperty("u_Albedo", OvMaths::FVector4{ 1.0, 1.0, 1.0, 1.0 });

			tempMat->SetProperty("u_AlphaClippingThreshold", 1.0f);
			tempMat->SetProperty("u_Roughness", 0.1f);
			tempMat->SetProperty("u_Metallic", 0.1f);
			// Emission
			tempMat->SetProperty("u_EmissiveIntensity", 1.0f);
			tempMat->SetProperty("u_EmissiveColor", OvMaths::FVector3{ 0.0f,0.0f,0.0f });

			auto& actor = scene->CreateActor();
			actor.AddComponent<OvCore::ECS::Components::CModelRenderer>().SetModel(model);

			actor.GetComponent<OvCore::ECS::Components::CTransform>()->SetMatrix(xform.data);
			auto& materilaRener = actor.AddComponent<OvCore::ECS::Components::CMaterialRenderer>();
			materilaRener.SetMaterialAtIndex(0, *tempMat);
			materilaRener.UpdateMaterialList();

		}

	}

	void ParseScene::updateTreeViewSceneRoot()
	{

	}

	void ParseScene::updateTreeViewPathRoot()
	{
	}

	ParseScene::ParseScene(QObject* parent) :QObject(parent)
	{
	}

}