#pragma once
namespace OvCore {
	namespace SceneSystem {
		class Scene;
	}
}
namespace MOON {
// 读取 STEP 模型并返回其形状
	namespace IO {
       void ReadSTEP(const char* filePath,OvCore::SceneSystem::Scene* scene);
	}
}

