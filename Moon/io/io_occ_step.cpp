#include <tracy/Tracy.hpp>
#include "TopoShape.h"
#include "Core/SceneSystem/Scene.h"
#include "feature/Feature.h"
#include "core/component/CTopoShape.h"
#include "core/JobSystem.h"
#include "io_occ_step.h"
#include <Core/ResourceManagement/ShaderManager.h>

#include "core/Global/ServiceLocator.h"
#include "editor/View/sceneview/viewerwidget.h"
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <renderer/SceneView.h>
namespace MOON {
    // 读取 STEP 模型并返回其形状
    namespace IO {
        struct LoadedDomain {

			std::vector<::Rendering::Geometry::VertexPositionNormal> vertex;
            std::vector<uint32_t> indices;
        };
		static bool LoadDomainsFromFile(const std::string& path, std::vector<LoadedDomain>& out)
		{
			std::ifstream in(path, std::ios::binary);
			if (!in.is_open()) return false;

			uint64_t domainCount = 0;
			in.read(reinterpret_cast<char*>(&domainCount), sizeof(domainCount));
			if (!in) return false;
			out.reserve((size_t)domainCount);

			for (uint64_t d = 0; d < domainCount; d++) {
				uint64_t ptSize = 0, nmlSize = 0, triSize = 0;
				in.read(reinterpret_cast<char*>(&ptSize), sizeof(ptSize));
				in.read(reinterpret_cast<char*>(&nmlSize), sizeof(nmlSize));
				in.read(reinterpret_cast<char*>(&triSize), sizeof(triSize));
				if (!in) return false;

				LoadedDomain dom;
				dom.vertex.resize((size_t)ptSize);
				dom.indices.resize((size_t)triSize * 3);

				// 整块读入再转 float3，避免逐分量 read
				std::vector<double> raw((size_t)(ptSize + nmlSize) * 3);
				in.read(reinterpret_cast<char*>(raw.data()),
					(std::streamsize)(raw.size() * sizeof(double)));
				if (!in) return false;

				for (size_t i = 0; i < ptSize; i++) {
					dom.vertex[i].position = {
						(float)raw[i * 3 + 0], (float)raw[i * 3 + 1], (float)raw[i * 3 + 2]
					};
				}
				for (size_t i = 0; i < nmlSize; i++) {
					dom.vertex[i].normals = {
						(float)raw[(ptSize + i) * 3 + 0],
						(float)raw[(ptSize + i) * 3 + 1],
						(float)raw[(ptSize + i) * 3 + 2]
					};
				}

				in.read(reinterpret_cast<char*>(dom.indices.data()),
					(std::streamsize)(dom.indices.size() * sizeof(uint32_t)));
				if (!in) return false;

				// 法线缺失（nmlSize != ptSize）时，用面法线累加补一份
				//if (dom.normals.size() != dom.positions.size()) {
				//	std::vector<Maths::FVector3> flat(dom.positions.size(), { 0, 0, 0 });
				//	for (size_t t = 0; t + 2 < dom.indices.size(); t += 3) {
				//		uint32_t i0 = dom.indices[t], i1 = dom.indices[t + 1], i2 = dom.indices[t + 2];
				//		if (i0 >= dom.positions.size() || i1 >= dom.positions.size() ||
				//			i2 >= dom.positions.size()) continue;
				//		const auto e1 = dom.positions[i1] - dom.positions[i0];
				//		const auto e2 = dom.positions[i2] - dom.positions[i0];
				//		const auto fn = Maths::FVector3::Cross(e1,e2);
				//		flat[i0] += fn; flat[i1] += fn; flat[i2] += fn;
				//	}
				//	dom.normals.resize(dom.positions.size());
				//	for (size_t i = 0; i < dom.normals.size(); i++) {
				//		dom.normals[i] = Maths::FVector3::Normalize(flat[i]);;
				//	}
				//}
				out.push_back(std::move(dom));
			}
			return true;
		}

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
        void IO::ReadSTEPBin(const char* filePath, Core::SceneSystem::Scene* scene)
        {
			std::string  path(filePath);
			std::vector<LoadedDomain> domains;
			if (!LoadDomainsFromFile(path, domains) || domains.empty()) {
				std::cout << "[Mesh] 解析失败: " << path << std::endl;
				return;
			}
			auto&parent=scene->CreateActor();
			GetViewerWidget.addActorToTreeView(&parent);
			for (int i = 0;i < domains.size();i++) {
				auto& actor = scene->CreateActor("Domain_" + std::to_string(i));
				actor.SetParent(parent);
				auto faceModel = new ::Rendering::Resources::Model(std::string("_faceModel") + std::to_string(i));

				auto& modelRender = actor.AddComponent<Core::ECS::Components::CModelRenderer>();
				modelRender.SetModel(faceModel);
				auto mesh = new ::Rendering::Resources::Mesh(domains[i].vertex,
					domains[i].indices,
					0,
					::Rendering::Settings::EPrimitiveMode::TRIANGLES);
			
				modelRender.GetModel()->AddMesh(mesh);
				auto tempMat = new Core::Resources::Material();
				

				tempMat->SetBackfaceCulling(false);;
				tempMat->SetCastShadows(false);
				tempMat->SetReceiveShadows(false);
				tempMat->SetShader(GetShaderService[":Shaders\\Standard.ovfx"]);
				tempMat->AddFeature("CLIP_PLANE");
				tempMat->SetProperty("_EnvironmentMap", GetSceneView.GetRenderer().GetPrefilterCube());					tempMat->SetProperty("u_Albedo", Maths::FVector4{ 1.0, 1.0, 1.0, 1.0 });

				tempMat->SetProperty("u_AlphaClippingThreshold", 1.0f);
				tempMat->SetProperty("u_Roughness", 0.1f);
				tempMat->SetProperty("u_Metallic", 0.1f);
				// Emission
				tempMat->SetProperty("u_EmissiveIntensity", 1.0f);
				tempMat->SetProperty("u_EmissiveColor", Maths::FVector3{ 0.0f,0.0f,0.0f });
				auto& matRender=actor.AddComponent<::Core::ECS::Components::CMaterialRenderer>();
				matRender.SetMaterialAtIndex(0, *tempMat);
				GetViewerWidget.addActorToTreeView(&actor);

			}
        }
    }
}