#include "io_occ_step.h"
#include "Geomerty/TopoShape.h"
#include "Core/SceneSystem/Scene.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/Context.h"
#include "Core/ECS/Components/CMaterialRenderer.h"
#include "Core/ECS/Components/CColorBar.h"
#include "Core/ResourceManagement/ModelManager.h"
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

namespace MOON {
    // 读取 STEP 模型并返回其形状
    namespace IO {
        void ReadSTEP(const char* filePath, Core::SceneSystem::Scene* scene) {
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
            std::vector<Domain> domains;
            static Maths::FVector4 colors[] = {
                { 140.0 / 255.0f, 180.0f / 255.0f, 216.0f / 255.0f, 1.0f }, { 237.0 / 255.0f, 28.0f / 255.0f,36.0f / 255.0f, 1.0f },
                { 0.0 / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f, 1.0f }, { 0.0 / 255.0f, 162.0f / 255.0f,232.0f / 255.0f, 1.0f },
                 { 112.0 / 255.0f, 146.0f / 255.0f, 190.0f / 255.0f, 1.0f }, { 255.0 / 255.0f, 0.0f / 255.0f,255.0f / 255.0f, 1.0f },
                  { 0.0 / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 1.0f }, { 161.0 / 255.0f, 161.0f / 255.0f,255.0f / 255.0f, 1.0f },
                   { 171.0 / 255.0f, 128.0f / 255.0f, 84.0f / 255.0f, 1.0f }, { 255.0 / 255.0f, 128.0f / 255.0f,191.0f / 255.0f, 1.0f },
                    { 135.0 / 255.0f, 89.0f / 255.0f, 179.0f / 255.0f, 1.0f }, { 255.0 / 255.0f, 191.0f / 255.0f,128.0f / 255.0f, 1.0f }
            };
            topo.getDomainfaces(domains,1.0);
            std::vector<Maths::FVector3> positions; 
            std::vector<Maths::FVector3> normals;
            std::vector<Maths::FVector2>uvs;
            std::vector<unsigned int>indices;
            struct Range
            {
                unsigned int upbound;
            };
            std::vector<Maths::FVector4>domainColor;
			unsigned int vertexOffset = 0;
            unsigned int indexOffset = 0;
            int cnt = 0;
            std::vector<Core::ECS::Actor*>domainActors;
            for (int i = 0;i < domains.size();i++) {
				domainColor.push_back(colors[cnt]);  
                cnt = (cnt + 1) % 12;
                positions.reserve(positions.size()+domains[i].points.size());
                normals.reserve(normals.size()+domains[i].normals.size());
                uvs.reserve(uvs.size()+ domains[i].points.size());
                indices.resize(indexOffset +domains[i].facets.size()*3);
                for (int k = 0;k < domains[i].points.size();k++) {
                    positions.emplace_back(Maths::FVector3{ static_cast<float>(domains[i].points[k].x()),static_cast<float>(domains[i].points[k].y()),static_cast<float>(domains[i].points[k].z()) });
					//need to support in further
                    normals.emplace_back(Maths::FVector3{ 0,0,0 });
                    uvs.emplace_back(Maths::FVector2{ i*1.0f,0.0f });
                }
                for (int k = 0;k < domains[i].facets.size();k++) {
                    indices[indexOffset +3 * k] = domains[i].facets[k].I1+vertexOffset;
                    indices[indexOffset+3 * k + 1] = domains[i].facets[k].I2 + vertexOffset;
                    indices[indexOffset+3 * k + 2] = domains[i].facets[k].I3 + vertexOffset;;
                }
				vertexOffset = positions.size();
                indexOffset = indices.size();
                auto& actor = scene->CreateActor(std::to_string(i));
				domainActors.push_back(&actor);
            }
             auto model = Core::Global::ServiceLocator::Get<Core::ResourceManagement::ModelManager>().LoadFromMemory(filePath, positions,normals,uvs, indices);
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
            auto& actor = scene->CreateActor("Root","Geomerty");
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
            desc.buffetLen = domainColor.size()*sizeof(Maths::FVector4);
            desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
                .data = domainColor.data()
            };
            ::Rendering::HAL::GLTexture*  domainColorTex = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_BUFFER);
			domainColorTex->Allocate(desc);
            tempMat->SetProperty("domainColorTex",domainColorTex);
            auto& colorBar=actor.AddComponent<Core::ECS::Components::ColorBar>();
			colorBar.SetColors(domainColor);
        }
    }
}