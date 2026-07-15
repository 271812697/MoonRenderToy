
#include "TopoShape.h"
#include "SketcherFeature.h"
#include "core/log.h"
#include "Sketcher/SketcherObj.h"

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
	class SketcherFeature::Internal {
	public:
		Internal(SketcherFeature* s):self(s) {
			sketcher = std::make_shared<SketcherObj>();
		}
		~Internal() {
		}
	private:
		friend SketcherFeature;
		SketcherFeature* self = nullptr;
		std::shared_ptr<SketcherObj> sketcher;
	};
    SketcherFeature::SketcherFeature(const std::string& p_name) :Feature(p_name, "Sketcher"),mInternal(new Internal(this))
	{
	}
	SketcherObj* SketcherFeature::getSketcherObj()
	{
		return mInternal->sketcher.get();
	}
    SketcherFeature::~SketcherFeature()
	{
		delete mInternal;
	}
	bool SketcherFeature::execute()
	{
		if (mInternal->sketcher.get()) {
			topoShape->setShape(mInternal->sketcher->getDoneFaceShape());
			return true;
	    }
        return false;
	}
}