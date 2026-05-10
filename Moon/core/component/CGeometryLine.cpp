#include <tinyxml2.h>
#include <Core/ECS/Actor.h>
#include "core/component/CGeometryLine.h"
#include <Core/ECS/Components/CModelRenderer.h>
#include <Core/Global/ServiceLocator.h>
#include <Core/ResourceManagement/MaterialManager.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include "Geometry.h"
#include "Core/ResourceManagement/ModelManager.h"
#include "renderer/SceneView.h"
#include "base/Tools2D.h"
#include "core/callbackManager.h"
namespace Core::ECS::Components
{
    static std::vector<Base::Vector2d> toVector2D(const Part::Geometry* geometry,int curvedEdgeCountSegments)
    {
        std::vector<Base::Vector2d> vector2d;
        auto emplaceasvector2d = [&vector2d](const Base::Vector3d& point) {
            vector2d.emplace_back(point.x, point.y);
            };
        auto isperiodicconic = geometry->is<Part::GeomCircle>() || geometry->is<Part::GeomEllipse>();
        auto isbounded = geometry->isDerivedFrom<Part::GeomBoundedCurve>();

        if (geometry->is<Part::GeomLineSegment>())
        {  // add a line
            auto geo = static_cast<const Part::GeomLineSegment*>(geometry);
            emplaceasvector2d(geo->getStartPoint());
            emplaceasvector2d(geo->getEndPoint());
        }
        else if (isperiodicconic || isbounded)
        {
            auto geo = static_cast<const Part::GeomConic*>(geometry);
            double segment = (geo->getLastParameter() - geo->getFirstParameter())
                / curvedEdgeCountSegments;
            for (int i = 0; i < curvedEdgeCountSegments; i++) {
                emplaceasvector2d(geo->value(geo->getFirstParameter() + i * segment));
            }
            // either close the curve for untrimmed conic or set the last point for bounded curves
            emplaceasvector2d(isperiodicconic ? geo->value(0) : geo->value(geo->getLastParameter()));
        }
        return vector2d;
    }
	class CGeometryLine::CGeometryLineInternal {
	public:
		CGeometryLineInternal(CGeometryLine* self) :mSelf(self){

		}
		~CGeometryLineInternal() {
		}
	private:
		friend class CGeometryLine;
		CGeometryLine* mSelf = nullptr;
        bool update = false;
        bool buildLine = false;
        int plane = 2;
        Part::Geometry* mGeometry;
	};
	CGeometryLine::CGeometryLine(ECS::Actor& p_owner) : AComponent(p_owner),mInternal(new CGeometryLineInternal(this))
	{
	}

	CGeometryLine::~CGeometryLine()
	{
		delete mInternal;
	}

	std::string CGeometryLine::GetName()
	{
		return "CGeometryLine";
	}

	void CGeometryLine::OnUpdate(float p_deltaTime)
	{ 
        if (mInternal->update) {
			mInternal->update = false;
            buildLines(mInternal->plane);
        }
	}

	Part::Geometry* CGeometryLine::GetGeometry()
	{
        
		return mInternal->mGeometry;
	}

    void CGeometryLine::setGeometry(Part::Geometry* geometry)
	{
		mInternal->mGeometry = geometry;
    }

	void CGeometryLine::discretizationShape(int plane)
	{
        mInternal->update = true;
        mInternal->plane = plane;
	}

	void CGeometryLine::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
	{
	}

	void CGeometryLine::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
	{
	}

    void CGeometryLine::buildComp()
    {
        if (mInternal->update) {
            mInternal->update = false;
            mInternal->buildLine = true;
        }
    }

    void CGeometryLine::buildLines(int plane)
	{
        //build lines
        std::vector<::Rendering::Geometry::VertexBVH> p_vertices;
        std::vector<uint32_t>lineIndex;
        std::vector<Base::Vector2d>linePoints = toVector2D(mInternal->mGeometry, 50);
        p_vertices.reserve(linePoints.size());
        for (int i = 0; i < linePoints.size(); i++) {
            ::Rendering::Geometry::VertexBVH v;
            if (plane == 2) {
                v.position.x = static_cast<float>(linePoints[i].x);
                v.position.y = static_cast<float>(linePoints[i].y);
                v.position.z = 0;
            }
            else if (plane == 0) {
                v.position.x = 0;
                v.position.y = static_cast<float>(linePoints[i].x);
                v.position.z = static_cast<float>(linePoints[i].y);
            }
            else {
                v.position.x = static_cast<float>(linePoints[i].x);
                v.position.y = 0;
                v.position.z = static_cast<float>(linePoints[i].y);
            }

            v.texCoords.x = i * 1.0f;
            v.texCoords.y = i * 1.0f;
            p_vertices.emplace_back(v);
            if (i < linePoints.size() - 1) {
                lineIndex.push_back(i);
                lineIndex.push_back(i + 1);
            }
        }
        auto lineMesh = new ::Rendering::Resources::Mesh(
            p_vertices,
            lineIndex,
            0,
            ::Rendering::Settings::EPrimitiveMode::LINES);
        auto lineModel = new ::Rendering::Resources::Model(owner.GetName() + std::string("_lineModel"));
        ::Core::Global::ServiceLocator::Get<Core::ResourceManagement::ModelManager>().RegisterResource(owner.GetName() + std::string("_lineModel"), lineModel);
        lineModel->GetMaterialNames().emplace_back("Default");
        lineModel->AddMesh(lineMesh);

        owner.GetComponent<Core::ECS::Components::CModelRenderer>()->SetModel(lineModel);
        auto& lineRener = *owner.GetComponent<Core::ECS::Components::CMaterialRenderer>();

        auto lineMat = new Core::Resources::Material();
        Core::Global::ServiceLocator::Get<Core::ResourceManagement::MaterialManager>().RegisterResource(owner.GetName() + std::string("_lineMat"), lineMat);
        lineMat->SetShader(Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager[":Shaders\\Line.ovfx"]);
        lineMat->SetBackfaceCulling(false);
        lineMat->SetCastShadows(false);
        lineMat->SetReceiveShadows(false);
        lineMat->SetLineWidth(2.0);
        lineRener.SetMaterialAtIndex(0, *lineMat);
        lineRener.UpdateMaterialList();
	}

	
}
