
# include <algorithm>
# include <BRepAdaptor_Surface.hxx>
# include "FCBRepAlgoAPI_Common.h"
# include "FCBRepAlgoAPI_Cut.h"
# include "FCBRepAlgoAPI_Section.h"
# include <BRepBuilderAPI_MakeFace.hxx>
# include <BRepBuilderAPI_MakeWire.hxx>
# include <BRepPrimAPI_MakeHalfSpace.hxx>
# include <gp_Pln.hxx>
# include <Precision.hxx>
# include <ShapeAnalysis_FreeBounds.hxx>
# include <ShapeFix_Wire.hxx>
# include <TopExp.hxx>
# include <TopExp_Explorer.hxx>
# include <TopTools_HSequenceOfShape.hxx>
# include <TopTools_IndexedMapOfShape.hxx>
# include <TopoDS.hxx>
# include <TopoDS_Edge.hxx>
# include <TopoDS_Wire.hxx>


#include "CrossSection.h"
namespace MOON {
    const std::string indexSuffix(int index, const char* label)
    {
        if (index < 2) {  // Don't add a suffix for item #1, begin appending at 2
            return { "" };
        }
        std::string name(label);
        name += std::to_string(index);
        return name;
    }
    class OpCodes {
    public:
        /** Element name encoding scheme version number
         *
         * Increase this version if there is major change in encoding scheme.
         * Opening a document containing a mismatched version number will cause the
         * element map to be regenerated after recompute
         */

        static constexpr const int Version = 15;

        /** @name Op codes that are also accepted by TopoShape::makEBoolean() */
        //@{
        static constexpr const char* Fuse = "FUS";
        static constexpr const char* Cut = "CUT";
        static constexpr const char* Common = "CMN";
        static constexpr const char* Section = "SEC";
        static constexpr const char* Compound = "CMP";
        static constexpr const char* Compsolid = "CSD";
        static constexpr const char* Pipe = "PIP";
        static constexpr const char* Shell = "SHL";
        static constexpr const char* Wire = "WIR";
        //@}

        static constexpr const char* Tag = "TAG";
        static constexpr const char* Copy = "CPY";
        static constexpr const char* Transform = "XFM";
        static constexpr const char* Gtransform = "GFM";
        static constexpr const char* Face = "FAC";
        static constexpr const char* FilledFace = "FFC";
        static constexpr const char* Extrude = "XTR";
        static constexpr const char* GeneralFuse = "GFS";
        static constexpr const char* Refine = "RFI";
        static constexpr const char* Boolean = "BOL";
        static constexpr const char* Slice = "SLC";
        static constexpr const char* Maker = "MAK";
        static constexpr const char* Fillet = "FLT";
        static constexpr const char* Chamfer = "CHF";
        static constexpr const char* Thicken = "THK";
        static constexpr const char* Offset = "OFS";
        static constexpr const char* Offset2D = "OFF";
        static constexpr const char* Revolve = "RVL";
        static constexpr const char* Loft = "LFT";
        static constexpr const char* Sweep = "SWP";
        static constexpr const char* PipeShell = "PSH";
        static constexpr const char* ShellFill = "SHF";
        static constexpr const char* Solid = "SLD";
        static constexpr const char* RuledSurface = "RSF";
        static constexpr const char* Mirror = "MIR";
        static constexpr const char* Sketch = "SKT";
        static constexpr const char* SketchExport = "SKE";
        static constexpr const char* Shapebinder = "BND";
        static constexpr const char* ThruSections = "TRU";
        static constexpr const char* Sewing = "SEW";
        static constexpr const char* Prism = "PSM";
        static constexpr const char* Draft = "DFT";
        static constexpr const char* HalfSpace = "HSP";
        static constexpr const char* BSplineFace = "BSF";
        static constexpr const char* Split = "SPT";
        static constexpr const char* Evolve = "EVO";
    };

}

using namespace MOON;

CrossSection::CrossSection(double a, double b, double c, const TopoDS_Shape& s)
  : a(a), b(b), c(c), s(s)
{
}

std::list<TopoDS_Wire> CrossSection::slice(double d) const
{
    std::list<TopoDS_Wire> wires;
    // Fixes: 0001228: Cross section of Torus in Part Workbench fails or give wrong results
    // Fixes: 0001137: Incomplete slices when using Part.slice on a torus
    TopExp_Explorer xp;
    for (xp.Init(s, TopAbs_SOLID); xp.More(); xp.Next()) {
        sliceSolid(d, xp.Current(), wires);
    }
    for (xp.Init(s, TopAbs_SHELL, TopAbs_SOLID); xp.More(); xp.Next()) {
        sliceNonSolid(d, xp.Current(), wires);
    }
    for (xp.Init(s, TopAbs_FACE, TopAbs_SHELL); xp.More(); xp.Next()) {
        sliceNonSolid(d, xp.Current(), wires);
    }

    return removeDuplicates(wires);
}

std::list<TopoDS_Wire> CrossSection::removeDuplicates(const std::list<TopoDS_Wire>& wires) const
{
    std::list<TopoDS_Wire> wires_reduce;
    for (const auto& wire : wires) {
        TopTools_IndexedMapOfShape mapOfEdges1;
        TopExp::MapShapes(wire, TopAbs_EDGE, mapOfEdges1);

        // The wires are independent shapes but their edges might be shared
        auto it = std::find_if(wires_reduce.begin(), wires_reduce.end(), [&mapOfEdges1](const TopoDS_Wire& w) {
            // same TShape and same placement but different orientation
            TopTools_IndexedMapOfShape mapOfEdges2;
            TopExp::MapShapes(w, TopAbs_EDGE, mapOfEdges2);
            int numEdges1 = mapOfEdges1.Extent();
            int numEdges2 = mapOfEdges2.Extent();
            if (numEdges1 != numEdges2)
                return false;

            TopTools_IndexedMapOfShape::Iterator it1(mapOfEdges1);
            TopTools_IndexedMapOfShape::Iterator it2(mapOfEdges2);
            for (; it1.More() && it2.More(); it1.Next(), it2.Next()) {
                if (!it1.Value().IsSame(it2.Value()))
                    return false;
            }

            return true;
        });

        if (it == wires_reduce.end()) {
            wires_reduce.push_back(wire);
        }
    }
    return wires_reduce;
}

void CrossSection::sliceNonSolid(double d, const TopoDS_Shape& shape, std::list<TopoDS_Wire>& wires) const
{
    FCBRepAlgoAPI_Section cs(shape, gp_Pln(a,b,c,-d));
    if (cs.IsDone()) {
        std::list<TopoDS_Edge> edges;
        TopExp_Explorer xp;
        for (xp.Init(cs.Shape(), TopAbs_EDGE); xp.More(); xp.Next())
            edges.push_back(TopoDS::Edge(xp.Current()));
        connectEdges(edges, wires);
    }
}

void CrossSection::sliceSolid(double d, const TopoDS_Shape& shape, std::list<TopoDS_Wire>& wires) const
{
    gp_Pln slicePlane(a,b,c,-d);
    BRepBuilderAPI_MakeFace mkFace(slicePlane);
    TopoDS_Face face = mkFace.Face();

    // Make sure to choose a point that does not lie on the plane (fixes #0001228)
    gp_Vec tempVector(a,b,c);
    tempVector.Normalize();//just in case.
    tempVector *= (d+1.0);
    gp_Pnt refPoint(0.0, 0.0, 0.0);
    refPoint.Translate(tempVector);

    BRepPrimAPI_MakeHalfSpace mkSolid(face, refPoint);
    TopoDS_Solid solid = mkSolid.Solid();
    FCBRepAlgoAPI_Cut mkCut(shape, solid);

    if (mkCut.IsDone()) {
        TopTools_IndexedMapOfShape mapOfFaces;
        TopExp::MapShapes(mkCut.Shape(), TopAbs_FACE, mapOfFaces);
        for (int i=1; i<=mapOfFaces.Extent(); i++) {
            const TopoDS_Face& face = TopoDS::Face(mapOfFaces.FindKey(i));
            BRepAdaptor_Surface adapt(face);
            if (adapt.GetType() == GeomAbs_Plane) {
                gp_Pln plane = adapt.Plane();
                if (plane.Axis().IsParallel(slicePlane.Axis(), Precision::Confusion()) &&
                    plane.Distance(slicePlane.Location()) < Precision::Confusion()) {
                    // sort and repair the wires
                    TopTools_IndexedMapOfShape mapOfWires;
                    TopExp::MapShapes(face, TopAbs_WIRE, mapOfWires);
                    connectWires(mapOfWires, wires);
                }
            }
        }
    }
}

void CrossSection::connectEdges (const std::list<TopoDS_Edge>& edges, std::list<TopoDS_Wire>& wires) const
{
    // Hint: Use ShapeAnalysis_FreeBounds::ConnectEdgesToWires() as an alternative
    std::list<TopoDS_Edge> edge_list = edges;
    while (!edge_list.empty()) {
        BRepBuilderAPI_MakeWire mkWire;
        // add and erase first edge
        mkWire.Add(edge_list.front());
        edge_list.erase(edge_list.begin());

        TopoDS_Wire new_wire = mkWire.Wire();  // current new wire

        // try to connect each edge to the wire, the wire is complete if no more edges are connectible
        bool found = false;
        do {
            found = false;
            for (std::list<TopoDS_Edge>::iterator pE = edge_list.begin(); pE != edge_list.end();++pE) {
                mkWire.Add(*pE);
                if (mkWire.Error() != BRepBuilderAPI_DisconnectedWire) {
                    // edge added ==> remove it from list
                    found = true;
                    edge_list.erase(pE);
                    new_wire = mkWire.Wire();
                    break;
                }
            }
        }
        while (found);

        // Fix any topological issues of the wire
        wires.push_back(fixWire(new_wire));
    }
}

void CrossSection::connectWires (const TopTools_IndexedMapOfShape& wireMap, std::list<TopoDS_Wire>& wires) const
{
    Handle(TopTools_HSequenceOfShape) hWires = new TopTools_HSequenceOfShape();
    for (int i=1; i<=wireMap.Extent(); i++) {
        const TopoDS_Shape& wire = wireMap.FindKey(i);
        hWires->Append(wire);
    }

    Handle(TopTools_HSequenceOfShape) hSorted = new TopTools_HSequenceOfShape();
    ShapeAnalysis_FreeBounds::ConnectWiresToWires(hWires, Precision::Confusion(), false, hSorted);

    for (int i=1; i<=hSorted->Length(); i++) {
        const TopoDS_Wire& new_wire = TopoDS::Wire(hSorted->Value(i));
        // Fix any topological issues of the wire
        wires.push_back(fixWire(new_wire));
    }
}

TopoDS_Wire CrossSection::fixWire(const TopoDS_Wire& wire) const
{
    // Fix any topological issues of the wire
    ShapeFix_Wire aFix;
    aFix.SetPrecision(Precision::Confusion());
    aFix.Load(wire);
    aFix.FixReorder();
    aFix.FixConnected();
    aFix.FixClosed();
    return aFix.Wire();
}

TopoCrossSection::TopoCrossSection(double a, double b, double c, const TopoShape& s, const char *op)
    : a(a), b(b), c(c), shape(s), op(op?op:OpCodes::Slice)
{
}

void TopoCrossSection::slice(int idx, double d, std::vector<MOON::TopoShape>& wires) const
{
    // Fixes: 0001228: Cross section of Torus in Part Workbench fails or give wrong results
    // Fixes: 0001137: Incomplete slices when using Part.slice on a torus
    bool found = false;
    for (auto& s : shape.getSubTopoShapes(TopAbs_SOLID)) {
        sliceSolid(idx, d, s, wires);
        found = true;
    }
    if (!found) {
        for (auto& s : shape.getSubTopoShapes(TopAbs_SHELL)) {
            sliceNonSolid(idx, d, s, wires);
            found = true;
        }
        if (!found) {
            for (auto& s : shape.getSubTopoShapes(TopAbs_FACE)) {
                sliceNonSolid(idx, d, s, wires);
            }
        }
    }
}

TopoShape TopoCrossSection::slice(int idx, double d) const
{
    std::vector<TopoShape> wires;
    slice(idx, d, wires);
    return TopoShape().makeElementCompound(
        wires,
        0,
        TopoShape::SingleShapeCompoundCreationPolicy::returnShape);
}

void TopoCrossSection::sliceNonSolid(int idx,
                                     double d,
                                     const TopoShape& shape,
                                     std::vector<TopoShape>& wires) const
{
    FCBRepAlgoAPI_Section cs(shape.getShape(), gp_Pln(a, b, c, -d));
    if (cs.IsDone()) {
        std::string prefix(op);
        prefix += indexSuffix(idx,"_");
        auto res = TopoShape()
                       .makeElementShape(cs, shape, prefix.c_str())
                       .makeElementWires()
                       .getSubTopoShapes(TopAbs_WIRE);
        wires.insert(wires.end(), res.begin(), res.end());
    }
}

void TopoCrossSection::sliceSolid(int idx,
                                  double d,
                                  const TopoShape& shape,
                                  std::vector<TopoShape>& wires) const
{
    gp_Pln slicePlane(a, b, c, -d);
    BRepBuilderAPI_MakeFace mkFace(slicePlane);
    TopoShape face(idx);
    face.setShape(mkFace.Face());

    // Make sure to choose a point that does not lie on the plane (fixes #0001228)
    gp_Vec tempVector(a, b, c);
    tempVector.Normalize();  // just in case.
    tempVector *= (d + 1.0);
    gp_Pnt refPoint(0.0, 0.0, 0.0);
    refPoint.Translate(tempVector);

    BRepPrimAPI_MakeHalfSpace mkSolid(TopoDS::Face(face.getShape()), refPoint);
    TopoShape solid(idx);
    std::string prefix(op);
    prefix += indexSuffix(idx,"_");
    solid.makeElementShape(mkSolid, face, prefix.c_str());
    FCBRepAlgoAPI_Cut mkCut(shape.getShape(), solid.getShape());

    if (mkCut.IsDone()) {
        TopoShape res(shape.Tag, shape.Hasher);
        std::vector<TopoShape> shapes;
        shapes.push_back(shape);
        shapes.push_back(solid);
        res.makeElementShape(mkCut, shapes, prefix.c_str());
        for (auto& face : res.getSubTopoShapes(TopAbs_FACE)) {
            BRepAdaptor_Surface adapt(TopoDS::Face(face.getShape()));
            if (adapt.GetType() == GeomAbs_Plane) {
                gp_Pln plane = adapt.Plane();
                if (plane.Axis().IsParallel(slicePlane.Axis(), Precision::Confusion())
                    && plane.Distance(slicePlane.Location()) < Precision::Confusion()) {
                    auto repaired_wires = TopoShape(face.Tag)
                                              .makeElementWires(face.getSubTopoShapes(TopAbs_EDGE),
                                                                prefix.c_str(),
                                                                true)
                                              .getSubTopoShapes(TopAbs_WIRE);
                    wires.insert(wires.end(), repaired_wires.begin(), repaired_wires.end());
                }
            }
        }
    }
}
