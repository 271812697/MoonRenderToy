#include <tinyxml2.h>
#include <tracy/Tracy.hpp>
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
#include "Interactive/Im3DRenderer.h"

#include "renderer/SceneView.h"
#include "core/JobSystem.h"
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
        HighLightOption highOption;
		CTopoShape* mSelf = nullptr;
		Part::TopoShape mTopoShape;
        std::vector<std::pair<int, int>>childMeshInfos;
        /*
        use these two vectors because we may skip some empty domains
         when we generate childMeshInfos       
        */
        std::vector<int>domainIndexToFaceChildIndex;
        //std::vector<int>domainIndexToEdgeChildIndex;
        bool updateFace = false;
        bool updateEdge = false;
        bool updateChildMesh = false;
        bool hoverLine = false;
		std::vector<int>curOpaqueChildMeshIndex;
        std::vector<int>curTransparentChildMeshIndex;
        std::vector<std::pair<int, int>> curTransparentChildMesh;
        std::vector<std::pair<int, int>>curOpaqueChildMesh;
        std::vector<Eigen::Vector3f> lineSeg;
	};
	CTopoShape::CTopoShape(ECS::Actor& p_owner) : AComponent(p_owner),mInternal(new CTopoShapeInternal(this))
	{
        //switchHighLightMode(HighLightOption::Mode mode)
	}

	CTopoShape::~CTopoShape()
	{
		delete mInternal;
	}

	std::string CTopoShape::GetName()
	{
		return "CTopoShape";
	}

    HighLightOption& CTopoShape::getHightLightOption()
    {
        return mInternal->highOption;
    }

    void CTopoShape::switchHighLightMode(HighLightOption::Mode mode)
    {
        if (mode == HighLightOption::Mode::Color) {
            if (mInternal->highOption.mode == HighLightOption::Mode::Transparent) {
                setChildsMeshTransParent({});
            }
        }
        mInternal->highOption.mode = mode;
    }

	void CTopoShape::OnUpdate(float p_deltaTime)
	{
        if (mInternal->hoverLine) {
            auto& instance=MOON::ImRenderer::instance();
            instance.drawLineList(mInternal->lineSeg, 3.0f, Eigen::Vector4<uint8_t>(255, 0, 255, 255));
        }
        if (mInternal->updateFace|| mInternal->updateEdge) {
            ZoneScoped;
            auto& view = GetService(::Editor::Panels::SceneView);
            auto& renderer = view.GetRenderer();
            auto scene = view.GetScene();
            if (mInternal->updateFace) {
                mInternal->updateFace = false; 
                mInternal->childMeshInfos.clear();
                std::vector<Data::ComplexGeoData::Domain> domains;
               
                mInternal->mTopoShape.getDomainfaces(domains,  mInternal->mTopoShape.getAccuracy());
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
                
                std::vector<::Rendering::Geometry::bbox>domainBoxs;
                std::vector<uint32_t>domainRange;
                auto faceChild = owner.GetChild("Face");
                scene->DelayDestroyActor(faceChild->GetChildren());
                int domainIndex = -1;
                mInternal->domainIndexToFaceChildIndex.resize(domains.size(),-1);
                std::vector<int>DomainIndexToi(domains.size(), -1);
                std::vector<int>domainId;
                std::vector<std::pair<int,int>>domainVertexNum;
                std::vector<std::pair<int, int>>domainIndexNum; 
                int numDomains =0 ;
                //int vertexOffset = 0;
                //int indexOffset = 0;
                ZoneScopedN("Batch");
                for (int i = 0; i < domains.size(); i++) {
                    if (domains[i].facets.size() > 0) {
                        domainIndex++;
                        mInternal->domainIndexToFaceChildIndex[i] = domainIndex;
                        DomainIndexToi[domainIndex] = i;
                        auto& actor = scene->CreateActor("face_" + std::to_string(i));
                        actor.SetParent(*faceChild);     
                        domainId.push_back(actor.GetID());
                        domainColor.push_back(colors[cnt]);
                        cnt = (cnt + 1) % 12;
                        mInternal->childMeshInfos.push_back(std::make_pair<int, int>(indexOffset, domains[i].facets.size() * 3));
                        domainVertexNum.push_back({vertexOffset, domains[i].points.size() });
                        domainIndexNum.push_back({indexOffset ,domains[i].facets.size() * 3 });
                        vertexOffset += domains[i].points.size();
                        indexOffset += domains[i].facets.size() * 3;
                        domainRange.push_back(indexOffset);
                        numDomains++;
                    }
                }
                faceVertices.resize(vertexOffset);
                indices.resize(indexOffset);
                domainBoxs.resize(numDomains);

                auto lamda = [&](JobDispatchArgs arg) {
                    int domainIndex=arg.jobIndex;
                    Maths::FVector2 indexId = Maths::FVector2{ domainIndex * 1.0f,domainId[domainIndex] * 1.0f };

                    ::Rendering::Geometry::bbox subBox;
                    int iIndex= DomainIndexToi[domainIndex];
                    for (int k = 0; k < domains[iIndex].points.size(); k++) {
                        faceVertices[domainVertexNum[domainIndex].first + k] = {
                            Maths::FVector3{ static_cast<float>(domains[iIndex].points[k].x),static_cast<float>(domains[iIndex].points[k].y),static_cast<float>(domains[iIndex].points[k].z) },
                            indexId,
                            Maths::FVector3{ static_cast<float>(domains[iIndex].normals[k].x),static_cast<float>(domains[iIndex].normals[k].y),static_cast<float>(domains[iIndex].normals[k].z) }
                   
                        };
                        subBox.grow(faceVertices[domainVertexNum[domainIndex].first + k].position);
                    }
                    domainBoxs[domainIndex]=subBox;
                    for (int k = 0; k < domains[iIndex].facets.size(); k++) {
                        indices[domainIndexNum[domainIndex].first + 3 * k] = domains[iIndex].facets[k].I1 + domainVertexNum[domainIndex].first;
                        indices[domainIndexNum[domainIndex].first + 3 * k + 1] = domains[iIndex].facets[k].I2 + domainVertexNum[domainIndex].first;
                        indices[domainIndexNum[domainIndex].first + 3 * k + 2] = domains[iIndex].facets[k].I3 + domainVertexNum[domainIndex].first;;
                    }
                };
                MOON::System::JobSystem::Context ctx;
                MOON::System::JobSystem::Dispatch(ctx, numDomains, 10, lamda);
                MOON::System::JobSystem::Wait(ctx);

                ::Rendering::Resources::Mesh* faceMesh = nullptr;
                {
                    ZoneScopedN("faceMesh");

                    faceMesh = new ::Rendering::Resources::Mesh(
                        faceVertices,
                        indices,
                        0,
                        ::Rendering::Settings::EPrimitiveMode::TRIANGLES);
                    //add this for transparent
                    faceMesh->AddSubRangeBuffer();
                    faceMesh->AddMaterial(1,1);
                }

                

                auto model = faceChild->GetComponent<Core::ECS::Components::CModelRenderer>()->GetModel();
                model->GetMaterialNames().emplace_back("Face");
                model->ClearMeshes();
                model->AddMesh(faceMesh);
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
                // 创建并注册默认材质
                auto& materilaRener = *faceChild->GetComponent <Core::ECS::Components::CMaterialRenderer>();
                Core::Resources::Material* tempMat = materilaRener.GetMaterialAtIndex(0);
                tempMat->SetProperty("domainColorTex", domainColorTex);
                auto& bacthMesh = *faceChild->GetComponent<Core::ECS::Components::CBatchMeshTriangle>();
                bacthMesh.SetColors(domainColor);
                {
                    ZoneScopedN("domainBoxs BuildBvh"); 
                    bacthMesh.BuildBvh(domainBoxs, domainRange);
                }
                setChildsMeshTransParent({});
            }
            if (mInternal->updateEdge)
            {
                mInternal->updateEdge = false;
                std::vector<Base::Vector3d>linePoints;
                std::vector<Data::ComplexGeoData::Line>LineRanges;
                mInternal->mTopoShape.getLines(linePoints, LineRanges, mInternal->mTopoShape.getAccuracy());
                //build lines
                std::vector<::Rendering::Geometry::VertexBVH> p_vertices;
                std::vector<uint32_t>lineIndex;
                std::vector<uint32_t>lineSegmentOffsets;
                auto edgeChild = owner.GetChild("Edge");
                std::vector<Core::ECS::Actor*> edgeChildList = edgeChild->GetChildren();
                for (int i = 0; i < edgeChildList.size(); i++) {
                    scene->DelayDestroyActor(*edgeChildList[i]);
                }
                p_vertices.reserve(linePoints.size());
                lineSegmentOffsets.reserve(LineRanges.size());
                for (int i = 0; i < LineRanges.size(); i++) {
                    ZoneScopedN("line");
                    auto& l = LineRanges[i];
                    auto& actor = scene->CreateActor("edge_" + std::to_string(i));
                    float subLineId=actor.GetID() * 1.0f;
                    
                    actor.SetParent(*edgeChild);
                    for (int k = l.I1; k <= l.I2 - 1; k++) {
                        ::Rendering::Geometry::VertexBVH v;
                        v.position.x = static_cast<float>(linePoints[k].x);
                        v.position.y = static_cast<float>(linePoints[k].y);
                        v.position.z = static_cast<float>(linePoints[k].z);
                        v.texCoords.x = i * 1.0f;
                        v.texCoords.y = subLineId;
                        p_vertices.emplace_back(v);

                        lineIndex.push_back(k);
                        lineIndex.push_back(k + 1);
                    }
                    ::Rendering::Geometry::VertexBVH v;
                    v.position.x = static_cast<float>(linePoints[l.I2].x);
                    v.position.y = static_cast<float>(linePoints[l.I2].y);
                    v.position.z = static_cast<float>(linePoints[l.I2].z);
                    v.texCoords.x = i * 1.0f;
                    v.texCoords.y = subLineId;
                    p_vertices.emplace_back(v);

                    lineSegmentOffsets.emplace_back(lineIndex.size());
                }
               
                auto lineMesh = new ::Rendering::Resources::Mesh(
                    p_vertices,
                    lineIndex,
                    0,
                    ::Rendering::Settings::EPrimitiveMode::LINES);
               
                auto lineModel = edgeChild->GetComponent<Core::ECS::Components::CModelRenderer>()->GetModel();
                lineModel->GetMaterialNames().emplace_back("Line");
                lineModel->ClearMeshes();
                lineModel->AddMesh(lineMesh);
                auto& lineBacthMesh =*edgeChild->GetComponent<Core::ECS::Components::CBatchMeshLine>();
                lineBacthMesh.BuildBvh(lineSegmentOffsets);
            }        
        }
        if (mInternal->updateChildMesh) {
            mInternal->updateChildMesh = false;
            updateChildMesh();
        }
	}

    void CTopoShape::updateChildBuffer()
    {
        mInternal->updateChildMesh = true;
    }

    std::vector<std::pair<int, int>> CTopoShape::GetChildMeshInfo()
    {
        return mInternal->childMeshInfos;
        
    }

    void CTopoShape::setChildsMeshTransParent(const std::vector<int>& childs)
    {
        updateChildBuffer();
        std::vector<int>listTransparentIndex;
        std::vector<int>listOpaqueIndex;
        listTransparentIndex.resize(childs.size());
        listOpaqueIndex.resize(mInternal->childMeshInfos.size() - childs.size());
        std::vector<int>table(mInternal->childMeshInfos.size(),0);
        int indexTransparent = 0;
        int indexOpaque = 0;
        for (int i = 0; i < childs.size(); i++) {
            table[childs[i]] = 1;
        }
        for (int i = 0;i < table.size();i++) {
            if (table[i] == 1) {
                listTransparentIndex[indexTransparent++] = i;
            }
            else
            {
                listOpaqueIndex[indexOpaque++] = i;
            }
        }
		mInternal->curTransparentChildMeshIndex = listTransparentIndex;
		mInternal->curOpaqueChildMeshIndex = listOpaqueIndex;
    }

	Part::TopoShape& CTopoShape::GetTopoShape()
	{
		return mInternal->mTopoShape;
	}

    Part::TopoShape CTopoShape::GetTopoFace(int childFaceId)
    {
        return mInternal->mTopoShape.getSubTopoShape(TopAbs_FACE,childFaceId+1);
    }

    Part::TopoShape CTopoShape::GetTopoEdge(int childFaceId)
    {
        return mInternal->mTopoShape.getSubTopoShape(TopAbs_EDGE, childFaceId + 1);
    }

    void CTopoShape::hoverChild(int childId)
    {
        
        if (mInternal->highOption.mode == HighLightOption::Mode::Color) {
            auto& bacthMesh = *owner.GetChild("Face")->GetComponent<Core::ECS::Components::CBatchMeshTriangle>();
            bacthMesh.SetHoverColor(mInternal->domainIndexToFaceChildIndex[childId], mInternal->highOption.hoverColor);
        }
        else if(mInternal->highOption.mode == HighLightOption::Mode::Transparent)
        {
            setChildsMeshTransParent({ mInternal->domainIndexToFaceChildIndex[childId] });
        }
    }

    void CTopoShape::hoverChildLine(int childId)
    {
        mInternal->hoverLine = true;
        auto& colorBar = *owner.GetChild("Edge")->GetComponent<Core::ECS::Components::CBatchMeshLine>();
        auto vertexArray = colorBar.getLineSeg(childId);
        mInternal->lineSeg.clear();
        mInternal->lineSeg.reserve(vertexArray.size());
        for (auto v : vertexArray) {
            mInternal->lineSeg.push_back(Eigen::Vector3f(v.x, v.y, v.z));;
        }
    }

    void CTopoShape::clearHover()
    {
        if (mInternal->highOption.mode == HighLightOption::Mode::Color) {
            auto& bacthMesh = *owner.GetChild("Face")->GetComponent<Core::ECS::Components::CBatchMeshTriangle>();
            bacthMesh.ClearHoverColor();
        }
        else if (mInternal->highOption.mode == HighLightOption::Mode::Transparent)
        {
            setChildsMeshTransParent({  });
        }
    }

    void CTopoShape::clearHoverLine()
    {
        mInternal->hoverLine = false;
    }

    void CTopoShape::discretizationFaceShape()
    {
        mInternal->updateFace = true;
    }

    void CTopoShape::discretizationEdgeShape()
    {
		mInternal->updateEdge = true;
    }

	void CTopoShape::discretizationShape()
	{
        discretizationFaceShape();
        discretizationEdgeShape();
	}

	void CTopoShape::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
	{
	}

	void CTopoShape::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
	{
	}
    void CTopoShape::updateChildMesh()
    {
		mInternal->curOpaqueChildMesh.clear();
		mInternal->curTransparentChildMesh.clear();
        auto& children = owner.GetChild("Face")->GetChildren();
        for (int i = 0;i < mInternal->curOpaqueChildMeshIndex.size();i++) {
			int id = mInternal->curOpaqueChildMeshIndex[i];
            if (children[id]->IsActive()) {
				mInternal->curOpaqueChildMesh.push_back(mInternal->childMeshInfos[id]);
            }   
        }
        for (int i = 0; i < mInternal->curTransparentChildMeshIndex.size(); i++) {
			int id = mInternal->curTransparentChildMeshIndex[i];
            if (children[id]->IsActive()) {
				mInternal->curTransparentChildMesh.push_back(mInternal->childMeshInfos[id]);
            }
        }
        auto model = owner.GetChild("Face")->GetComponent<Core::ECS::Components::CModelRenderer>()->GetModel();
        auto mesh = model->GetMesh(0);
        mesh->UploadIndices(mInternal->curOpaqueChildMesh, 0);
        mesh->UploadIndices(mInternal->curTransparentChildMesh, 1);
    }
}
