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


class gp_Lin;
class gp_Pln;
namespace MOON {

	class  Tools
	{
	public:
    /*!
    * \brief getPolygon3D
    * \param edge
    * \param points
    * \return true if a polygon exists or false otherwise
    */
        static bool getPolygon3D(const TopoDS_Edge& edge, std::vector<gp_Pnt>& points);

        static bool getPolygonOnTriangulation(const TopoDS_Edge& edge, const TopoDS_Face& face, std::vector<gp_Pnt>& points);
        static bool getTriangulation(const TopoDS_Face& face, std::vector<gp_Pnt>& points, std::vector<Poly_Triangle>& facets);
    };
}