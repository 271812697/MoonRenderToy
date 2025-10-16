#include "io_occ_step.h"
#include "OvCore/SceneSystem/Scene.h"
#include "OvCore/Global/ServiceLocator.h"
#include "renderer/Context.h"
#include "OvCore/ECS/Components/CMaterialRenderer.h"
#include "OvCore/ResourceManagement/ModelManager.h"
#include <STEPControl_Reader.hxx>
#include <TopoDS_Shape.hxx>
#include <TCollection_AsciiString.hxx>
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

#include <Geom_Plane.hxx>          // 具体曲面类（按需添加）

namespace MOON {
	// 读取 STEP 模型并返回其形状
	namespace IO {
        // 递归遍历形状，收集所有面
        static void CollectFaces(const TopoDS_Shape& shape, std::vector<TopoDS_Face>& faces) {
            TopExp_Explorer exp;
            // 遍历所有面（TopAbs_FACE 表示只找面类型）
            for (exp.Init(shape, TopAbs_FACE); exp.More(); exp.Next()) {
                const TopoDS_Shape& subShape = exp.Current();
                if (subShape.ShapeType() == TopAbs_FACE) {
                    faces.push_back(TopoDS::Face(subShape)); // 转换为面并存储
                }
            }
        }
        // 离散化面为三角形网格，并提取顶点和索引
        static void DiscretizeFace(const TopoDS_Face& face,
            std::vector<gp_Pnt>& vertices,  // 顶点坐标（OCC的3D点）
            std::vector<unsigned int>& indices) { // 三角形索引
            // 1. 离散化面：设置线性公差（越小精度越高，性能消耗越大）
            BRepMesh_IncrementalMesh mesher(face, 0.1); // 公差0.1
            if (!mesher.IsDone()) {
                std::cerr << "面离散化失败" << std::endl;
                return;
            }

            // 2. 获取离散后的三角形网格
            TopLoc_Location loc;
            const Handle(Poly_Triangulation)& triangulation = BRep_Tool::Triangulation(face, loc);
            if (triangulation.IsNull()) {
                std::cerr << "面没有三角形网格数据" << std::endl;
                return;
            }

            // 3. 提取顶点（需考虑位置变换 loc）
            const Standard_Integer nbNodes = triangulation->NbNodes();
            for (Standard_Integer i = 1; i <= nbNodes; ++i) {
                gp_Pnt p = triangulation->Node(i);  // 获取原始顶点变换
                p.Transform(loc.Transformation());  // 应用位置变换
                vertices.push_back(p);
            }

            // 4. 提取三角形索引（OCC索引从1开始，需转换为0基索引）
            const Poly_Array1OfTriangle& triangles = triangulation->Triangles();
            for (int i = triangles.Lower(); i <= triangles.Upper(); ++i) {
                Poly_Triangle tri = triangles(i);
                Standard_Integer v1, v2, v3;
                tri.Get(v1, v2, v3);
                // 转换为0基索引并添加到索引数组
                indices.push_back(v1 - 1);
                indices.push_back(v2 - 1);
                indices.push_back(v3 - 1);
            }
        }
        // 计算面的顶点法向量（基于 DynamicType() 判断曲面类型）
        static void ComputeNormals(const TopoDS_Face& face,
            const std::vector<gp_Pnt>& vertices,
            std::vector<gp_Dir>& normals) {
      
        }

		void ReadSTEP(const char* filePath, OvCore::SceneSystem::Scene* scene) {
            // 1. 创建 STEP 读取器
            STEPControl_Reader reader;

            // 2. 读取 STEP 文件
            TCollection_AsciiString fileName(filePath);
            IFSelect_ReturnStatus status = reader.ReadFile(fileName.ToCString());

            // 检查读取是否成功
            if (status != IFSelect_RetDone) {
                std::cerr << "无法读取 STEP 文件: " << filePath << std::endl;
                return; // 返回空形状
            }

            // 3. 转换所有根实体（将 STEP 数据转换为 OCC 内部形状）
            int numRoots = reader.NbRootsForTransfer();
            std::cout << "找到 " << numRoots << " 个根实体" << std::endl;

            reader.TransferRoots(); // 转换所有根实体

            // 4. 获取转换后的形状（若有多个根实体，可循环获取）
            TopoDS_Shape shape;
            if (reader.NbShapes() > 0) {
                shape = reader.Shape(1); // 获取第一个形状（索引从 1 开始）
                std::cout << "成功读取 STEP 模型" << std::endl;
            }
            else {
                std::cerr << "STEP 文件中未找到有效形状" << std::endl;
            }
            std::vector<TopoDS_Face> faces;
            CollectFaces(shape,faces);
            int i = 0;
            for (auto& f : faces) {
                i++;

				std::string faceName = filePath + std::string("_face_") + std::to_string(i);
                std::vector<gp_Pnt> vertices;
                std::vector<unsigned int> indices;
				DiscretizeFace(f, vertices, indices);
                auto model=OvCore::Global::ServiceLocator::Get<OvCore::ResourceManagement::ModelManager>().LoadFromMemory(faceName, 
                    [&vertices]() {
                        std::vector<OvMaths::FVector3> verts;
                        for (auto& v : vertices) {
                            verts.push_back(OvMaths::FVector3(v.X(), v.Y(), v.Z()));
                        }
                        return verts;
                    }(),indices);
                OvCore::Resources::Material* tempMat = new OvCore::Resources::Material();
                OvCore::Global::ServiceLocator::Get<OvCore::ResourceManagement::MaterialManager>().RegisterResource(faceName, tempMat);
                tempMat->SetBackfaceCulling(false);;
                tempMat->SetCastShadows(false);
                tempMat->SetReceiveShadows(false);

                tempMat->SetShader(OvCore::Global::ServiceLocator::Get<OvEditor::Core::Context>().shaderManager[":Shaders\\Standard.ovfx"]);
                tempMat->SetProperty("u_Albedo", OvMaths::FVector4{ 1.0, 1.0, 1.0, 1.0 });

                tempMat->SetProperty("u_AlphaClippingThreshold", 1.0f);
                tempMat->SetProperty("u_Roughness", 0.1f);
                tempMat->SetProperty("u_Metallic", 0.1f);
                // Emission
                tempMat->SetProperty("u_EmissiveIntensity", 1.0f);
                tempMat->SetProperty("u_EmissiveColor", OvMaths::FVector3{ 0.0f,0.0f,0.0f });

                auto& actor = scene->CreateActor();
                actor.AddComponent<OvCore::ECS::Components::CModelRenderer>().SetModel(model);

                //actor.GetComponent<OvCore::ECS::Components::CTransform>()->SetMatrix(xform.data);
                auto& materilaRener = actor.AddComponent<OvCore::ECS::Components::CMaterialRenderer>();
                materilaRener.SetMaterialAtIndex(0, *tempMat);
                materilaRener.UpdateMaterialList();

            }
           
		}
	}
}
