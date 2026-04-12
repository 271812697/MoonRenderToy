#include <tinyxml2.h>
#include <Core/ECS/Actor.h>
#include "core/component/CTopoShape.h"
#include <Core/ECS/Components/CModelRenderer.h>
#include <Core/Global/ServiceLocator.h>
#include <Core/ResourceManagement/MaterialManager.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include "TopoShape.h"
#include "Core/ECS/Components/CBatchMeshTriangle.h"
#include "Core/ECS/Components/CBatchMeshLine.h"
#include "Core/ResourceManagement/ModelManager.h"
#include "renderer/SceneView.h"
namespace Core::ECS::Components
{
	class CTopoShape::CTopoShapeInternal {
	public:
		CTopoShapeInternal(CTopoShape* self) :mSelf(self){

		}
		~CTopoShapeInternal() {
		}
	private:
		friend class CTopoShape;
		CTopoShape* mSelf = nullptr;
		Part::TopoShape mTopoShape;
	};
	CTopoShape::CTopoShape(ECS::Actor& p_owner) : AComponent(p_owner),mInternal(new CTopoShapeInternal(this))
	{
	}

	CTopoShape::~CTopoShape()
	{
		delete mInternal;
	}

	std::string CTopoShape::GetName()
	{
		return "CTopoShape";
	}

	void CTopoShape::OnUpdate(float p_deltaTime)
	{

	}

	Part::TopoShape& CTopoShape::GetTopoShape()
	{
		return mInternal->mTopoShape;
	}

	void CTopoShape::discretizationShape()
	{
        auto& view = GetService(::Editor::Panels::SceneView);
        auto& renderer = view.GetRenderer();
        auto scene = view.GetScene();
		std::vector<Data::ComplexGeoData::Domain> domains;
		std::vector<Base::Vector3d>linePoints;
		std::vector<Data::ComplexGeoData::Line>LineRanges;
		mInternal->mTopoShape.getDomainfaces(domains,1.0);
		mInternal->mTopoShape.getLines(linePoints, LineRanges, 1.0);
		static Maths::FVector4 colors[] = {
				{ 140.0 / 255.0f, 180.0f / 255.0f, 216.0f / 255.0f, 1.0f }, { 237.0 / 255.0f, 28.0f / 255.0f,36.0f / 255.0f, 1.0f },
				{ 0.0 / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f, 1.0f }, { 0.0 / 255.0f, 162.0f / 255.0f,232.0f / 255.0f, 1.0f },
				 { 112.0 / 255.0f, 146.0f / 255.0f, 190.0f / 255.0f, 1.0f }, { 255.0 / 255.0f, 0.0f / 255.0f,255.0f / 255.0f, 1.0f },
				  { 0.0 / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 1.0f }, { 161.0 / 255.0f, 161.0f / 255.0f,255.0f / 255.0f, 1.0f },
				   { 171.0 / 255.0f, 128.0f / 255.0f, 84.0f / 255.0f, 1.0f }, { 255.0 / 255.0f, 128.0f / 255.0f,191.0f / 255.0f, 1.0f },
					{ 135.0 / 255.0f, 89.0f / 255.0f, 179.0f / 255.0f, 1.0f }, { 255.0 / 255.0f, 191.0f / 255.0f,128.0f / 255.0f, 1.0f }
		};
		std::vector<::Rendering::Geometry::VertexBVH> faceVertices;
		std::vector<unsigned int>indices;

		std::vector<Maths::FVector4>domainColor;
		unsigned int vertexOffset = 0;
		unsigned int indexOffset = 0;
		int cnt = 0;
        std::vector<Core::ECS::Actor*>domainActors;
        std::vector<::Rendering::Geometry::bbox>domainBoxs;
        std::vector<uint32_t>domainRange;
        int domainIndex = -1;
        for (int i = 0; i < domains.size(); i++) {
            if (domains[i].facets.size() > 0) {
                domainIndex++;
                domainColor.push_back(colors[cnt]);
                cnt = (cnt + 1) % 12;
                faceVertices.reserve(faceVertices.size() + domains[i].points.size());

                indices.resize(indexOffset + domains[i].facets.size() * 3);
                ::Rendering::Geometry::bbox subBox;
                for (int k = 0; k < domains[i].points.size(); k++) {
                    faceVertices.emplace_back(
                        Maths::FVector3{ static_cast<float>(domains[i].points[k].x),static_cast<float>(domains[i].points[k].y),static_cast<float>(domains[i].points[k].z) },
                        Maths::FVector2{ domainIndex * 1.0f,0.0f },
                        Maths::FVector3{ static_cast<float>(domains[i].normals[k].x),static_cast<float>(domains[i].normals[k].y),static_cast<float>(domains[i].normals[k].z) }
                    );
                    subBox.grow(faceVertices.back().position);
                }
                for (int k = 0; k < domains[i].facets.size(); k++) {
                    indices[indexOffset + 3 * k] = domains[i].facets[k].I1 + vertexOffset;
                    indices[indexOffset + 3 * k + 1] = domains[i].facets[k].I2 + vertexOffset;
                    indices[indexOffset + 3 * k + 2] = domains[i].facets[k].I3 + vertexOffset;;
                }
                vertexOffset = faceVertices.size();
                indexOffset = indices.size();
                auto& actor = scene->CreateActor("face_"+std::to_string(i));
                domainActors.push_back(&actor);
                domainBoxs.push_back(subBox);
                domainRange.push_back(indexOffset);
            }
        }

        auto faceMesh = new ::Rendering::Resources::Mesh(
            faceVertices,
            indices,
            0,
            ::Rendering::Settings::EPrimitiveMode::TRIANGLES);
       
        auto model = new ::Rendering::Resources::Model(owner.GetName()+std::string("_faceModel"));
        model->AddMesh(faceMesh);
        Core::Global::ServiceLocator::Get<Core::ResourceManagement::ModelManager>().RegisterResource(owner.GetName() + std::string("_faceModel"), model);

        // 创建并注册默认材质
        Core::Resources::Material* tempMat = new Core::Resources::Material();
        Core::Global::ServiceLocator::Get<Core::ResourceManagement::MaterialManager>().RegisterResource(owner.GetName()+"_mat", tempMat);
        tempMat->SetBackfaceCulling(false);
        tempMat->SetCastShadows(false);
        tempMat->SetReceiveShadows(false);
        //tempMat->SetBlendable(true);
        //tempMat->SetDepthWriting(false);
        tempMat->SetShader(Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager[":Shaders\\GeomertySurface.ovfx"]);
        tempMat->SetProperty("u_Albedo", Maths::FVector4(1, 1, 1, 1));
        tempMat->SetProperty("u_AlphaClippingThreshold", 0.0f);
        tempMat->SetProperty("u_Roughness", 0.25f);
        tempMat->SetProperty("u_Metallic", 0.75f);
        // Emission
        tempMat->SetProperty("u_EmissiveIntensity", 1.0f);
        tempMat->SetProperty("u_EmissiveColor", Maths::FVector3{ 0.0f, 0.0f, 0.0f });
        tempMat->AddFeature("WITH_EDGE");

        tempMat->TrySetProperty("_IrradianceCube", renderer.GetIrradianceCube());
        tempMat->TrySetProperty("_PrefilterCube", renderer.GetPrefilterCube());
        tempMat->TrySetProperty("_BRDFLut", renderer.GetBrdfTexture());
        
        // 在场景中创建 Actor 并绑定模型/材质
        auto& actor = scene->CreateActor("RootFace", "Geomerty");
        actor.SetParent(owner);
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
        std::vector<::Rendering::Geometry::VertexBVH> p_vertices;
        std::vector<uint32_t>lineIndex;
        std::vector<uint32_t>lineSegmentOffsets;
        std::vector<Maths::FVector4>lineColor;

        p_vertices.reserve(linePoints.size());
        lineColor.reserve(LineRanges.size());
        lineSegmentOffsets.reserve(LineRanges.size());
        for (int i = 0; i < LineRanges.size(); i++) {
            auto& l = LineRanges[i];
            for (int k = l.I1; k <= l.I2 - 1; k++) {
                ::Rendering::Geometry::VertexBVH v;
                v.position.x = static_cast<float>(linePoints[k].x);
                v.position.y = static_cast<float>(linePoints[k].y);
                v.position.z = static_cast<float>(linePoints[k].z);
                v.texCoords.x = i * 1.0f;
                v.texCoords.y = i * 1.0f;
                p_vertices.emplace_back(v);

                lineIndex.push_back(k);
                lineIndex.push_back(k + 1);
            }
            ::Rendering::Geometry::VertexBVH v;
            v.position.x = static_cast<float>(linePoints[l.I2].x);
            v.position.y = static_cast<float>(linePoints[l.I2].y);
            v.position.z = static_cast<float>(linePoints[l.I2].z);
            v.texCoords.x = i * 1.0f;
            v.texCoords.y = i * 1.0f;
            p_vertices.emplace_back(v);

            lineSegmentOffsets.emplace_back(lineIndex.size());
            lineColor.emplace_back(0, 1, 1, 1);
        }
        //lineColor[575] = { 0,1,1,1 };
        auto lineMesh = new ::Rendering::Resources::Mesh(
            p_vertices,
            lineIndex,
            0,
            ::Rendering::Settings::EPrimitiveMode::LINES);
        auto lineModel = new ::Rendering::Resources::Model(owner.GetName() + std::string("_lineModel"));
        ::Core::Global::ServiceLocator::Get<Core::ResourceManagement::ModelManager>().RegisterResource(owner.GetName()  + std::string("_lineModel"), lineModel);
        lineModel->GetMaterialNames().emplace_back("Default");
        lineModel->AddMesh(lineMesh);

        auto& lineActor = scene->CreateActor("RootLine", "GeomertyLine");
        lineActor.SetParent(owner);
        lineActor.AddComponent<Core::ECS::Components::CModelRenderer>().SetModel(lineModel);
        auto& lineRener = lineActor.AddComponent<Core::ECS::Components::CMaterialRenderer>();

        auto lineMat = new Core::Resources::Material();
        Core::Global::ServiceLocator::Get<Core::ResourceManagement::MaterialManager>().RegisterResource(owner.GetName()+ std::string("_lineMat"), lineMat);
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
        auto& lineBacthMesh = lineActor.AddComponent<Core::ECS::Components::CBatchMeshLine>();
        lineBacthMesh.SetColors(lineColor);
        lineBacthMesh.BuildBvh(lineSegmentOffsets);
	}

	void CTopoShape::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
	{
	}

	void CTopoShape::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
	{
	}

	
}
