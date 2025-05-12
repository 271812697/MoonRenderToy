#pragma once
#include <string>
#include <vector>

using namespace std;
namespace PathTrace {
	class Scene;
	class Renderer;
	class RenderOptions;
	extern int sampleSceneIdx;
	extern std::vector<string> sceneFiles;
	extern int envMapIdx;
	extern std::vector<string> envMaps;
	extern int selectedInstance;
	extern int selectedMat;

	extern float CameraMoveSpeed;
	extern bool objectPropChanged;
	extern bool showTransform;
	extern bool space_down;
	extern float screenX[2];
	extern float screenY[2];

	Scene* GetScene();
	Renderer* GetRenderer();
	void GetSceneFiles();
	void Update();

	RenderOptions& GetRenderOptions();
	void GetEnvMaps();
	void Resize(int width, int height);
	void SwitchScene(std::string sceneName);
	void LoadScene(std::string sceneName);

	void TraceScene();

	bool InitRenderer();

	void Ret();

	class CameraController {
		CameraController();
	public:
		static CameraController& Instance();
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
		void PustCameraDestination(float x,float y,float z);
	private:
		//for rotate
		int rx, ry;
		//for translate
		int tx, ty;
		bool mouseMiddle = false;

		bool mouseRight = false;
	};
}