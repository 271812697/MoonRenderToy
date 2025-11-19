#pragma once
#include <string>
namespace Core {
	namespace SceneSystem {
		class Scene;
	}
}
namespace PathTrace
{
	struct Mat4;
	class Scene;
	class RenderOptions;
	bool LoadGLTF(const std::string& filename, Scene* scene, RenderOptions& renderOptions, Mat4 xform, bool binary);
	bool LoadSceneFromFile(const std::string& filename, Scene* scene, RenderOptions& renderOptions);
	bool LoadSingleModel(const std::string& filename,Scene* scene);

	bool LoadSceneFromFile(const std::string& filename, Core::SceneSystem::Scene* scene);
	bool LoadGLTF(const std::string& filename, Core::SceneSystem::Scene* scene, Mat4 xform, bool binary);
}