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

	RenderOptions& GetRenderOptions();
	void GetEnvMaps();
	void Resize(int width, int height);
	void LoadScene(std::string sceneName);

	bool InitRenderer();

	void Ret();

	class CameraController {
		CameraController();
	public:
		static CameraController& Instance();
		void mouseMove(int _x, int _y);
		void mouseLeftPress(int x, int y);
		void mouseMiddlePress();
		void mouseRightPress();
		void mouseMiddleRelease();
		void mouseRightRelease();
		void wheelMouseWheel(float delta);
	private:
		int x, y;
		bool mouseMiddle = false;

		bool mouseRight = false;
	};
}