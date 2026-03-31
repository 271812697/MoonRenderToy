#pragma once
#include "base/Converter.h"
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
class gp_Lin;
class gp_Pln;
namespace Base {
    // Specialization for gp_Pnt
    template <>
    struct vec_traits<gp_Pnt> {
        using vec_type = gp_Pnt;
        using float_type = double;
        explicit vec_traits(const vec_type& v) : v(v) {}
        inline std::tuple<float_type, float_type, float_type> get() const {
            return std::make_tuple(v.X(), v.Y(), v.Z());
        }
    private:
        const vec_type& v;
    };
    // Specialization for gp_Vec
    template <>
    struct vec_traits<gp_Vec> {
        using vec_type = gp_Vec;
        using float_type = double;
        explicit vec_traits(const vec_type& v) : v(v) {}
        inline std::tuple<float_type, float_type, float_type> get() const {
            return std::make_tuple(v.X(), v.Y(), v.Z());
        }
    private:
        const vec_type& v;
    };
    // Specialization for gp_Dir
    template <>
    struct vec_traits<gp_Dir> {
        using vec_type = gp_Dir;
        using float_type = double;
        explicit vec_traits(const vec_type& v) : v(v) {}
        inline std::tuple<float_type, float_type, float_type> get() const {
            return std::make_tuple(v.X(), v.Y(), v.Z());
        }
    private:
        const vec_type& v;
    };
    // Specialization for gp_XYZ
    template <>
    struct vec_traits<gp_XYZ> {
        using vec_type = gp_XYZ;
        using float_type = double;
        explicit vec_traits(const vec_type& v) : v(v) {}
        inline std::tuple<float_type, float_type, float_type> get() const {
            return std::make_tuple(v.X(), v.Y(), v.Z());
        }
    private:
        const vec_type& v;
    };
}
namespace MOON {

    bool intersect(const gp_Pln& pln1, const gp_Pln& pln2, gp_Lin& lin);
    void closestPointsOnLines(const gp_Lin& lin1, const gp_Lin& lin2, gp_Pnt& p1, gp_Pnt& p2);
	class  Tools
	{
	public:
        static void getNormal(const Handle(Geom_Surface)& surf, double u, double v, const Standard_Real tol, gp_Dir& dir, Standard_Boolean& done);
        static bool getPolygon3D(const TopoDS_Edge& edge, std::vector<gp_Pnt>& points);
        static bool getPolygonOnTriangulation(const TopoDS_Edge& edge, const TopoDS_Face& face, std::vector<gp_Pnt>& points);
        static bool getTriangulation(const TopoDS_Face& face, std::vector<gp_Pnt>& points, std::vector<gp_Vec>& normals,std::vector<Poly_Triangle>& facets);
        static void getPointNormals(const TopoDS_Face& face, Handle(Poly_Triangulation) aPoly, TColgp_Array1OfDir& normals);
        static void getPointNormals(const TopoDS_Face& face, Handle(Poly_Triangulation) aPoly, std::vector<gp_Vec>& normals);
    };
}