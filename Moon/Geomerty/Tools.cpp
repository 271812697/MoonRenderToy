#include <Standard_Version.hxx>
#include "Geomerty/Tools.h"
#include <BRep_Tool.hxx>
#include <gp_Vec3f.hxx>
#include <Poly_Connect.hxx>
#include <TopoDS.hxx>
#include <Precision.hxx>
#include <GeomLib.hxx>
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
    bool Tools::getTriangulation(const TopoDS_Face& face, std::vector<gp_Pnt>& points, std::vector<gp_Vec>& normals , std::vector<Poly_Triangle>& facets)
    {
        
        TopLoc_Location loc;
        Handle(Poly_Triangulation) hTria = BRep_Tool::Triangulation(face, loc);
        if (hTria.IsNull())
            return false;
        getPointNormals(face,hTria,normals);
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
    void Tools::getPointNormals(const TopoDS_Face& theFace, Handle(Poly_Triangulation) aPolyTri, TColgp_Array1OfDir& theNormals)
    {
#if OCC_VERSION_HEX < 0x070600
        const TColgp_Array1OfPnt& aNodes = aPolyTri->Nodes();

        if (aPolyTri->HasNormals())
        {
            // normals pre-computed in triangulation structure
            const TShort_Array1OfShortReal& aNormals = aPolyTri->Normals();
            const Standard_ShortReal* aNormArr = &(aNormals.Value(aNormals.Lower()));

            for (Standard_Integer aNodeIter = aNodes.Lower(); aNodeIter <= aNodes.Upper(); ++aNodeIter)
            {
                const Standard_Integer anId = 3 * (aNodeIter - aNodes.Lower());
                const gp_Dir aNorm(aNormArr[anId + 0],
                    aNormArr[anId + 1],
                    aNormArr[anId + 2]);
                theNormals(aNodeIter) = aNorm;
            }

            if (theFace.Orientation() == TopAbs_REVERSED)
            {
                for (Standard_Integer aNodeIter = aNodes.Lower(); aNodeIter <= aNodes.Upper(); ++aNodeIter)
                {
                    theNormals.ChangeValue(aNodeIter).Reverse();
                }
            }
        }
        else {
            // take in face the surface location
            Poly_Connect thePolyConnect(aPolyTri);
            const TopoDS_Face      aZeroFace = TopoDS::Face(theFace.Located(TopLoc_Location()));
            Handle(Geom_Surface)   aSurf = BRep_Tool::Surface(aZeroFace);
            const Standard_Real    aTol = Precision::Confusion();
            Handle(TShort_HArray1OfShortReal) aNormals = new TShort_HArray1OfShortReal(1, aPolyTri->NbNodes() * 3);
            const Poly_Array1OfTriangle& aTriangles = aPolyTri->Triangles();
            const TColgp_Array1OfPnt2d* aNodesUV = aPolyTri->HasUVNodes() && !aSurf.IsNull()
                ? &aPolyTri->UVNodes()
                : nullptr;
            Standard_Integer aTri[3];

            for (Standard_Integer aNodeIter = aNodes.Lower(); aNodeIter <= aNodes.Upper(); ++aNodeIter)
            {
                // try to retrieve normal from real surface first, when UV coordinates are available
                if (!aNodesUV || GeomLib::NormEstim(aSurf, aNodesUV->Value(aNodeIter), aTol, theNormals(aNodeIter)) > 1)
                {
                    // compute flat normals
                    gp_XYZ eqPlan(0.0, 0.0, 0.0);

                    for (thePolyConnect.Initialize(aNodeIter); thePolyConnect.More(); thePolyConnect.Next())
                    {
                        aTriangles(thePolyConnect.Value()).Get(aTri[0], aTri[1], aTri[2]);
                        const gp_XYZ v1(aNodes(aTri[1]).Coord() - aNodes(aTri[0]).Coord());
                        const gp_XYZ v2(aNodes(aTri[2]).Coord() - aNodes(aTri[1]).Coord());
                        const gp_XYZ vv = v1 ^ v2;
                        const Standard_Real aMod = vv.Modulus();

                        if (aMod >= aTol)
                        {
                            eqPlan += vv / aMod;
                        }
                    }

                    const Standard_Real aModMax = eqPlan.Modulus();
                    theNormals(aNodeIter) = (aModMax > aTol) ? gp_Dir(eqPlan) : gp::DZ();
                }

                const Standard_Integer anId = (aNodeIter - aNodes.Lower()) * 3;
                aNormals->SetValue(anId + 1, (Standard_ShortReal)theNormals(aNodeIter).X());
                aNormals->SetValue(anId + 2, (Standard_ShortReal)theNormals(aNodeIter).Y());
                aNormals->SetValue(anId + 3, (Standard_ShortReal)theNormals(aNodeIter).Z());
            }

            aPolyTri->SetNormals(aNormals);

            if (theFace.Orientation() == TopAbs_REVERSED)
            {
                for (Standard_Integer aNodeIter = aNodes.Lower(); aNodeIter <= aNodes.Upper(); ++aNodeIter)
                {
                    theNormals.ChangeValue(aNodeIter).Reverse();
                }
            }
        }
#else
        Standard_Integer numNodes = aPolyTri->NbNodes();

        if (aPolyTri->HasNormals())
        {
            for (Standard_Integer aNodeIter = 1; aNodeIter <= numNodes; ++aNodeIter)
            {
                theNormals(aNodeIter) = aPolyTri->Normal(aNodeIter);
            }

            if (theFace.Orientation() == TopAbs_REVERSED)
            {
                for (Standard_Integer aNodeIter = 1; aNodeIter <= numNodes; ++aNodeIter)
                {
                    theNormals.ChangeValue(aNodeIter).Reverse();
                }
            }
        }
        else {
            // take in face the surface location
            Poly_Connect thePolyConnect(aPolyTri);
            const TopoDS_Face      aZeroFace = TopoDS::Face(theFace.Located(TopLoc_Location()));
            Handle(Geom_Surface)   aSurf = BRep_Tool::Surface(aZeroFace);
            const Standard_Real    aTol = Precision::Confusion();
            Standard_Boolean hasNodesUV = aPolyTri->HasUVNodes() && !aSurf.IsNull();
            Standard_Integer aTri[3];

            aPolyTri->AddNormals();
            for (Standard_Integer aNodeIter = 1; aNodeIter <= numNodes; ++aNodeIter)
            {
                // try to retrieve normal from real surface first, when UV coordinates are available
                if (!hasNodesUV || GeomLib::NormEstim(aSurf, aPolyTri->UVNode(aNodeIter), aTol, theNormals(aNodeIter)) > 1)
                {
                    // compute flat normals
                    gp_XYZ eqPlan(0.0, 0.0, 0.0);

                    for (thePolyConnect.Initialize(aNodeIter); thePolyConnect.More(); thePolyConnect.Next())
                    {
                        aPolyTri->Triangle(thePolyConnect.Value()).Get(aTri[0], aTri[1], aTri[2]);
                        const gp_XYZ v1(aPolyTri->Node(aTri[1]).Coord() - aPolyTri->Node(aTri[0]).Coord());
                        const gp_XYZ v2(aPolyTri->Node(aTri[2]).Coord() - aPolyTri->Node(aTri[1]).Coord());
                        const gp_XYZ vv = v1 ^ v2;
                        const Standard_Real aMod = vv.Modulus();

                        if (aMod >= aTol)
                        {
                            eqPlan += vv / aMod;
                        }
                    }

                    const Standard_Real aModMax = eqPlan.Modulus();
                    theNormals(aNodeIter) = (aModMax > aTol) ? gp_Dir(eqPlan) : gp::DZ();
                }

                aPolyTri->SetNormal(aNodeIter, theNormals(aNodeIter));
            }

            if (theFace.Orientation() == TopAbs_REVERSED)
            {
                for (Standard_Integer aNodeIter = 1; aNodeIter <= numNodes; ++aNodeIter)
                {
                    theNormals.ChangeValue(aNodeIter).Reverse();
                }
            }
        }
#endif
    }
    void Tools::getPointNormals(const TopoDS_Face& face, Handle(Poly_Triangulation) aPoly, std::vector<gp_Vec>& normals)
    {
        TColgp_Array1OfDir dirs(1, aPoly->NbNodes());
        getPointNormals(face, aPoly, dirs);
        normals.reserve(aPoly->NbNodes());

        for (int i = dirs.Lower(); i <= dirs.Upper(); ++i) {
            normals.emplace_back(dirs(i).XYZ());
        }
    }
}