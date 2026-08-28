#include <numbers>
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
#include "ExtrudeFeature.h"
#include "SketcherFeature.h"
#include "Sketcher/SketcherObj.h"
#include "core/log.h"
#include "App/ExtrusionHelper.h"
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

namespace MOON {
    ExtrudeFeature::ExtrudeFeature(const std::string& p_name, int addSubType) :FeatureBaseProfile(p_name,addSubType==0? "Pad": "Pocket")
	{
        this->addSubType = addSubType;
	}
    ExtrudeFeature::~ExtrudeFeature()
	{
	}
	bool ExtrudeFeature::execute()
	{
		Part::TopoShape face=getProfileFace();
        Part::TopoShape baseShape;
        if (m_baseFeature) {
            baseShape=getBaseTopoShape();
        }
       
		Part::ExtrusionParameters params;
		params.taperAngleFwd = angleForward * std::numbers::pi / 180.0;
		params.innerWireTaper = Part::InnerWireTaper::SameAsOuter;
		params.dir = finalDir;
		params.solid = true;
		params.lengthFwd = lengthForward;
		if (dirType == 0)      // 正向
		{

		}
		else if (dirType == 1) {
			params.lengthFwd *= -1;
		}
		else if (dirType == 2) // 双向
		{
			params.lengthRev = lengthRev;
			params.taperAngleRev = angleRev * std::numbers::pi / 180.0;

		}
		else if (dirType == 3) // 对称
		{
			params.lengthRev = params.lengthFwd;
			params.taperAngleRev = params.taperAngleFwd * std::numbers::pi / 180.0;
		}
        Part::TopoShape prism;
        if (extrudeType==2 && !upToFace.isNull()) {
            try
            {
                Part::TopoShape tempShape =face.makeElementFace(nullptr, "Part::FaceMakerBullseye");
                prism = prism.makeElementPrismUntil(
                    tempShape,
                    supportShape,
                    upToFace, -params.dir, Part::TopoShape::PrismMode::None,
                    true);
                if (prism.isNull()) {
                    CORE_ERROR("Prim is Null");
                    return false;
                }
                Part::TopoShape resShape;
                if (!baseShape.isNull()) {
                    if (addSubType == 0) {
                        resShape = prism.makeElementFuse(baseShape);
                    }
                    else if (addSubType == 1) {
                        resShape = baseShape.makeElementCut(prism);
                    }
                }
                else {
                    if(addSubType == 0)
                    resShape = prism;
                }
                topoShape->setShape(resShape);
                getPreviewShape() =resShape;
                return true;
            }
            catch (const std::exception&)
            {
                return false;
            }
        }
        else
        {
            try {
                std::vector<Part::TopoShape> drafts;
                Part::ExtrusionHelper::makeElementDraft(
                    params,
                    face,
                    drafts, App::StringHasherRef()
                );
                if (drafts.empty()) {
                    return false;
                }
                prism.makeElementCompound(
                    drafts,
                    nullptr,
                    Part::TopoShape::SingleShapeCompoundCreationPolicy::returnShape
                );

                getPreviewShape() = prism;

                Part::TopoShape resShape;
                if (!baseShape.isNull()) {
                    if (addSubType == 0) {
                        resShape = prism.makeElementFuse(baseShape);
                    }
                    else if (addSubType == 1) {
                        resShape = baseShape.makeElementCut(prism);
                    }
                }
                else {
                    if (addSubType == 0)
                    resShape = prism;
                }
                topoShape->setShape(resShape);
                return true;
            }
            catch (Base::ValueError e) {
                CORE_ERROR(e.getMessage());
                return false;
            }
            catch (...) {
                CORE_ERROR("Unknow Exception throw");
                return false;
            }
        }
        return false;
	}
}