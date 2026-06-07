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
        HighLightOption highOption;
		CTopoShape* mSelf = nullptr;
		Part::TopoShape mTopoShape;
        std::vector<std::pair<int, int>>childMeshInfos;
        bool updateFace = false;
        bool updateEdge = false;
        bool updateChildMesh = false;
		std::vector<int>curOpaqueChildMeshIndex;
        std::vector<int>curTransparentChildMeshIndex;
        std::vector<std::pair<int, int>> curTransparentChildMesh;
        std::vector<std::pair<int, int>>curOpaqueChildMesh;
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
        if (mInternal->updateFace|| mInternal->updateEdge) {
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
                std::vector<Core::ECS::Actor*>domainActors;
                std::vector<::Rendering::Geometry::bbox>domainBoxs;
                std::vector<uint32_t>domainRange;
                int domainIndex = -1;
                for (int i = 0; i < domains.size(); i++) {
                    if (domains[i].facets.size() > 0) {
                        auto& actor = scene->CreateActor("face_" + std::to_string(i));
                        domainActors.push_back(&actor);
                        domainIndex++;
                        domainColor.push_back(colors[cnt]);
                        cnt = (cnt + 1) % 12;
                        faceVertices.reserve(faceVertices.size() + domains[i].points.size());

                        indices.resize(indexOffset + domains[i].facets.size() * 3);                       
                        mInternal->childMeshInfos.push_back(std::make_pair<int,int>(indexOffset,domains[i].facets.size() * 3 ));
                        ::Rendering::Geometry::bbox subBox;
                        for (int k = 0; k < domains[i].points.size(); k++) {
                            faceVertices.emplace_back(
                                Maths::FVector3{ static_cast<float>(domains[i].points[k].x),static_cast<float>(domains[i].points[k].y),static_cast<float>(domains[i].points[k].z) },
                                Maths::FVector2{ domainIndex * 1.0f,actor.GetID()*1.0f},
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

                        domainBoxs.push_back(subBox);
                        domainRange.push_back(indexOffset);
                    }
                }

                auto faceMesh = new ::Rendering::Resources::Mesh(
                    faceVertices,
                    indices,
                    0,
                    ::Rendering::Settings::EPrimitiveMode::TRIANGLES);
                //add this for transparent
                faceMesh->AddSubRangeBuffer();
                faceMesh->AddMaterial(2,1);
                auto model = owner.GetComponent<Core::ECS::Components::CModelRenderer>()->GetModel();
                model->GetMaterialNames().emplace_back("Face");
                model->AddMesh(faceMesh);

                for (auto* acptr : domainActors) {
                    acptr->SetParent(owner);
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
                // 创建并注册默认材质
                auto& materilaRener = *owner.GetComponent <Core::ECS::Components::CMaterialRenderer>();
                Core::Resources::Material* tempMat = materilaRener.GetMaterialAtIndex(0);
                tempMat->SetProperty("domainColorTex", domainColorTex);
                auto& bacthMesh = *owner.GetComponent<Core::ECS::Components::CBatchMeshTriangle>();
                bacthMesh.SetColors(domainColor);
                bacthMesh.BuildBvh(domainBoxs, domainRange);
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

                p_vertices.reserve(linePoints.size());
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
                }
                auto lineMesh = new ::Rendering::Resources::Mesh(
                    p_vertices,
                    lineIndex,
                    1,
                    ::Rendering::Settings::EPrimitiveMode::LINES);
                auto lineModel = owner.GetComponent<Core::ECS::Components::CModelRenderer>()->GetModel();
                lineModel->GetMaterialNames().emplace_back("Line");
                lineModel->AddMesh(lineMesh);
                auto& lineBacthMesh =*owner.GetComponent<Core::ECS::Components::CBatchMeshLine>();
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
        //std::vector<std::pair<int, int>>listTransparent;
        //std::vector<std::pair<int, int>>listOpaque;
        std::vector<int>listTransparentIndex;
        std::vector<int>listOpaqueIndex;
        //listTransparent.resize(childs.size());
        //listOpaque.resize(mInternal->childMeshInfos.size()- childs.size());
        listTransparentIndex.resize(childs.size());
        listOpaqueIndex.resize(mInternal->childMeshInfos.size() - childs.size());
        std::vector<int>table(mInternal->childMeshInfos.size(),0);
        int indexTransparent = 0;
        int indexOpaque = 0;
        auto& children=owner.GetChildren();
        for (int i = 0; i < childs.size(); i++) {
            table[childs[i]] = 1;
        }
        for (int i = 0;i < table.size();i++) {
            //if (children[i]->IsActive())
            {
                if (table[i] == 1) {
                    listTransparentIndex[indexTransparent++] = i;
                    //listTransparent[indexTransparent++] = mInternal->childMeshInfos[i];
                }
                else
                {
                    listOpaqueIndex[indexOpaque++] = i;
                   // listOpaque[indexOpaque++]= mInternal->childMeshInfos[i];
                }
            }
        }
		mInternal->curTransparentChildMeshIndex = listTransparentIndex;
		mInternal->curOpaqueChildMeshIndex = listOpaqueIndex;
        //mInternal->curTransparentChildMesh = listTransparent;
        //mInternal->curOpaqueChildMesh = listOpaque;
    }

	Part::TopoShape& CTopoShape::GetTopoShape()
	{
		return mInternal->mTopoShape;
	}

    void CTopoShape::hoverChild(int childId)
    {
        if (mInternal->highOption.mode == HighLightOption::Mode::Color) {
            auto& bacthMesh = *owner.GetComponent<Core::ECS::Components::CBatchMeshTriangle>();
            bacthMesh.SetHoverColor(childId, mInternal->highOption.hoverColor);
        }
        else if(mInternal->highOption.mode == HighLightOption::Mode::Transparent)
        {
            setChildsMeshTransParent({ childId });
        }
    }

    void CTopoShape::clearHover()
    {
        if (mInternal->highOption.mode == HighLightOption::Mode::Color) {
            auto& bacthMesh = *owner.GetComponent<Core::ECS::Components::CBatchMeshTriangle>();
            bacthMesh.ClearHoverColor();
        }
        else if (mInternal->highOption.mode == HighLightOption::Mode::Transparent)
        {
            setChildsMeshTransParent({  });
        }
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
        auto& children = owner.GetChildren();
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
        auto model = owner.GetComponent<Core::ECS::Components::CModelRenderer>()->GetModel();
        auto mesh = model->GetMesh(0);
        mesh->UploadIndices(mInternal->curOpaqueChildMesh, 0);
        mesh->UploadIndices(mInternal->curTransparentChildMesh, 1);
    }
}
