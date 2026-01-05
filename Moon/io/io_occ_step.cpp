#include "io_occ_step.h"
#include "Geomerty/TopoShape.h"
#include "Core/SceneSystem/Scene.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/Context.h"
#include "Core/ECS/Components/CMaterialRenderer.h"
#include "Core/ECS/Components/CBatchMeshTriangle.h"
#include "Core/ECS/Components/CBatchMeshLine.h"
#include "Core/ResourceManagement/ModelManager.h"
#include "Gizmo/Gizmo.h"
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
#include <Geom_Plane.hxx>          // 具体曲面类 按需添加
#include <fstream>
#include <filesystem>

namespace MOON {
    // 读取 STEP 模型并返回其形状
    namespace IO {
        void ReadSTEP(const char* filePath, Core::SceneSystem::Scene* scene) {

            std::vector<Domain> domains;
            std::vector<Vector3d>linePoints;
            std::vector<Line>LineRanges;

            std::filesystem::directory_entry p_directory(filePath);
            std::string path = p_directory.path().parent_path().string();
            std::string name = p_directory.path().filename().string();
            name = name.substr(0, name.rfind("."));
            std::string fileExt = p_directory.path().extension().string();
            std::string cachePath = path + "/" + name + ".bstp";
            if (!std::filesystem::exists(cachePath)) {
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
                TopoShape topo(shape);
                topo.getDomainfaces(domains,1.0);
                topo.getLines(linePoints, LineRanges, 1.0);
               
                std::ofstream  f(cachePath);
                f << domains.size() << std::endl;;
                for (int i = 0; i < domains.size(); i++) {
                    f << domains[i].points.size() << std::endl;
                    for (int k = 0; k < domains[i].points.size(); k++) {
                        f << domains[i].points[k].x() << " " << domains[i].points[k].y() << " " << domains[i].points[k].z() << std::endl;
                    }
                    f << domains[i].facets.size() << std::endl;
                    for (int k = 0; k < domains[i].facets.size(); k++) {
                        f << domains[i].facets[k].I1 << " " << domains[i].facets[k].I2 << " " << domains[i].facets[k].I3 << " " << std::endl;
                    }
                }
                f << linePoints.size() << std::endl;
                for (int i = 0; i < linePoints.size(); i++) {
                    f << linePoints[i].x() << " " << linePoints[i].y() << " " << linePoints[i].z() << std::endl;
                }
                f << LineRanges.size() << std::endl;;
                for (int i = 0; i < LineRanges.size(); i++) {
                    f << LineRanges[i].I1 << " " << LineRanges[i].I2 << std::endl;
                }
                f.close();
            }
            else
            {
                int domainSize;
                std::ifstream  f(cachePath);
                f >> domainSize;
                domains.resize(domainSize);
                for (int i = 0; i < domainSize; i++) {
                    int pointSize;
                    f >> pointSize;
                    domains[i].points.reserve(pointSize);
                    double x, y, z;
                    for (int k = 0; k < pointSize; k++) {
                        f >> x >> y >> z;
                        domains[i].points.emplace_back(x,y,z);
                    }
                    int faceSize;
                    f >> faceSize;
                    domains[i].facets.reserve(faceSize);
                    uint32_t I1, I2, I3;
                    for (int k = 0; k < faceSize; k++) {
                        f >> I1 >> I2 >> I3;
                        domains[i].facets.emplace_back( I1, I2, I3 );
                    }
                }
                int linePointSize;
                int lineRangeSize;
                f >>linePointSize;
                linePoints.reserve(linePointSize);
                double x, y, z;
                for (int i = 0; i < linePointSize; i++) {
                    f >> x >> y >> z;
                    linePoints.emplace_back(x,y,z);

                }
                f >> lineRangeSize;
                uint32_t I1, I2;
                LineRanges.reserve(lineRangeSize);
                for (int i = 0; i < lineRangeSize; i++) {
                    f >> I1 >> I2;
                    LineRanges.emplace_back(I1, I2);
                }
                f.close();

            }
            static Maths::FVector4 colors[] = {
                 { 140.0 / 255.0f, 180.0f / 255.0f, 216.0f / 255.0f, 1.0f }, { 237.0 / 255.0f, 28.0f / 255.0f,36.0f / 255.0f, 1.0f },
                 { 0.0 / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f, 1.0f }, { 0.0 / 255.0f, 162.0f / 255.0f,232.0f / 255.0f, 1.0f },
                  { 112.0 / 255.0f, 146.0f / 255.0f, 190.0f / 255.0f, 1.0f }, { 255.0 / 255.0f, 0.0f / 255.0f,255.0f / 255.0f, 1.0f },
                   { 0.0 / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 1.0f }, { 161.0 / 255.0f, 161.0f / 255.0f,255.0f / 255.0f, 1.0f },
                    { 171.0 / 255.0f, 128.0f / 255.0f, 84.0f / 255.0f, 1.0f }, { 255.0 / 255.0f, 128.0f / 255.0f,191.0f / 255.0f, 1.0f },
                     { 135.0 / 255.0f, 89.0f / 255.0f, 179.0f / 255.0f, 1.0f }, { 255.0 / 255.0f, 191.0f / 255.0f,128.0f / 255.0f, 1.0f }
             };
            std::vector<Maths::FVector3> positions;
            std::vector<Maths::FVector3> normals;
            std::vector<Maths::FVector2>uvs;
            std::vector<unsigned int>indices;

            std::vector<Maths::FVector4>domainColor;
            unsigned int vertexOffset = 0;
            unsigned int indexOffset = 0;
            int cnt = 0;
            std::vector<Core::ECS::Actor*>domainActors;
            std::vector<Rendering::Geometry::bbox>domainBoxs;
            std::vector<uint32_t>domainRange;
            int domainIndex = -1;
            for (int i = 0; i < domains.size(); i++) {
                if (domains[i].facets.size() > 0) {
                    domainIndex++;
                    domainColor.push_back(colors[cnt]);
                    cnt = (cnt + 1) % 12;
                    positions.reserve(positions.size() + domains[i].points.size());
                    normals.reserve(normals.size() + domains[i].points.size());
                    uvs.reserve(uvs.size() + domains[i].points.size());
                    indices.resize(indexOffset + domains[i].facets.size() * 3);
                    Rendering::Geometry::bbox subBox;
                    for (int k = 0; k < domains[i].points.size(); k++) {
                        positions.emplace_back(Maths::FVector3{ static_cast<float>(domains[i].points[k].x()),static_cast<float>(domains[i].points[k].y()),static_cast<float>(domains[i].points[k].z()) });
                        subBox.grow(positions.back());
                        //need to support in further
                        normals.emplace_back(Maths::FVector3{ 0,0,0 });
                        uvs.emplace_back(Maths::FVector2{ domainIndex * 1.0f,0.0f });
                    }
                    for (int k = 0; k < domains[i].facets.size(); k++) {
                        indices[indexOffset + 3 * k] = domains[i].facets[k].I1 + vertexOffset;
                        indices[indexOffset + 3 * k + 1] = domains[i].facets[k].I2 + vertexOffset;
                        indices[indexOffset + 3 * k + 2] = domains[i].facets[k].I3 + vertexOffset;;
                    }
                    vertexOffset = positions.size();
                    indexOffset = indices.size();
                    auto& actor = scene->CreateActor(std::to_string(i));
                    std::string registryName = filePath + std::to_string(i);
                    domainActors.push_back(&actor);

                    domainBoxs.push_back(subBox);
                    domainRange.push_back(indexOffset);
                }
            }
            auto model = Core::Global::ServiceLocator::Get<Core::ResourceManagement::ModelManager>().LoadFromMemory(filePath, positions, normals, uvs, indices);
            // 创建并注册默认材质
            Core::Resources::Material* tempMat = new Core::Resources::Material();
            Core::Global::ServiceLocator::Get<Core::ResourceManagement::MaterialManager>().RegisterResource(filePath, tempMat);
            tempMat->SetBackfaceCulling(false);
            tempMat->SetCastShadows(false);
            tempMat->SetReceiveShadows(false);
            tempMat->SetShader(Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager[":Shaders\\GeomertySurface.ovfx"]);
            tempMat->SetProperty("u_Albedo", colors[0]);
            tempMat->SetProperty("u_AlphaClippingThreshold", 1.0f);
            tempMat->SetProperty("u_Roughness", 0.3f);
            tempMat->SetProperty("u_Metallic", 0.1f);
            // Emission
            tempMat->SetProperty("u_EmissiveIntensity", 1.0f);
            tempMat->SetProperty("u_EmissiveColor", Maths::FVector3{ 0.0f, 0.0f, 0.0f });
            tempMat->AddFeature("WITH_EDGE");
            // 在场景中创建 Actor 并绑定模型/材质
            auto& actor = scene->CreateActor("RootFace", "Geomerty");
            actor.AddComponent<Core::ECS::Components::CModelRenderer>().SetModel(model);
            auto& materilaRener = actor.AddComponent<Core::ECS::Components::CMaterialRenderer>();
            materilaRener.SetMaterialAtIndex(0, *tempMat);
            materilaRener.UpdateMaterialList();
            for (auto* acptr : domainActors) {
                acptr->SetParent(actor);
            }
            //these two tex have leaks!!!!
            //domain colors Tex
            ::Rendering::Settings::TextureDesc desc;
            desc.isTextureBuffer = true;
            desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA32F;
            desc.buffetLen = domainColor.size() * sizeof(Maths::FVector4);
            desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
                .data = domainColor.data()
            };
            ::Rendering::HAL::GLTexture* domainColorTex = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_BUFFER);
            domainColorTex->Allocate(desc);
            tempMat->SetProperty("domainColorTex", domainColorTex);
            auto& bacthMesh = actor.AddComponent<Core::ECS::Components::CBatchMeshTriangle>();
            bacthMesh.SetColors(domainColor);
            bacthMesh.BuildBvh(domainBoxs, domainRange);


            //build lines
            std::vector<::Rendering::Geometry::Vertex> p_vertices;
            std::vector<uint32_t>lineIndex;
			std::vector<uint32_t>lineSegmentOffsets;
            std::vector<Maths::FVector4>lineColor;

            p_vertices.reserve(linePoints.size());
            for (int i = 0;i < linePoints.size();i++) {
                ::Rendering::Geometry::Vertex v;
                v.position[0] = static_cast<float>(linePoints[i].x());
                v.position[1] = static_cast<float>(linePoints[i].y());
                v.position[2] = static_cast<float>(linePoints[i].z());
                p_vertices.emplace_back(v);
            } 
            lineColor.reserve(LineRanges.size());
            for (int i = 0;i < LineRanges.size();i++) {
				auto& l = LineRanges[i];
                for (int k = l.I1; k <= l.I2 - 1; k++) {
                    p_vertices[k].texCoords[0] = i;
                    lineIndex.push_back(k);
                    lineIndex.push_back(k + 1);
                }
                p_vertices[l.I2].texCoords[0] = i;
				lineSegmentOffsets.push_back(lineIndex.size());
                lineColor.emplace_back(0,1,1,1);
            }
            //lineColor[575] = { 0,1,1,1 };
            auto lineMesh = new ::Rendering::Resources::Mesh(
                p_vertices,
                lineIndex,
                0,
                ::Rendering::Settings::EPrimitiveMode::LINES);
            auto lineModel = new ::Rendering::Resources::Model(filePath + std::string("_lineModel"));
            Core::Global::ServiceLocator::Get<Core::ResourceManagement::ModelManager>().RegisterResource(filePath + std::string("_lineModel"), lineModel);
            lineModel->GetMaterialNames().emplace_back("Default");
            lineModel->AddMesh(lineMesh);

            auto& lineActor = scene->CreateActor("RootLine", "GeomertyLine");
            lineActor.AddComponent<Core::ECS::Components::CModelRenderer>().SetModel(lineModel);
            auto& lineRener = lineActor.AddComponent<Core::ECS::Components::CMaterialRenderer>();

            auto lineMat = new Core::Resources::Material();
            Core::Global::ServiceLocator::Get<Core::ResourceManagement::MaterialManager>().RegisterResource(filePath + std::string("_lineMat"), lineMat);
            lineMat->SetShader(Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager[":Shaders\\GeomertyLine.ovfx"]);
            lineMat->SetBackfaceCulling(false);
            lineMat->SetCastShadows(false);
            lineMat->SetReceiveShadows(false);
            lineMat->SetLineWidth(2.0);
          
            desc.buffetLen = lineColor.size() * sizeof(Maths::FVector4);
            desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
                .data = lineColor.data()
            };

            ::Rendering::HAL::GLTexture* lineColorTex = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_BUFFER);
            lineColorTex->Allocate(desc);
            lineMat->SetProperty("lineColorTex", lineColorTex);
            lineRener.SetMaterialAtIndex(0, *lineMat);

            lineRener.UpdateMaterialList();
            auto& lineBacthMesh =lineActor.AddComponent<Core::ECS::Components::CBatchMeshLine>();
            lineBacthMesh.SetColors(lineColor);
            lineBacthMesh.BuildBvh(lineSegmentOffsets);
        }
    }
}