#include "editor/UI/TreeViewPanel/treeViewpanel.h"
#include "editor/View/pathtrace/pathtracePanel.h"
#include "OvCore/Global/ServiceLocator.h"
#define __glad_h_
#include "PathTrace.h"
#include "LoadScene.h"
#include "Scene.h"
#include "RendererOptions.h"
#include "Camera.h"
#include "Renderer.h"
#include "core/log.h"
#include "Trace.h"

#include <filesystem>
#include <QMouseEvent>

namespace PathTrace {

	std::string shadersDir = PATH_TRACE_SHADER_PATH;
	std::string assetsDir = PATH_TRACE_SCENE_PATH;
	std::string envMapDir = PATH_TRACE_HDR_PATH;

	class CameraController {

	public:
		CameraController(PathTraceRender* ptr);

		void mouseMove(int _x, int _y);
		void mouseLeftPress(int x, int y);
		void mouseMiddlePress(int x, int y);
		void mouseRightPress(int x, int y);
		void mouseMiddleRelease(int x, int y);
		void mouseRightRelease(int x, int y);
		void wheelMouseWheel(float delta);
		void GetCameraPosition(float eye[3]);
		void GetViewProject(float view[16], float proj[16]);
		void MoveToPivot(float deltaTime);
		void PustCameraDestination(float x, float y, float z);
	private:
		std::vector<Vec3>cameraDestinations;
		PathTraceRender* render = nullptr;
		//for rotate
		int rx, ry;
		//for translate
		int tx, ty;
		bool mouseMiddle = false;

		bool mouseRight = false;
	};
	class PathTraceRender::PathTraceRenderInternal {
	public:
		PathTraceRenderInternal(PathTraceRender* render) :mSelf(render), cameraController(new CameraController(render)) {}

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
				OVSERVICE(MOON::PathTracePanel).onUpdateEntityTreeView();
			}
			cameraController->MoveToPivot(0.016);
			GetRenderer()->Update(0.016);
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
			switchSceneName = sceneFiles[sampleSceneIdx];
		}
		void Resize(int width, int height) {
			renderOptions.windowResolution.x = width;
			renderOptions.windowResolution.y = height;
			if (!renderOptions.independentRenderSize)
				renderOptions.renderResolution = renderOptions.windowResolution;
			scene->getRenderOptions() = renderOptions;
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
		std::string GetSceneFilePath()
		{
			return switchSceneName;
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
			else {
				success = LoadSingleModel(sceneName, scene);
			}

			if (!success)
			{
				CORE_ERROR("Unable to load scene");
				exit(0);
			}

			selectedInstance = 0;
			selectedMat = 0;
			// Add a default HDR if there are no lights in the scene
			if (!scene->getEnvironmentMap() && !envMaps.empty())
			{
				scene->AddEnvMap(envMaps[envMapIdx]);
				renderOptions.enableEnvMap = true;
				renderOptions.envMapIntensity = 1.5f;
			}

			scene->getRenderOptions() = renderOptions;
		}
		void LoadDefaultScene() {
			LoadScene(sceneFiles[sampleSceneIdx]);
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
			delete cameraController;
		}
		void ReceiveEvent(QEvent* event) {

			if (event == nullptr)
				return;

			const QEvent::Type t = event->type();
			if (t == QEvent::Resize) {
				QResizeEvent* e = static_cast<QResizeEvent*>(event);
				mSelf->Resize(e->size().width(), e->size().height());
			}
			else if (t == QEvent::MouseButtonPress) {
				QMouseEvent* e2 = static_cast<QMouseEvent*>(event);
				auto x = e2->x();
				auto y = e2->y();
				Qt::MouseButton mb = e2->button();
				if (mb == Qt::MouseButton::LeftButton) {
					cameraController->mouseLeftPress(x, y);
				}
				else if (mb == Qt::MouseButton::MiddleButton) {
					cameraController->mouseMiddlePress(x, y);
				}
				else if (mb == Qt::MouseButton::RightButton)
				{
					cameraController->mouseRightPress(x, y);
				}
			}
			else if (t == QEvent::MouseMove) {
				QMouseEvent* e2 = static_cast<QMouseEvent*>(event);
				auto x = e2->x();
				auto y = e2->y();
				cameraController->mouseMove(x, y);
			}
			else if (t == QEvent::MouseButtonRelease) {
				QMouseEvent* e2 = static_cast<QMouseEvent*>(event);
				auto x = e2->x();
				auto y = e2->y();
				Qt::MouseButton mb = e2->button();
				if (mb == Qt::MouseButton::LeftButton) {
					//PathTrace::CameraController::Instance().mousele;
				}
				else if (mb == Qt::MouseButton::MiddleButton) {
					cameraController->mouseMiddleRelease(0, 0);
				}
				else if (mb == Qt::MouseButton::RightButton)
				{
					cameraController->mouseRightRelease(0, 0);
				}
			}
			else if (t == QEvent::Wheel) {
				QWheelEvent* e2 = static_cast<QWheelEvent*>(event);
				cameraController->wheelMouseWheel(e2->angleDelta().y());
			}

		}
	private:
		friend CameraController;
		PathTraceRender* mSelf = nullptr;
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
		RenderOptions renderOptions;
		CameraController* cameraController = nullptr;
	};


	PathTraceRender& PathTraceRender::instance()
	{
		static PathTraceRender render;
		return render;
	}
	PathTraceRender::PathTraceRender() : mInternal(new PathTraceRenderInternal(this)) {

	}

	PathTraceRender::~PathTraceRender() {
		delete mInternal;
	}

	Scene* PathTraceRender::GetScene()
	{
		return mInternal->GetScene();
	}

	Renderer* PathTraceRender::GetRenderer()
	{
		return mInternal->GetRenderer();
	}

	void PathTraceRender::GetSceneFiles()
	{
		mInternal->GetSceneFiles();
	}

	void PathTraceRender::Update()
	{
		mInternal->Update();
	}

	RenderOptions& PathTraceRender::GetRenderOptions()
	{
		return mInternal->GetRenderOptions();
	}

	void PathTraceRender::GetEnvMaps()
	{
		mInternal->GetEnvMaps();
	}

	void PathTraceRender::Resize(int width, int height)
	{
		mInternal->Resize(width, height);
	}

	void PathTraceRender::onSwitchScene(std::string sceneName)
	{
		mInternal->SwitchScene(sceneName);
	}

	std::string PathTraceRender::GetSceneFilePath()
	{
		return mInternal->GetSceneFilePath();
	}

	void PathTraceRender::LoadScene(std::string sceneName)
	{
		mInternal->LoadScene(sceneName);
	}

	void PathTraceRender::LoadDefaultScene()
	{
		mInternal->LoadDefaultScene();
	}

	void PathTraceRender::Render()
	{
		mInternal->GetRenderer()->Render();
	}

	void PathTraceRender::Present()
	{
		mInternal->GetRenderer()->Present();
	}
	void PathTraceRender::ReceiveEvent(QEvent* event)
	{
		mInternal->ReceiveEvent(event);

	}
	void PathTraceRender::TraceScene()
	{
		mInternal->TraceScene();
	}

	bool PathTraceRender::InitRenderer()
	{
		return mInternal->InitRenderer();
	}

	void PathTraceRender::Ret()
	{
		mInternal->Ret();
	}

	void PathTraceRender::Destory()
	{

	}
	CameraController::CameraController(PathTraceRender* ptr) :render(ptr)
	{
	}


	void CameraController::mouseMove(int _x, int _y)
	{
		if (mouseMiddle) {
			render->GetScene()->getCamera()->Strafe((_x - tx) * 0.1, (_y - ty) * 0.1);
			render->GetScene()->setDirty(true);
		}
		else if (mouseRight)
		{

			render->GetScene()->getCamera()->OffsetRotateByScreen((_x - rx) * 0.5, (ry - _y) * 0.5);
			render->GetScene()->setDirty(true);
		}
		//x = _x;
		//y = _y;
	}

	void CameraController::mouseLeftPress(int x, int y)
	{
		Vec3 p;
		if (render->GetScene()->IntersectionByScreen(1.0 * x / render->GetRenderOptions().windowResolution.x, 1.0 - 1.0 * y / render->GetRenderOptions().windowResolution.y, p)) {
			cameraDestinations.push_back(p);
		}
	}

	void CameraController::mouseMiddlePress(int x, int y)
	{
		this->tx = x;
		this->ty = y;
		render->GetScene()->getCamera()->UpdateCamera();
		mouseMiddle = true;
	}

	void CameraController::mouseRightPress(int x, int y)
	{
		this->rx = x;
		this->ry = y;
		render->GetScene()->getCamera()->UpdateCamera();
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

		float dt = delta * 0.000025 * render->GetScene()->getBBox().diagonalDistance();
		render->GetScene()->getCamera()->OffsetRadius(dt);
		render->GetScene()->setDirty(true);
	}
	void CameraController::GetCameraPosition(float eye[3]) {
		Vec3 p = render->GetScene()->getCamera()->GetEye();
		eye[0] = p.x;
		eye[1] = p.y;
		eye[2] = p.z;
	}
	void CameraController::GetViewProject(float view[16], float proj[16]) {
		render->GetRenderer()->GetProgress();
		render->GetScene()->getCamera()->ComputeViewProjectionMatrix(view, proj, 1.0f * render->GetRenderOptions().windowResolution.x / render->GetRenderOptions().windowResolution.y);
	}
	void CameraController::MoveToPivot(float deltaTime) {
		if (!cameraDestinations.empty()) {
			while (cameraDestinations.size() != 1) {
				cameraDestinations.pop_back();
			}
			float t = 5.0f * deltaTime;
			auto& destion = cameraDestinations[0];
			auto& piviot = render->GetScene()->getCamera()->GetPivoit();
			if (Vec3::Length(destion - piviot) < 0.03f) {
				render->GetScene()->getCamera()->setPivot(destion);
				cameraDestinations.pop_back();
			}
			else
			{
				render->GetScene()->getCamera()->setPivot(destion * t + (1 - t) * piviot);
			}
			render->GetScene()->setDirty(true);
		}
	}
	void CameraController::PustCameraDestination(float x, float y, float z) {
		cameraDestinations.push_back({ x,y,z });
	}
}