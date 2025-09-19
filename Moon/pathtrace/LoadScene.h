#pragma once
#include <string>
namespace OvCore {
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
	bool LoadSceneFromFile(const std::string& filename, OvCore::SceneSystem::Scene* scene);
	bool LoadGLTF(const std::string& filename, OvCore::SceneSystem::Scene* scene, Mat4 xform, bool binary);
}