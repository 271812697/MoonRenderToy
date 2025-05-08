#include "PathTrace.h"
#include "LoadScene.h"
#include "Scene.h"
#include "RendererOptions.h"
#include "Camera.h"
#include "Renderer.h"
#include "core/log.h"
#include "Trace.h"
#include <filesystem>
namespace PathTrace {

	Scene* scene = nullptr;
	Renderer* renderer = nullptr;
	std::vector<string> sceneFiles;
	std::vector<string> envMaps;
	int sampleSceneIdx = 0;
	int selectedInstance = -1;
	int selectedMat = 0;
	int envMapIdx = 0;
	float CameraMoveSpeed = 3.0f;
	bool objectPropChanged = false;
	bool showTransform = false;
	bool space_down = false;
	bool switchScene = false;
	std::string switchSceneName = "";
	float screenX[2] = { 0,0 };
	float screenY[2] = { 0,0 };

	std::string shadersDir = "C:/Project/UseQt/Moon/pathtrace/shaders/";
	std::string assetsDir = "C:/Project/UseQt/Resource/pathtrace/scenes/";
	std::string envMapDir = "C:/Project/UseQt/Resource/pathtrace/scenes/HDR/";
	//std::string shadersDir = "../../Moon/pathtrace/shaders/";
	//std::string assetsDir = "../../Resource/pathtrace/scenes/";
	//std::string envMapDir = "../../Resource/pathtrace/scenes/HDR/";

	RenderOptions renderOptions;

	Scene* GetScene() {
		return scene;
	}
	Renderer* GetRenderer() {
		return renderer;
	}
	RenderOptions& GetRenderOptions() {
		return renderOptions;
	}
	void Update() {
		if (switchScene) {
			switchScene = false;
			LoadScene(switchSceneName);
			InitRenderer();
		}
	}
	void GetSceneFiles()
	{
		std::filesystem::directory_entry p_directory(assetsDir);
		for (auto& item : std::filesystem::directory_iterator(p_directory))
			if (!item.is_directory()) {
				auto ext = item.path().extension();
				if (ext == ".scene" || ext == ".gltf" || ext == ".glb")
				{
					sceneFiles.push_back(item.path().generic_string());
				}
			}
	}
	void Resize(int width, int height) {
		renderOptions.windowResolution.x = width;
		renderOptions.windowResolution.y = height;
		if (!renderOptions.independentRenderSize)
			renderOptions.renderResolution = renderOptions.windowResolution;
		scene->renderOptions = renderOptions;
		renderer->ResizeRenderer();
	}
	void GetEnvMaps()
	{
		std::filesystem::directory_entry p_directory(envMapDir);
		for (auto& item : std::filesystem::directory_iterator(p_directory)) {
			if (item.path().extension() == ".hdr")
			{
				envMaps.push_back(item.path().generic_string());
			}
		}
	}

	void SwitchScene(std::string sceneName) {
		switchScene = true;
		switchSceneName = sceneName;
	}
	void LoadScene(std::string sceneName)
	{
		delete scene;
		scene = new Scene();
		std::string ext = sceneName.substr(sceneName.find_last_of(".") + 1);

		bool success = false;
		Mat4 xform;

		if (ext == "scene")
			success = LoadSceneFromFile(sceneName, scene, renderOptions);
		else if (ext == "gltf")
			success = LoadGLTF(sceneName, scene, renderOptions, xform, false);
		else if (ext == "glb")
			success = LoadGLTF(sceneName, scene, renderOptions, xform, true);

		if (!success)
		{
			CORE_ERROR("Unable to load scene");
			exit(0);
		}

		selectedInstance = 0;
		selectedMat = 0;
		// Add a default HDR if there are no lights in the scene
		if (!scene->envMap && !envMaps.empty())
		{
			scene->AddEnvMap(envMaps[envMapIdx]);
			renderOptions.enableEnvMap = true;
			renderOptions.envMapIntensity = 1.5f;
		}

		scene->renderOptions = renderOptions;
	}
	void TraceScene()
	{

		//TraceScreen(renderOptions.windowResolution.x, renderOptions.windowResolution.y);
	}
	bool InitRenderer()
	{
		delete renderer;
		renderer = new Renderer(scene, shadersDir);
		return true;
	}
	void Ret() {
		delete renderer;
		delete scene;
	}

	CameraController::CameraController()
	{
	}

	CameraController& CameraController::Instance()
	{
		static CameraController instance;
		return instance;
	}

	void CameraController::mouseMove(int _x, int _y)
	{



		if (mouseMiddle) {
			scene->camera->Strafe((_x - tx) * 0.1, (_y - ty) * 0.1);
			scene->dirty = true;
		}
		else if (mouseRight)
		{

			scene->camera->OffsetRotateByScreen((_x - rx) * 0.5, (ry - _y) * 0.5);
			scene->dirty = true;
		}
		//x = _x;
		//y = _y;
	}

	void CameraController::mouseLeftPress(int x, int y)
	{

		scene->IntersectionByScreen(1.0 * x / renderOptions.windowResolution.x, 1.0 - 1.0 * y / renderOptions.windowResolution.y);;
	}

	void CameraController::mouseMiddlePress(int x, int y)
	{
		this->tx = x;
		this->ty = y;
		scene->camera->UpdateCamera();
		mouseMiddle = true;
	}

	void CameraController::mouseRightPress(int x, int y)
	{
		this->rx = x;
		this->ry = y;
		scene->camera->UpdateCamera();
		mouseRight = true;
	}

	void CameraController::mouseMiddleRelease(int x, int y)
	{
		mouseMiddle = false;
	}

	void CameraController::mouseRightRelease(int x, int y)
	{
		mouseRight = false;
	}

	void CameraController::wheelMouseWheel(float delta)
	{
		scene->camera->SetRadius(delta * 0.0025);
		GetScene()->dirty = true;
	}
	void CameraController::GetCameraPosition(float eye[3]) {
		Vec3 p = scene->camera->GetEye();
		eye[0] = p.x;
		eye[1] = p.y;
		eye[2] = p.z;
	}
	void CameraController::GetViewProject(float view[16], float proj[16]) {
		scene->camera->ComputeViewProjectionMatrix(view, proj, 1.0f * renderOptions.windowResolution.x / renderOptions.windowResolution.y);
	}

}