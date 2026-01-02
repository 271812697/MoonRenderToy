#include <Standard_Version.hxx>
#include "Geomerty/Tools.h"
#include <BRep_Tool.hxx>
#include <gp_Vec3f.hxx>
namespace MOON {
	bool Tools::getPolygon3D(const TopoDS_Edge& edge, std::vector<gp_Pnt>& points)
	{
        TopLoc_Location loc;
        Handle(Poly_Polygon3D) hPoly = BRep_Tool::Polygon3D(edge, loc);
        if (hPoly.IsNull())
            return false;

        // getting the transformation of the edge
        gp_Trsf transf;
        bool identity = true;
        if (!loc.IsIdentity()) {
            identity = false;
            transf = loc.Transformation();
        }

        // getting size and create the array
        Standard_Integer nbNodes = hPoly->NbNodes();
        points.reserve(nbNodes);
        const TColgp_Array1OfPnt& nodes = hPoly->Nodes();

        for (int i = 1; i <= nbNodes; i++) {
            gp_Pnt p = nodes(i);

            // transform the vertices to the location of the face
            if (!identity) {
                p.Transform(transf);
            }

            points.push_back(p);
        }

        return true;
	}
    bool Tools::getPolygonOnTriangulation(const TopoDS_Edge& edge, const TopoDS_Face& face, std::vector<gp_Pnt>& points)
    {
        TopLoc_Location loc;
        Handle(Poly_Triangulation) hTria = BRep_Tool::Triangulation(face, loc);
        if (hTria.IsNull())
            return false;

        // this holds the indices of the edge's triangulation to the actual points
        Handle(Poly_PolygonOnTriangulation) hPoly = BRep_Tool::PolygonOnTriangulation(edge, hTria, loc);
        if (hPoly.IsNull())
            return false;

        // getting the transformation of the edge
        gp_Trsf transf;
        bool identity = true;
        if (!loc.IsIdentity()) {
            identity = false;
            transf = loc.Transformation();
        }

        // getting size and create the array
        Standard_Integer nbNodes = hPoly->NbNodes();
        points.reserve(nbNodes);
        const TColStd_Array1OfInteger& indices = hPoly->Nodes();
#if OCC_VERSION_HEX < 0x070600
        const TColgp_Array1OfPnt& Nodes = hTria->Nodes();
#endif

        // go through the index array
        for (Standard_Integer i = indices.Lower(); i <= indices.Upper(); i++) {
#if OCC_VERSION_HEX < 0x070600
            gp_Pnt p = Nodes(indices(i));
#else
            gp_Pnt p = hTria->Node(indices(i));
#endif
            if (!identity) {
                p.Transform(transf);
            }

            points.push_back(p);
        }

        return true;
    }
    bool Tools::getTriangulation(const TopoDS_Face& face, std::vector<gp_Pnt>& points, std::vector<Poly_Triangle>& facets)
    {
        TopLoc_Location loc;
        Handle(Poly_Triangulation) hTria = BRep_Tool::Triangulation(face, loc);
        if (hTria.IsNull())
            return false;

        // getting the transformation of the face
        gp_Trsf transf;
        bool identity = true;
        if (!loc.IsIdentity()) {
            identity = false;
            transf = loc.Transformation();
        }

        // check orientation
        TopAbs_Orientation orient = face.Orientation();

        Standard_Integer nbNodes = hTria->NbNodes();
       
        Standard_Integer nbTriangles = hTria->NbTriangles();
#if OCC_VERSION_HEX < 0x070600
        const TColgp_Array1OfPnt& nodes = hTria->Nodes();
        const Poly_Array1OfTriangle& triangles = hTria->Triangles();
#endif

        points.reserve(nbNodes);
        facets.reserve(nbTriangles);

        // cycling through the poly mesh
        for (int i = 1; i <= nbNodes; i++) {
#if OCC_VERSION_HEX < 0x070600
            gp_Pnt p = nodes(i);
#else
            gp_Pnt p = hTria->Node(i);
#endif
            // transform the vertices to the location of the face
            if (!identity) {
                p.Transform(transf);
                //nor.Transform(transf);
            }
            points.push_back(p);
        }
        for (int i = 1; i <= nbTriangles; i++) {
            // Get the triangle
            Standard_Integer n1, n2, n3;
#if OCC_VERSION_HEX < 0x070600
            triangles(i).Get(n1, n2, n3);
#else
            hTria->Triangle(i).Get(n1, n2, n3);
#endif
            --n1; --n2; --n3;
            // change orientation of the triangles
            if (orient != TopAbs_FORWARD) {
                std::swap(n1, n2);
            }
            facets.emplace_back(n1, n2, n3);
        }
        return true;
    }
}