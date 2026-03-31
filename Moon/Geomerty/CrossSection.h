#pragma once
#include <list>
#include <TopTools_IndexedMapOfShape.hxx>

#include "TopoShape.h"


class TopoDS_Shape;
class TopoDS_Wire;

namespace MOON {

class  CrossSection
{
public:
    CrossSection(double a, double b, double c, const TopoDS_Shape& s);
    std::list<TopoDS_Wire> slice(double d) const;

private:
    void sliceNonSolid(double d, const TopoDS_Shape&, std::list<TopoDS_Wire>& wires) const;
    void sliceSolid(double d, const TopoDS_Shape&, std::list<TopoDS_Wire>& wires) const;
    void connectEdges (const std::list<TopoDS_Edge>& edges, std::list<TopoDS_Wire>& wires) const;
    void connectWires (const TopTools_IndexedMapOfShape& wireMap, std::list<TopoDS_Wire>& wires) const;
    TopoDS_Wire fixWire(const TopoDS_Wire& wire) const;
    std::list<TopoDS_Wire> removeDuplicates(const std::list<TopoDS_Wire>& wires) const;

private:
    double a,b,c;
    const TopoDS_Shape& s;
};

class  TopoCrossSection
{
public:
    TopoCrossSection(double a, double b, double c, const TopoShape& s, const char* op = 0);
    void slice(int idx, double d, std::vector<TopoShape>& wires) const;
    TopoShape slice(int idx, double d) const;

private:
    void sliceNonSolid(int idx, double d, const TopoShape&, std::vector<TopoShape>& wires) const;
    void sliceSolid(int idx, double d, const TopoShape&, std::vector<TopoShape>& wires) const;

private:
    double a, b, c;
    const TopoShape& shape;
    const char* op;
};

}  // namespace Part
