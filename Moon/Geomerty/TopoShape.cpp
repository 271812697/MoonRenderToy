#include <Standard_Version.hxx>
#include "Geomerty/TopoShape.h"
#include "Geomerty/Tools.h"
#include "Geomerty/BRepMesh.h"
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <GProp_GProps.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepGProp.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <BRepClass_FaceClassifier.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <GeomLib.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <map>
namespace MOON {
	namespace {
		inline double defaultAngularDeflection(double linearTolerance) {
			// Default OCC angular deflection is 0.5 radians, or about 28.6 degrees.
			// That is a bit coarser than necessary for performance, so we default to at
			// most 0.1 radians, or 5.7 degrees. We also do not go finer than 0.005, or
			// roughly 0.28 degree angular resolution, to avoid performance tanking
			// completely at very fine resolutions.
			return std::min(0.1, linearTolerance * 5 + 0.005);
		}
		bool getShapeProperties(const TopoDS_Shape& shape, GProp_GProps& prop)
		{
			TopExp_Explorer xpSolid(shape, TopAbs_SOLID);
			if (xpSolid.More()) {
				BRepGProp::VolumeProperties(shape, prop);
				return true;
			}

			TopExp_Explorer xpFace(shape, TopAbs_FACE);
			if (xpFace.More()) {
				BRepGProp::SurfaceProperties(shape, prop);
				return true;
			}

			TopExp_Explorer xpEdge(shape, TopAbs_EDGE);
			if (xpEdge.More()) {
				BRepGProp::LinearProperties(shape, prop);
				return true;
			}

			TopExp_Explorer xpVert(shape, TopAbs_VERTEX);
			if (xpVert.More()) {
				gp_Pnt pnts;
				int count = 0;
				for (; xpVert.More(); xpVert.Next()) {
					count++;
					gp_Pnt pnt = BRep_Tool::Pnt(TopoDS::Vertex(xpVert.Current()));
					pnts.SetX(pnts.X() + pnt.X());
					pnts.SetY(pnts.Y() + pnt.Y());
					pnts.SetZ(pnts.Z() + pnt.Z());
				}

				pnts.SetX(pnts.X() / count);
				pnts.SetY(pnts.Y() / count);
				pnts.SetZ(pnts.Z() / count);
				prop = GProp_GProps(pnts);

				return true;
			}

			return false;
		}
	}
	TopoShape::TopoShape(const TopoDS_Shape& shape):_Shape(*this, shape)
	{
	}
	TopoShape::~TopoShape()
	{
	}
	double TopoShape::getAccuracy() const
	{
		double deviation = 0.2;
		Rendering::Geometry::bbox bbox = getBoundBox();
		if (bbox.isValid())
			return ((bbox.lengthX() + bbox.lengthY() + bbox.lengthZ()) / 300.0 * deviation);
		return 0.0;
	}
	void TopoShape::getPoints(std::vector<Vector3d>& Points, std::vector<Vector3d>& Normals, double Accuracy, uint16_t flags) const
	{
		if (_Shape.IsNull())
			return;

		const int minPointsPerEdge = 30;
		const double lateralDistance = Accuracy;

		// get all 3d points from free vertices
		for (TopExp_Explorer xp(_Shape, TopAbs_VERTEX, TopAbs_EDGE); xp.More(); xp.Next()) {
			gp_Pnt p = BRep_Tool::Pnt(TopoDS::Vertex(xp.Current()));
			Points.push_back({p.X(),p.Y(),p.Z()});
			Normals.emplace_back(0, 0, 0);
		}

		// sample inner points of all free edges
		for (TopExp_Explorer xp(_Shape, TopAbs_EDGE, TopAbs_FACE); xp.More(); xp.Next()) {
			BRepAdaptor_Curve curve(TopoDS::Edge(xp.Current()));
			GCPnts_UniformAbscissa discretizer(curve, lateralDistance, curve.FirstParameter(), curve.LastParameter());
			if (discretizer.IsDone() && discretizer.NbPoints() > 0) {
				int nbPoints = discretizer.NbPoints();
				for (int i = 1; i <= nbPoints; i++) {
					gp_Pnt p = curve.Value(discretizer.Parameter(i));
					Points.push_back({ p.X(),p.Y(),p.Z() });
					Normals.emplace_back(0, 0, 0);
				}
			}
		}

		// sample inner points of all faces
		BRepClass_FaceClassifier classifier;
		bool hasFaces = false;
		for (TopExp_Explorer xp(_Shape, TopAbs_FACE); xp.More(); xp.Next()) {
			hasFaces = true;
			int pointsPerEdge = minPointsPerEdge;
			TopoDS_Face face = TopoDS::Face(xp.Current());
			BRepAdaptor_Surface surface(face);
			Handle(Geom_Surface) aSurf = BRep_Tool::Surface(face);

			// parameter ranges
			Standard_Real uFirst = surface.FirstUParameter();
			Standard_Real uLast = surface.LastUParameter();
			Standard_Real uMid = (uFirst + uLast) / 2;
			Standard_Real vFirst = surface.FirstVParameter();
			Standard_Real vLast = surface.LastVParameter();
			Standard_Real vMid = (vFirst + vLast) / 2;

			// get geometrical length and width of the surface
			//
			gp_Pnt p1, p2;
			Standard_Real fLengthU = 0.0, fLengthV = 0.0;
			for (int i = 1; i <= pointsPerEdge; i++) {
				double u1 = static_cast<double>(i - 1) / static_cast<double>(pointsPerEdge);
				double s1 = (1.0 - u1) * uFirst + u1 * uLast;
				p1 = surface.Value(s1, vMid);

				double u2 = static_cast<double>(i) / static_cast<double>(pointsPerEdge);
				double s2 = (1.0 - u2) * uFirst + u2 * uLast;
				p2 = surface.Value(s2, vMid);

				fLengthU += p1.Distance(p2);
			}

			for (int i = 1; i <= pointsPerEdge; i++) {
				double v1 = static_cast<double>(i - 1) / static_cast<double>(pointsPerEdge);
				double t1 = (1.0 - v1) * vFirst + v1 * vLast;
				p1 = surface.Value(uMid, t1);

				double v2 = static_cast<double>(i) / static_cast<double>(pointsPerEdge);
				double t2 = (1.0 - v2) * vFirst + v2 * vLast;
				p2 = surface.Value(uMid, t2);

				fLengthV += p1.Distance(p2);
			}

			int uPointsPerEdge = static_cast<int>(fLengthU / lateralDistance);
			int vPointsPerEdge = static_cast<int>(fLengthV / lateralDistance);
			uPointsPerEdge = std::max(uPointsPerEdge, 1);
			vPointsPerEdge = std::max(vPointsPerEdge, 1);

			for (int i = 0; i <= uPointsPerEdge; i++) {
				double u = static_cast<double>(i) / static_cast<double>(uPointsPerEdge);
				double s = (1.0 - u) * uFirst + u * uLast;

				for (int j = 0; j <= vPointsPerEdge; j++) {
					double v = static_cast<double>(j) / static_cast<double>(vPointsPerEdge);
					double t = (1.0 - v) * vFirst + v * vLast;

					gp_Pnt2d p2d(s, t);
					classifier.Perform(face, p2d, 1.0e-4);
					if (classifier.State() == TopAbs_IN || classifier.State() == TopAbs_ON) {
						gp_Pnt p = surface.Value(s, t);
						Points.push_back({ p.X(),p.Y(),p.Z() });
						gp_Dir normal;
						if (GeomLib::NormEstim(aSurf, p2d, Precision::Confusion(), normal) <= 1) {
							if (face.Orientation() == TopAbs_REVERSED)
								normal.Reverse();
							Normals.push_back({ normal.X(),normal.Y(),normal.Z() });
						}
						else {
							Normals.emplace_back(0, 0, 0);
						}
					}
				}
			}
		}

		// if no faces are found then the normals can be cleared
		if (!hasFaces)
			Normals.clear();
	}
	void TopoShape::getFaces(std::vector<Vector3d>& Points, std::vector<Vector3d>& Normals, std::vector<unsigned int>&indices,double Accuracy, uint16_t flags) const {
		if (_Shape.IsNull())
			return;

		const int minPointsPerEdge = 30;
		const double lateralDistance = Accuracy;


		// sample inner points of all faces
		BRepClass_FaceClassifier classifier;
		bool hasFaces = false;
		for (TopExp_Explorer xp(_Shape, TopAbs_FACE); xp.More(); xp.Next()) {
			hasFaces = true;
			int pointsPerEdge = minPointsPerEdge;
			TopoDS_Face face = TopoDS::Face(xp.Current());
			BRepAdaptor_Surface surface(face);
			Handle(Geom_Surface) aSurf = BRep_Tool::Surface(face);

			// parameter ranges
			Standard_Real uFirst = surface.FirstUParameter();
			Standard_Real uLast = surface.LastUParameter();
			Standard_Real uMid = (uFirst + uLast) / 2;
			Standard_Real vFirst = surface.FirstVParameter();
			Standard_Real vLast = surface.LastVParameter();
			Standard_Real vMid = (vFirst + vLast) / 2;

			// get geometrical length and width of the surface
			//
			gp_Pnt p1, p2;
			Standard_Real fLengthU = 0.0, fLengthV = 0.0;
			for (int i = 1; i <= pointsPerEdge; i++) {
				double u1 = static_cast<double>(i - 1) / static_cast<double>(pointsPerEdge);
				double s1 = (1.0 - u1) * uFirst + u1 * uLast;
				p1 = surface.Value(s1, vMid);

				double u2 = static_cast<double>(i) / static_cast<double>(pointsPerEdge);
				double s2 = (1.0 - u2) * uFirst + u2 * uLast;
				p2 = surface.Value(s2, vMid);

				fLengthU += p1.Distance(p2);
			}

			for (int i = 1; i <= pointsPerEdge; i++) {
				double v1 = static_cast<double>(i - 1) / static_cast<double>(pointsPerEdge);
				double t1 = (1.0 - v1) * vFirst + v1 * vLast;
				p1 = surface.Value(uMid, t1);

				double v2 = static_cast<double>(i) / static_cast<double>(pointsPerEdge);
				double t2 = (1.0 - v2) * vFirst + v2 * vLast;
				p2 = surface.Value(uMid, t2);

				fLengthV += p1.Distance(p2);
			}

			int uPointsPerEdge = static_cast<int>(fLengthU / lateralDistance);
			int vPointsPerEdge = static_cast<int>(fLengthV / lateralDistance);
			uPointsPerEdge = std::max(uPointsPerEdge, 1);
			vPointsPerEdge = std::max(vPointsPerEdge, 1);

			for (int i = 0; i < uPointsPerEdge; i++) {
				double u = static_cast<double>(i) / static_cast<double>(uPointsPerEdge);
				double u1 = static_cast<double>(i+1) / static_cast<double>(uPointsPerEdge);
				double s = (1.0 - u) * uFirst + u * uLast;
				double s1= (1.0 - u1) * uFirst + u1 * uLast;

				for (int j = 0; j < vPointsPerEdge; j++) {
					double v = static_cast<double>(j) / static_cast<double>(vPointsPerEdge);
					double v1 = static_cast<double>(j+1) / static_cast<double>(vPointsPerEdge);
					double t = (1.0 - v) * vFirst + v * vLast;
					double t1 = (1.0 - v1) * vFirst + v1 * vLast;

					gp_Pnt2d p2d(s, t);
					gp_Pnt2d p2d1(s1, t);
					gp_Pnt2d p2d2(s1, t1);
					gp_Pnt2d p2d3(s, t1);
					classifier.Perform(face, p2d, 1.0e-4);
					bool flag = (classifier.State() == TopAbs_IN || classifier.State() == TopAbs_ON);
					classifier.Perform(face, p2d1, 1.0e-4);
					bool flag1 = (classifier.State() == TopAbs_IN || classifier.State() == TopAbs_ON);
					classifier.Perform(face, p2d2, 1.0e-4);
					bool flag2 = (classifier.State() == TopAbs_IN || classifier.State() == TopAbs_ON);
					classifier.Perform(face, p2d3, 1.0e-4);
					bool flag3 = (classifier.State() == TopAbs_IN || classifier.State() == TopAbs_ON);

					if (flag&&flag1&&flag2&&flag3) {
						int vertexOffset = Points.size();
						gp_Pnt p = surface.Value(s, t);
						gp_Pnt p1 = surface.Value(s1, t);
						gp_Pnt p2 = surface.Value(s1, t1);
						gp_Pnt p3 = surface.Value(s, t1);
						indices.push_back(vertexOffset);
						indices.push_back(vertexOffset+2);
						indices.push_back(vertexOffset+3);
						indices.push_back(vertexOffset);
						indices.push_back(vertexOffset + 1);
						indices.push_back(vertexOffset + 2);
						Points.push_back({ p.X(),p.Y(),p.Z() });
						Points.push_back({ p1.X(),p1.Y(),p1.Z() });
						Points.push_back({ p2.X(),p2.Y(),p2.Z() });
						Points.push_back({ p3.X(),p3.Y(),p3.Z() });
						gp_Dir normal;
						if (GeomLib::NormEstim(aSurf, p2d, Precision::Confusion(), normal) <= 1) {
							if (face.Orientation() == TopAbs_REVERSED)
								normal.Reverse();
							Normals.push_back({ normal.X(),normal.Y(),normal.Z() });
						}
						else {
							Normals.emplace_back(0, 0, 0);
						}
						gp_Dir normal1;
						if (GeomLib::NormEstim(aSurf, p2d, Precision::Confusion(), normal1) <= 1) {
							if (face.Orientation() == TopAbs_REVERSED)
								normal1.Reverse();
							Normals.push_back({ normal1.X(),normal1.Y(),normal1.Z() });
						}
						else {
							Normals.emplace_back(0, 0, 0);
						}
						gp_Dir normal2;
						if (GeomLib::NormEstim(aSurf, p2d, Precision::Confusion(), normal2) <= 1) {
							if (face.Orientation() == TopAbs_REVERSED)
								normal2.Reverse();
							Normals.push_back({ normal2.X(),normal2.Y(),normal2.Z() });
						}
						else {
							Normals.emplace_back(0, 0, 0);
						}
						gp_Dir normal3;
						if (GeomLib::NormEstim(aSurf, p2d, Precision::Confusion(), normal3) <= 1) {
							if (face.Orientation() == TopAbs_REVERSED)
								normal3.Reverse();
							Normals.push_back({ normal3.X(),normal3.Y(),normal3.Z() });
						}
						else {
							Normals.emplace_back(0, 0, 0);
						}
					}
				}
			}
		}
	}
	void TopoShape::getLines(std::vector<Vector3d>& Points, std::vector<Line>& lines, double Accuracy, uint16_t flags) const
	{
		getLinesFromSubShape(_Shape, Points, lines);
	}
	void TopoShape::getFaces(std::vector<Vector3d>& Points, std::vector<Facet>& faces, double accuracy, uint16_t flags) const
	{
		if (this->_Shape.IsNull())
			return;

		// get the meshes of all faces and then merge them
		BRepMesh_IncrementalMesh aMesh(this->_Shape, accuracy,
			/*isRelative*/ Standard_False,
			/*theAngDeflection*/
			defaultAngularDeflection(accuracy),
			/*isInParallel*/ true);
		std::vector<Domain> domains;
		getDomains(domains);
		getFacesFromDomains(domains, Points, faces);
	}
	void TopoShape::setFaces(const std::vector<Vector3d>& Points, const std::vector<Facet>& Topo, double tolerance)
	{
		gp_XYZ p1, p2, p3;
		std::vector<TopoDS_Vertex> Vertexes;
		std::map<std::pair<uint32_t, uint32_t>, TopoDS_Edge> Edges;
		TopoDS_Face newFace;
		TopoDS_Wire newWire;
		Standard_Real x1, y1, z1;
		Standard_Real x2, y2, z2;
		Standard_Real x3, y3, z3;

		TopoDS_Compound aComp;
		BRep_Builder BuildTool;
		BuildTool.MakeCompound(aComp);

		uint32_t ctPoints = Points.size();
		Vertexes.resize(ctPoints);

		// Create array of vertexes
		auto CreateVertex = [](const Vector3d& v) {
			gp_XYZ p(v.x(), v.y(), v.z());
			return BRepBuilderAPI_MakeVertex(p);
			};
		for (const auto& it : Topo) {
			if (it.I1 < ctPoints) {
				if (Vertexes[it.I1].IsNull())
					Vertexes[it.I1] = CreateVertex(Points[it.I1]);
			}
			if (it.I2 < ctPoints) {
				if (Vertexes[it.I2].IsNull())
					Vertexes[it.I2] = CreateVertex(Points[it.I2]);
			}
			if (it.I3 < ctPoints) {
				if (Vertexes[it.I3].IsNull())
					Vertexes[it.I3] = CreateVertex(Points[it.I3]);
			}
		}

		// Create map of edges
		auto CreateEdge = [&Vertexes, &Edges](uint32_t p1, uint32_t p2) {
			// First check if the edge of a neighbour facet already exists
			// The point indices must be flipped.
			auto key1 = std::make_pair(p2, p1);
			auto key2 = std::make_pair(p1, p2);
			auto it = Edges.find(key1);
			if (it != Edges.end()) {
				TopoDS_Edge edge = it->second;
				edge.Reverse();
				Edges[key2] = edge;
			}
			else {
				BRepBuilderAPI_MakeEdge mkEdge(Vertexes[p1], Vertexes[p2]);
				if (mkEdge.IsDone())
					Edges[key2] = mkEdge.Edge();
			}
			};
		auto GetEdge = [&Edges](uint32_t p1, uint32_t p2) {
			auto key = std::make_pair(p1, p2);
			return Edges[key];
			};
		for (const auto& it : Topo) {
			CreateEdge(it.I1, it.I2);
			CreateEdge(it.I2, it.I3);
			CreateEdge(it.I3, it.I1);
		}

		for (const auto& it : Topo) {
			if (it.I1 >= ctPoints || it.I2 >= ctPoints || it.I3 >= ctPoints)
				continue;
			x1 = Points[it.I1].x(); y1 = Points[it.I1].y(); z1 = Points[it.I1].z();
			x2 = Points[it.I2].x(); y2 = Points[it.I2].y(); z2 = Points[it.I2].z();
			x3 = Points[it.I3].x(); y3 = Points[it.I3].y(); z3 = Points[it.I3].z();

			p1.SetCoord(x1, y1, z1);
			p2.SetCoord(x2, y2, z2);
			p3.SetCoord(x3, y3, z3);

			// Avoid very tiny edges as this may result into broken faces. The tolerance is Approximation
			// because Confusion might be too tight.
			if ((!(p1.IsEqual(p2, Precision::Approximation()))) && (!(p1.IsEqual(p3, Precision::Approximation())))) {
				const TopoDS_Edge& e1 = GetEdge(it.I1, it.I2);
				const TopoDS_Edge& e2 = GetEdge(it.I2, it.I3);
				const TopoDS_Edge& e3 = GetEdge(it.I3, it.I1);
				if (e1.IsNull() || e2.IsNull() || e3.IsNull())
					continue;

				newWire = BRepBuilderAPI_MakeWire(e1, e2, e3);
				if (!newWire.IsNull()) {
					newFace = BRepBuilderAPI_MakeFace(newWire);
					if (!newFace.IsNull())
						BuildTool.Add(aComp, newFace);
				}
			}
		}

		// If performSewing is true BRepBuilderAPI_Sewing creates a compound of
		// shells. Since the resulting shape isn't very usable in most use cases
		// it's fine to set it to false so the algorithm only performs some control
		// checks and creates a compound of faces.
		// However, the computing time can be reduced by 90%.
		// If a shell is needed then the sewShape() function should be called explicitly.
		BRepBuilderAPI_Sewing aSewingTool;
		Standard_Boolean performSewing = Standard_False;
		aSewingTool.Init(tolerance, performSewing);
		aSewingTool.Load(aComp);

#if OCC_VERSION_HEX < 0x070500
		Handle(Message_ProgressIndicator) pi = new ProgressIndicator(100);
		pi->NewScope(100, "Create shape from mesh...");
		pi->Show();

		aSewingTool.Perform(pi);
#else
		aSewingTool.Perform();
#endif

		_Shape = aSewingTool.SewedShape();
#if OCC_VERSION_HEX < 0x070500
		pi->EndScope();
#endif
		if (_Shape.IsNull())
			_Shape = aComp;
	}
	void TopoShape::getDomainfaces(std::vector<Domain>& domains, double accuracy) const
	{
		if (this->_Shape.IsNull())
			return;

		// get the meshes of all faces and then merge them
		BRepMesh_IncrementalMesh aMesh(this->_Shape, accuracy,
			/*isRelative*/ Standard_False,
			/*theAngDeflection*/
			defaultAngularDeflection(accuracy),
			/*isInParallel*/ true);
		
		getDomains(domains);
	}

	void TopoShape::getDomains(std::vector<Domain>& domains) const
	{
		std::size_t countFaces = 0;
		for (TopExp_Explorer xp(this->_Shape, TopAbs_FACE); xp.More(); xp.Next()) {
			++countFaces;
		}
		domains.reserve(countFaces);

		for (TopExp_Explorer xp(this->_Shape, TopAbs_FACE); xp.More(); xp.Next()) {
			TopoDS_Face face = TopoDS::Face(xp.Current());

			std::vector<gp_Pnt> points;
			std::vector<Poly_Triangle> facets;
			if (!Tools::getTriangulation(face, points, facets)) {
				// For a face that cannot be meshed append an empty domain.
				// It's important for some algorithms (e.g. color mapping) that the numbers of
				// faces and domains match
				Domain domain;
				domains.push_back(domain);
			}
			else {
				Domain domain;
				// copy the points
				domain.points.reserve(points.size());
				for (const auto& it : points) {
					Standard_Real X, Y, Z;
					it.Coord(X, Y, Z);
					domain.points.emplace_back(X, Y, Z);
				}

				// copy the triangles
				domain.facets.reserve(facets.size());
				for (const auto& it : facets) {
					Standard_Integer N1, N2, N3;
					it.Get(N1, N2, N3);

					Facet tria;
					tria.I1 = N1;
					tria.I2 = N2;
					tria.I3 = N3;
					domain.facets.push_back(tria);
				}

				domains.push_back(domain);
			}
		}
	}
	void TopoShape::setTransform(const Maths::FMatrix4& rclTrf)
	{
		gp_Trsf mov;
		convertTogpTrsf(rclTrf, mov);
		TopLoc_Location loc(mov);
		_Shape.Location(loc);
	}
	Maths::FMatrix4 TopoShape::getTransform() const
	{
		Maths::FMatrix4 mtrx;
		gp_Trsf Trf = _Shape.Location().Transformation();
		Trf.SetScaleFactor(1.0);
		convertToMatrix(Trf, mtrx);
		return mtrx;
	}
	Rendering::Geometry::bbox TopoShape::getBoundBox() const
	{
		Rendering::Geometry::bbox box;
		try {
			// If the shape is empty an exception may be thrown
			Bnd_Box bounds;
			BRepBndLib::Add(_Shape, bounds);
			bounds.SetGap(0.0);
			Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
			bounds.Get(xMin, yMin, zMin, xMax, yMax, zMax);

			
			box.pmin.x = xMin;
			box.pmax.x = xMax;
			box.pmin.y = yMin;
			box.pmax.y = yMax;
			box.pmin.z= zMin;
			box.pmax.z = zMax;
		}
		catch (Standard_Failure&) {
		}

		return box;
		//return Rendering::Geometry::bbox();
	}
	bool TopoShape::getCenterOfGravity(Vector3d& center) const
	{
		if (_Shape.IsNull())
			return false;

		// Computing of CentreOfMass
		GProp_GProps prop;
		if (getShapeProperties(_Shape, prop)) {
			if (prop.Mass() > Precision::Infinite()) {
				return false;
			}
			gp_Pnt pnt = prop.CentreOfMass();
			center = { pnt.X(), pnt.Y(), pnt.Z() };
			return true;
		}

		return false;
	}
	void TopoShape::convertTogpTrsf(const Maths::FMatrix4& mtrx, gp_Trsf& trsf)
	{
		
		trsf.SetValues(
			mtrx(0,0), mtrx(0,1), mtrx(0,2), mtrx(0,3),
			mtrx(1,0), mtrx(1,1), mtrx(1,2), mtrx(1,3),
			mtrx(2,0), mtrx(2,1), mtrx(2,2), mtrx(2,3));
	}
	void TopoShape::convertToMatrix(const gp_Trsf& trsf, Maths::FMatrix4& mtrx)
	{
		gp_Mat m = trsf.VectorialPart();
		gp_XYZ p = trsf.TranslationPart();
		// set Rotation matrix
		mtrx(0,0) = m(1, 1);
		mtrx(0,1) = m(1, 2);
		mtrx(0,2) = m(1, 3);

		mtrx(1,0) = m(2, 1);
		mtrx(1,1) = m(2, 2);
		mtrx(1,2) = m(2, 3);

		mtrx(2,0) = m(3, 1);
		mtrx(2,1) = m(3, 2);
		mtrx(2,2) = m(3, 3);

		// set pos vector
		mtrx(0,3) = p.X();
		mtrx(1,3) = p.Y();
		mtrx(2,3) = p.Z();

	}
	Maths::FMatrix4 TopoShape::convert(const gp_Trsf& trsf)
	{
		Maths::FMatrix4 mat;
		convertToMatrix(trsf, mat);
		return mat;
	}
	gp_Trsf TopoShape::convert(const Maths::FMatrix4& mtrx)
	{
		gp_Trsf trsf;
		convertTogpTrsf(mtrx, trsf);
		return trsf;
	}
	TopoDS_Shape& TopoShape::move(TopoDS_Shape& tds, const TopLoc_Location& location)
	{
#if OCC_VERSION_HEX < 0x070600
		tds.Move(location);
#else
		tds.Move(location, false);
#endif
		return tds;
	}
	TopoDS_Shape& TopoShape::locate(TopoDS_Shape& tds, const TopLoc_Location& loc)
	{
		tds.Location(TopLoc_Location());
		return move(tds, loc);
	}
	void TopoShape::getLinesFromSubShape(const TopoDS_Shape& shape, std::vector<Vector3d>& vertices, std::vector<Line>& lines) const
	{
		if (shape.IsNull())
			return;

		// build up map edge->face
		TopTools_IndexedDataMapOfShapeListOfShape edge2Face;
		TopExp::MapShapesAndAncestors(this->_Shape, TopAbs_EDGE, TopAbs_FACE, edge2Face);

		for (TopExp_Explorer exp(shape, TopAbs_EDGE); exp.More(); exp.Next()) {
			TopoDS_Edge aEdge = TopoDS::Edge(exp.Current());
			std::vector<gp_Pnt> points;

			if (!Tools::getPolygon3D(aEdge, points)) {
				// the edge has not its own triangulation, but then a face the edge is attached to
				// must provide this triangulation

				// Look for one face in our map (it doesn't care which one we take)
				int index = edge2Face.FindIndex(aEdge);
				if (index < 1)
					continue;

				const auto& faces = edge2Face.FindFromIndex(index);
				if (faces.IsEmpty())
					continue;

				const TopoDS_Face& aFace = TopoDS::Face(faces.First());
				if (!Tools::getPolygonOnTriangulation(aEdge, aFace, points))
					continue;
			}

			auto line_start = vertices.size();
			vertices.reserve(vertices.size() + points.size());
			std::for_each(points.begin(), points.end(), [&vertices](const gp_Pnt& p) {
				vertices.push_back({p.X(),p.Y(),p.Z()});
				});

			if (line_start + 1 < vertices.size()) {
				lines.emplace_back();
				lines.back().I1 = line_start;
				lines.back().I2 = vertices.size() - 1;
			}
		}
	}
	void TopoShape::getFacesFromDomains(const std::vector<Domain>& domains, std::vector<Vector3d>& vertices, std::vector<Facet>& faces) const
	{
		BRepMesh mesh;
		mesh.getFacesFromDomains(domains, vertices, faces);
	}
}