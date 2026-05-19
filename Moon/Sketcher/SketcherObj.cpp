#include "Sketcher/SketcherObj.h"
#include "renderer/SceneView.h"
#include "Gizmo/Gizmo.h"
#include "Core/Global/ServiceLocator.h"
#include <TopoDS.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <ShapeFix_Wire.hxx>
#include <BRep_Builder.hxx>
namespace MOON {
    SketcherObj::SketcherObj()
    {
    }
    SketcherObj::~SketcherObj()
    {
    }
    void SketcherObj::setPlane(int p)
    {
        mPlane = p;        
        auto& view = GetService(Editor::Panels::SceneView);
        view.GetCameraController().EnableRotate(false);
        if (mPlane == 0) {
            view.LookAt({0,0,0},{1,0,0},1);
        }
        if (mPlane == 1) {
            view.LookAt({ 0,0,0 }, { 0,1,0 }, 1);
        }
        if (mPlane == 2) {
            view.LookAt({ 0,0,0 }, { 0,0,1 }, 1);
        }
    }
    int SketcherObj::getPlane()
    {
        return mPlane;
    }
    void SketcherObj::getPlaneNormal(double* p)
    {
        if (mPlane == 0) {
            p[0] = 1;
			p[1] = 0;
            p[2] = 0;
        }
        if (mPlane == 1) {
            p[0] = 0;
            p[1] = 1;
            p[2] = 0;
        }
        if (mPlane == 2) {
            p[0] = 0;
            p[1] = 0;
            p[2] = 1;
        }
    }
    void SketcherObj::draw() {
        if (InEdit()) {
            auto renderer=&Gizmo::instance();

            renderer->pushSize(3);
            renderer->pushColor({ 255,0,0,255 });
            renderer->drawLine2D({ 100,0 }, { -100,0 },static_cast<Plane2D>(mPlane));
            renderer->popColor();
            renderer->pushColor({255,0,255,0});
            renderer->drawLine2D({ 0,100 }, { 0,-100 }, static_cast<Plane2D>(mPlane));
            renderer->popColor();
            //renderer->drawCircle2D(m_internal->centerPoint,m_internal->radius);
            renderer->popSize();

        }
    }
    bool SketcherObj::InEdit() const
    {
        return isInEdit;
    }
    void SketcherObj::makeDone()
    {
        isInEdit = false;
        auto& view = GetService(Editor::Panels::SceneView);
        view.GetCameraController().EnableRotate(true);
    }
    void SketcherObj::addGeometry(std::unique_ptr<Part::Geometry> ptr)
    {
        mGeoList.push_back(std::move(ptr));
    }
    Part::TopoShape SketcherObj::toShape() const
    {
        Part::TopoShape result;
        std::list<TopoDS_Edge> edge_list;
        std::list<TopoDS_Wire> wires;
		for (const auto& geo : mGeoList) {
			auto shape = geo->toShape();
			if (shape.ShapeType() == TopAbs_EDGE) {
				edge_list.push_back(TopoDS::Edge(shape));
			}
		}
        // Hint: Use ShapeAnalysis_FreeBounds::ConnectEdgesToWires() as an alternative
   //
   // sort them together to wires
        while (!edge_list.empty()) {
            BRepBuilderAPI_MakeWire mkWire;
            // add and erase first edge
            mkWire.Add(edge_list.front());
            edge_list.pop_front();

            TopoDS_Wire new_wire = mkWire.Wire();  // current new wire

            // try to connect each edge to the wire, the wire is complete if no more edges are
            // connectible
            bool found = false;
            do {
                found = false;
                for (auto pE = edge_list.begin(); pE != edge_list.end(); ++pE) {
                    mkWire.Add(*pE);
                    if (mkWire.Error() != BRepBuilderAPI_DisconnectedWire) {
                        // edge added ==> remove it from list
                        found = true;
                        edge_list.erase(pE);
                        new_wire = mkWire.Wire();
                        break;
                    }
                }
            } while (found);

            // Fix any topological issues of the wire
            ShapeFix_Wire aFix;
            aFix.SetPrecision(Precision::Confusion());
            aFix.Load(new_wire);
            aFix.FixReorder();
            aFix.FixConnected();
            aFix.FixClosed();
            wires.push_back(aFix.Wire());
        }

        if (wires.size() == 1 ) {
            result = *wires.begin();
        }
        else if (wires.size() > 1 ) {
            BRep_Builder builder;
            TopoDS_Compound comp;
            builder.MakeCompound(comp);
            for (auto& wire : wires) {
                builder.Add(comp, wire);
            }
            result.setShape(comp);
        }
        if (mPlane == 0) {
			result.setTransform(Base::Matrix4D(
				0.0, 0.0, 1.0, 0.0,
				1.0, 0.0, 0.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 0.0, 1.0
			));
        }
        if (mPlane == 1) {
            result.setTransform(Base::Matrix4D(
                1.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 1.0
            ));
        }
        if (mPlane == 2) {
            result.setTransform(Base::Matrix4D(
                1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0
            ));
        }
        //return result.makeFace();
        return result;
    }
}