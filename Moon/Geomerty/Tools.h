#pragma once
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_XYZ.hxx>
#include <Geom_Surface.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_Triangle.hxx>
#include <Poly_Triangulation.hxx>
#include <TColgp_Array1OfDir.hxx>
#include <TColStd_ListOfTransient.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <vector>
namespace MOON {
	class  Tools
	{
	public:
        static bool getPolygon3D(const TopoDS_Edge& edge, std::vector<gp_Pnt>& points);
        static bool getPolygonOnTriangulation(const TopoDS_Edge& edge, const TopoDS_Face& face, std::vector<gp_Pnt>& points);
        static bool getTriangulation(const TopoDS_Face& face, std::vector<gp_Pnt>& points, std::vector<gp_Vec>& normals,std::vector<Poly_Triangle>& facets);
        static void getPointNormals(const TopoDS_Face& face, Handle(Poly_Triangulation) aPoly, TColgp_Array1OfDir& normals);
        static void getPointNormals(const TopoDS_Face& face, Handle(Poly_Triangulation) aPoly, std::vector<gp_Vec>& normals);
    };
}