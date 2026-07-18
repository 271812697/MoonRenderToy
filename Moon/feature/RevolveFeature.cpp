#include "core/component/TopoShapeActor.h"
#include "renderer/SceneView.h"
#include <Core/ResourceManagement/MaterialManager.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/ECS/Components/CModelRenderer.h>
#include "Core/ECS/Components/CBatchMeshTriangle.h"
#include "Core/ECS/Components/CBatchMeshLine.h"
#include "Core/ResourceManagement/ModelManager.h"
#include "editor/View/sceneview/viewerwidget.h"
#include "core/component/CTopoShape.h"
#include <Core/Global/ServiceLocator.h>
#include <Core/SceneSystem/Scene.h>
#include "TopoShape.h"
#include "RevolveFeature.h"
#include "core/log.h"
#include <gp_Pln.hxx>
#include <BRepTools.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <GeomAbs_Shape.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <BRepAlgo.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <BRepLProp_SLProps.hxx>
#include <Precision.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <tracy/Tracy.hpp>
namespace MOON {
	RevolveFeature::RevolveFeature(const std::string& p_name, int addSubType) :FeatureBaseProfile(p_name, addSubType==0? "Revolve" : "Groove")
	{
        this->addSubType = addSubType;
	}
	RevolveFeature::~RevolveFeature()
	{
	}
	bool RevolveFeature::execute()
	{
        Part::TopoShape face = getProfileFace();
        Part::TopoShape baseShape;
        if (m_baseFeature) {
            baseShape = getBaseTopoShape();
        }
        gp_Ax1 raxis = axis;
        if (reverse) {
            raxis.Reverse();
        }
        float radAngle =angle * 3.14159265358979323846f / 180.0f;
        Part::TopoShape revolve;
        {
            ZoneScopedN("Revolve");
            revolve = face.makeElementRevolve(raxis, radAngle, "Part::FaceMakerCheese");
            getPreviewShape() = revolve;
        }

        Part::TopoShape resShape;
        if (!baseShape.isNull()) {
            ZoneScopedN("makeBoolen");
            if (addSubType == 0) {
                resShape = baseShape.makeElementFuse(revolve);
            }
            else if (addSubType == 1) {
                resShape = baseShape.makeElementCut(revolve);
            }
        }
        else
        {
            resShape = revolve;
        }
        
        topoShape->setShape(resShape);
        
        return true;
	}
}