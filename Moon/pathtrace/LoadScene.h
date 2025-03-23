#pragma once

#include <string>
namespace PathTrace
{
	struct Mat4;
	class Scene;
	class RenderOptions;
	bool LoadGLTF(const std::string& filename, Scene* scene, RenderOptions& renderOptions, Mat4 xform, bool binary);
	bool LoadSceneFromFile(const std::string& filename, Scene* scene, RenderOptions& renderOptions);
}