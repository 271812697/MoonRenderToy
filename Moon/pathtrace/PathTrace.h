#pragma once
#include <string>
#include <vector>
using namespace std;
class QEvent;
namespace PathTrace {
	class Scene;
	class Renderer;
	class RenderOptions;

	class PathTraceRender {
	public:
		static PathTraceRender& instance();
		Scene* GetScene();
		Renderer* GetRenderer();
		void GetSceneFiles();
		void Update();
		RenderOptions& GetRenderOptions();
		void GetEnvMaps();
		void Resize(int width, int height);
		void onSwitchScene(std::string sceneName);
		std::string GetSceneFilePath();
		void LoadScene(std::string sceneName);
		void LoadDefaultScene();
		void Render();
		void Present();
		void ReceiveEvent(QEvent* e);
		void TraceScene();

		bool InitRenderer();

		void Ret();
		void Destory();
		~PathTraceRender();
	private:
		PathTraceRender();
	private:
		class PathTraceRenderInternal;
		PathTraceRenderInternal* mInternal = nullptr;
	};
}