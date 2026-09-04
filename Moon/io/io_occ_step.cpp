#include <tracy/Tracy.hpp>
#include "TopoShape.h"
#include "Core/SceneSystem/Scene.h"
#include "feature/Feature.h"
#include "core/component/CTopoShape.h"
#include "core/JobSystem.h"
namespace MOON {
    // 读取 STEP 模型并返回其形状
    namespace IO {
        void ReadSTEP(const char* filePath, Core::SceneSystem::Scene* scene) {
            auto topoActor = new Feature3D("Feature", "Feature");
         
            Core::ECS::Components::CTopoShape* topoComp=topoActor->GetComponent<Core::ECS::Components::CTopoShape>();
            static MOON::System::JobSystem::Context ctx;
            static std::string path;
            path = filePath;
	        auto lamda=[=](JobDispatchArgs arg) { 
                ZoneScopedN("ReadSTEP");
                Part::TopoShape& topo = topoComp->GetTopoShape();
                topo.importStep(path.c_str());
                topoActor->makeDone();
		    };
	        MOON::System::JobSystem::Execute(ctx,lamda);
        }
    }
}