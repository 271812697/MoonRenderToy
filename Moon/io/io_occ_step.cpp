#include "io_occ_step.h"
#include "TopoShape.h"
#include "Core/SceneSystem/Scene.h"
#include "core/component/TopoShapeActor.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/Context.h"
#include "renderer/AView.h"
#include "editor/View/sceneview/viewerwidget.h"
#include "Core/ECS/Components/CMaterialRenderer.h"
#include "Core/ECS/Components/CBatchMeshTriangle.h"
#include "Core/ECS/Components/CBatchMeshLine.h"
#include "core/component/CTopoShape.h"
#include "Core/ResourceManagement/ModelManager.h"
#include "Gizmo/Gizmo.h"
#include <TopoDS_Shape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly_Triangulation.hxx>
#include <TopLoc_Location.hxx>
#include <BRep_Tool.hxx>  // 关键：包含 BRep_Tool 的定义
#include <BRepLib_FindSurface.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <gp_Dir.hxx>
#include <Geom_Surface.hxx>  // 必须包含
#include <gp_Vec.hxx>
#include <Precision.hxx>
#include <Geom_Plane.hxx>          // 具体曲面类 按需添加
#include <fstream>
#include <filesystem>

namespace MOON {
    // 读取 STEP 模型并返回其形状
    namespace IO {
        void ReadSTEP(const char* filePath, Core::SceneSystem::Scene* scene) {
            auto topoActor = new Core::ECS::TopoActor(scene->GetAvailableID(), "TopoShape", "TopoShape", false);
            scene->AddActor(topoActor);
            const auto& topoComp=topoActor->GetComponent<Core::ECS::Components::CTopoShape>();
            Part::TopoShape& topo= topoComp->GetTopoShape();
            topo.importStep(filePath);
            topoComp->discretizationShape();
        }
    }
}