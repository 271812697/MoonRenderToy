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
#include <gp_Vec.hxx>
#include <Precision.hxx>

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
            BRepMesh_IncrementalMesh mesher(face, 0.1); // 公差0.1，可根据需要降低
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
            vertices.reserve(nbNodes);
            for (Standard_Integer i = 1; i <= nbNodes; ++i) {
                gp_Pnt p = triangulation->Node(i);  // 获取原始顶点
                p.Transform(loc.Transformation());  // 应用位置变换
                vertices.push_back(p);
            }

            // 4. 提取三角形索引（OCC索引从1开始，需转换为0基索引）
            const Poly_Array1OfTriangle& triangles = triangulation->Triangles();
            indices.reserve((triangles.Upper() - triangles.Lower() + 1) * 3);
            for (int i = triangles.Lower(); i <= triangles.Upper(); ++i) {
                Poly_Triangle tri = triangles(i);
                Standard_Integer v1, v2, v3;
                tri.Get(v1, v2, v3);
                // 转换为0基索引并添加到索引数组
                indices.push_back(static_cast<unsigned int>(v1 - 1));
                indices.push_back(static_cast<unsigned int>(v2 - 1));
                indices.push_back(static_cast<unsigned int>(v3 - 1));
            }
        }

        // 计算顶点法线：通过遍历三角形，按三角形法线累计到顶点，然后归一化（平滑法线）
        static void ComputeNormals(const std::vector<gp_Pnt>& vertices,
            const std::vector<unsigned int>& indices,
            std::vector<gp_Dir>& outNormals) {
            size_t vcount = vertices.size();
            outNormals.clear();
            if (vcount == 0) return;

            std::vector<gp_Vec> accum(vcount, gp_Vec(0.0, 0.0, 0.0));

            for (size_t i = 0; i + 2 < indices.size(); i += 3) {
                unsigned int ia = indices[i + 0];
                unsigned int ib = indices[i + 1];
                unsigned int ic = indices[i + 2];
                if (ia >= vcount || ib >= vcount || ic >= vcount) continue;

                const gp_Pnt& pa = vertices[ia];
                const gp_Pnt& pb = vertices[ib];
                const gp_Pnt& pc = vertices[ic];

                gp_Vec v1(pb.X() - pa.X(), pb.Y() - pa.Y(), pb.Z() - pa.Z());
                gp_Vec v2(pc.X() - pa.X(), pc.Y() - pa.Y(), pc.Z() - pa.Z());
                gp_Vec n = v1.Crossed(v2);

                double mag = n.Magnitude();
                if (mag > Precision::Confusion()) {
                    n = n / mag; // 单位化三角面法线
                    accum[ia] += n;
                    accum[ib] += n;
                    accum[ic] += n;
                }
            }

            outNormals.resize(vcount);
            for (size_t i = 0; i < vcount; ++i) {
                double m = accum[i].Magnitude();
                if (m > Precision::Confusion()) {
                    gp_Vec v = accum[i] / m;
                    outNormals[i] = gp_Dir(v);
                }
                else {
                    // 退化情况，给默认法线
                    outNormals[i] = gp_Dir(0, 0, 1);
                }
            }
        }

        // 简单平面投影 UV（用于可视化）：基于法线选定投影平面，然后把顶点投影到局部坐标并归一化到 [0,1]
        static void GeneratePlanarUVs(const std::vector<gp_Pnt>& vertices,
            const std::vector<gp_Dir>& normals,
            std::vector<OvMaths::FVector2>& outUVs) {
            outUVs.clear();
            if (vertices.empty()) return;

            // 1. 计算中心点
            gp_Vec center(0, 0, 0);
            for (const auto& p : vertices) center += gp_Vec(p.X(), p.Y(), p.Z());
            center /= static_cast<double>(vertices.size());
            gp_Pnt centerP(center.X(), center.Y(), center.Z());

            // 2. 计算平均法线（作为投影平面法线）
            gp_Vec avgN(0, 0, 0);
            for (const auto& n : normals) avgN += gp_Vec(n.X(), n.Y(), n.Z());
            if (avgN.Magnitude() < Precision::Confusion()) avgN = gp_Vec(0, 0, 1);
            avgN.Normalize();
            gp_Vec ref(0, 0, 1);
            if (std::fabs(avgN.Dot(ref)) > 0.9) ref = gp_Vec(0, 1, 0); // 若接近同向，改参考向量

            gp_Vec axisU = avgN.Crossed(ref);
            if (axisU.Magnitude() < Precision::Confusion()) axisU = gp_Vec(1, 0, 0);
            axisU.Normalize();

            gp_Vec axisV = avgN.Crossed(axisU);
            axisV.Normalize();

            // 3. 计算投影坐标并找 min/max
            double minU = 1e308, maxU = -1e308, minV = 1e308, maxV = -1e308;
            std::vector<std::pair<double, double>> tmp;
            tmp.reserve(vertices.size());
            for (const auto& p : vertices) {
                gp_Vec rel(p.X() - centerP.X(), p.Y() - centerP.Y(), p.Z() - centerP.Z());
                double u = rel.Dot(axisU);
                double v = rel.Dot(axisV);
                tmp.emplace_back(u, v);
                minU = std::min(minU, u); maxU = std::max(maxU, u);
                minV = std::min(minV, v); maxV = std::max(maxV, v);
            }

            double spanU = maxU - minU;
            double spanV = maxV - minV;
            if (spanU < 1e-9) spanU = 1.0;
            if (spanV < 1e-9) spanV = 1.0;

            outUVs.resize(tmp.size());
            for (size_t i = 0; i < tmp.size(); ++i) {
                float uu = static_cast<float>((tmp[i].first - minU) / spanU);
                float vv = static_cast<float>((tmp[i].second - minV) / spanV);
                outUVs[i] = OvMaths::FVector2{ uu, vv };
            }
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
            CollectFaces(shape, faces);

            int i = 0;
            for (auto& f : faces) {
                i++;

                std::string faceName = std::string(filePath) + std::string("_face_") + std::to_string(i);
                std::vector<gp_Pnt> vertices;
                std::vector<unsigned int> indices;
                DiscretizeFace(f, vertices, indices);

                if (vertices.empty() || indices.empty()) {
                    continue;
                }

                // 计算并转换法线
                std::vector<gp_Dir> occNormals;
                ComputeNormals(vertices, indices, occNormals);

                // 转换为引擎类型：positions, normals, uvs
                std::vector<OvMaths::FVector3> positions;
                positions.reserve(vertices.size());
                for (const auto& p : vertices) positions.push_back(OvMaths::FVector3{ (float)p.X(), (float)p.Y(), (float)p.Z() });

                std::vector<OvMaths::FVector3> normals;
                normals.reserve(occNormals.size());
                for (const auto& n : occNormals) normals.push_back(OvMaths::FVector3{ (float)n.X(), (float)n.Y(), (float)n.Z() });

                // 生成简单的平面 UV（可选）
                std::vector<OvMaths::FVector2> uvs;
                GeneratePlanarUVs(vertices, occNormals, uvs);

                // 使用 ModelManager 把 mesh 注册到资源系统（使用与 glTF 加载处相同的重载）
                auto model = OvCore::Global::ServiceLocator::Get<OvCore::ResourceManagement::ModelManager>().LoadFromMemory(faceName, positions, normals, uvs, indices);

                // 创建并注册默认材质
                OvCore::Resources::Material* tempMat = new OvCore::Resources::Material();
                OvCore::Global::ServiceLocator::Get<OvCore::ResourceManagement::MaterialManager>().RegisterResource(faceName, tempMat);
                tempMat->SetBackfaceCulling(false);
                tempMat->SetCastShadows(false);
                tempMat->SetReceiveShadows(false);

                tempMat->SetShader(OvCore::Global::ServiceLocator::Get<OvEditor::Core::Context>().shaderManager[":Shaders\\Standard.ovfx"]);
                tempMat->SetProperty("u_Albedo", OvMaths::FVector4{ 1.0f, 1.0f, 1.0f, 1.0f });

                tempMat->SetProperty("u_AlphaClippingThreshold", 1.0f);
                tempMat->SetProperty("u_Roughness", 0.1f);
                tempMat->SetProperty("u_Metallic", 0.1f);
                // Emission
                tempMat->SetProperty("u_EmissiveIntensity", 1.0f);
                tempMat->SetProperty("u_EmissiveColor", OvMaths::FVector3{ 0.0f, 0.0f, 0.0f });

                // 在场景中创建 Actor 并绑定模型/材质
                auto& actor = scene->CreateActor();
                actor.AddComponent<OvCore::ECS::Components::CModelRenderer>().SetModel(model);
                auto& materilaRener = actor.AddComponent<OvCore::ECS::Components::CMaterialRenderer>();
                materilaRener.SetMaterialAtIndex(0, *tempMat);
                materilaRener.UpdateMaterialList();
            }
        }
    }
}