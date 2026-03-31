#include "core/log.h"
#include "base/Exception.h"
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
#include <STEPControl_Reader.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BOPAlgo_ArgumentAnalyzer.hxx>
#include <GeomLib_IsPlanarSurface.hxx>
#include <BRepLib_FindSurface.hxx>
#include <BRepLProp_SLProps.hxx>
#include <BRepGProp_Face.hxx>
#include <ShapeUpgrade_ShellSewing.hxx>
#include <BRepTools.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <ShapeFix_Root.hxx>
#include <ShapeBuild_Reshape.hxx>
#include <ShapeFix_Shape.hxx>
#include "FCBRepAlgoAPI_Cut.h"
#include "FCBRepAlgoAPI_Section.h"
#include "FCBRepAlgoAPI_Common.h"
#include "FCBRepAlgoAPI_Fuse.h"
#include "CrossSection.h"
#include "TopoShapeMapper.h"
#include "TopoShapeCache.h"
#include <map>
namespace MOON {
	static std::array<std::string, TopAbs_SHAPE> _ShapeNames;

	static void initShapeNameMap() {
		if (_ShapeNames[TopAbs_VERTEX].empty()) {
			_ShapeNames[TopAbs_VERTEX] = "Vertex";
			_ShapeNames[TopAbs_EDGE] = "Edge";
			_ShapeNames[TopAbs_FACE] = "Face";
			_ShapeNames[TopAbs_WIRE] = "Wire";
			_ShapeNames[TopAbs_SHELL] = "Shell";
			_ShapeNames[TopAbs_SOLID] = "Solid";
			_ShapeNames[TopAbs_COMPOUND] = "Compound";
			_ShapeNames[TopAbs_COMPSOLID] = "CompSolid";
		}
	}
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
	TopoShape::TopoShape(long Tag, App::StringHasherRef hasher, const TopoDS_Shape& shape) :_Shape(*this, shape)
	{
		this->Tag = Tag;
		Hasher = hasher;
	}
	TopoShape::TopoShape(const TopoDS_Shape&shape, long tag, App::StringHasherRef hasher) : _Shape(*this, shape)
	{
		this->Tag = tag;
		Hasher = hasher;
	}
	TopoShape::TopoShape(const TopoShape& shape):_Shape(*this)
	{
		*this = shape;
	}
	void TopoShape::operator=(const TopoShape&sh)
	{
		if (this != &sh) {
			this->setShape(sh._Shape, true);
			this->Tag = sh.Tag;
			this->Hasher = sh.Hasher;
			this->_cache = sh._cache;
			this->_parentCache = sh._parentCache;
			this->_subLocation = sh._subLocation;
			resetElementMap(sh.elementMap(false));
		}
	}
	TopoShape::~TopoShape()
	{
	}
	Data::ElementMapPtr TopoShape::elementMap(bool flush) const
	{
		if (flush) {
			//flushElementMap();
		}
		return _elementMap;
	}
	Data::ElementMapPtr TopoShape::resetElementMap(Data::ElementMapPtr elementMap)
	{
		if (_cache && elementMap != this->elementMap(false)) {
			for (auto& info : _cache->shapeAncestryCache) {
				info.clear();
			}
		}
		else {
			initCache();
		}
		if (elementMap) {
			_cache->cachedElementMap = elementMap;
			_cache->subLocation.Identity();
			_subLocation.Identity();
			_parentCache.reset();
		}
		_elementMap.swap(elementMap);
		// We expect that if the ComplexGeoData ( TopoShape ) has a hasher, then its elementMap will
		// have the same one.  Make sure that happens.
		if (_elementMap && !_elementMap->hasher)
			_elementMap->hasher = Hasher;
		return elementMap;
		//return Data::ComplexGeoData::resetElementMap(elementMap);
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
			std::vector<gp_Vec> normals;
			std::vector<Poly_Triangle> facets;
			if (!Tools::getTriangulation(face, points,normals, facets)) {
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
				domain.normals.reserve(points.size());
				for (const auto& it : points) {
					Standard_Real X, Y, Z;
					it.Coord(X, Y, Z);
					domain.points.emplace_back(X, Y, Z);
				}
				for (const auto& it : normals) {
					Standard_Real X, Y, Z;
					it.Coord(X, Y, Z);
					domain.normals.emplace_back(X, Y, Z);
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
	TopoDS_Shape TopoShape::makeShell(const TopoDS_Shape&input) const
	{    // For comparison see also:
	// GEOMImpl_BooleanDriver::makeCompoundShellFromFaces
		if (input.IsNull())
			return input;
		if (input.ShapeType() != TopAbs_COMPOUND)
			return input;

		// we need a compound that consists of only faces
		TopExp_Explorer it;
		// no shells
		it.Init(input, TopAbs_SHELL);
		if (it.More())
			return input;

		// no wires outside a face
		it.Init(input, TopAbs_WIRE, TopAbs_FACE);
		if (it.More())
			return input;

		// no edges outside a wire
		it.Init(input, TopAbs_EDGE, TopAbs_WIRE);
		if (it.More())
			return input;

		// no vertexes outside an edge
		it.Init(input, TopAbs_VERTEX, TopAbs_EDGE);
		if (it.More())
			return input;

		BRep_Builder builder;
		TopoDS_Shape shape;
		TopoDS_Shell shell;
		builder.MakeShell(shell);


		for (it.Init(input, TopAbs_FACE); it.More(); it.Next()) {
			if (!it.Current().IsNull())
				builder.Add(shell, it.Current());
		}

		shape = shell;
		BRepCheck_Analyzer check(shell);
		if (!check.IsValid()) {
			ShapeUpgrade_ShellSewing sewShell;
			shape = sewShell.ApplySewing(shell);
		}

		if (shape.IsNull())
			return input;

		if (shape.ShapeType() != TopAbs_SHELL)
			return input;

		return shape; // success
	
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
	TopoDS_Shape TopoShape::moved(const TopoDS_Shape& tds, const TopLoc_Location& loc)
	{
#if OCC_VERSION_HEX < 0x070600
		return tds.Moved(loc);
#else
		return tds.Moved(loc, false);
#endif
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
	TopoDS_Shape TopoShape::located(const TopoDS_Shape& tds, const TopLoc_Location& loc)
	{
		auto sCopy(tds);
		sCopy.Location(TopLoc_Location());
		return moved(sCopy, loc);
	}
	const std::string& TopoShape::shapeName(TopAbs_ShapeEnum type, bool silent)
	{
		initShapeNameMap();
		if (type >= 0 && type < _ShapeNames.size() && !_ShapeNames[type].empty())
			return _ShapeNames[type];
		if (!silent)
			FC_THROWM(Base::CADKernelError, "invalid shape type '" << type << "'");
		static std::string ret;
		return ret;
	}
	void TopoShape::importStep(const char* FileName)
	{

		STEPControl_Reader aReader;
		if (aReader.ReadFile(FileName) != IFSelect_RetDone) {
			CORE_ERROR("Error in reading STEP");
		}

		// Root transfers
		aReader.TransferRoots();
		// one shape that contains all subshapes
		this->_Shape = aReader.OneShape();

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
	std::vector<std::string> buildShapeEnumVector()
	{
		std::vector<std::string> names;
		names.emplace_back("Compound");             //TopAbs_COMPOUND
		names.emplace_back("Compound Solid");       //TopAbs_COMPSOLID
		names.emplace_back("Solid");                //TopAbs_SOLID
		names.emplace_back("Shell");                //TopAbs_SHELL
		names.emplace_back("Face");                 //TopAbs_FACE
		names.emplace_back("Wire");                 //TopAbs_WIRE
		names.emplace_back("Edge");                 //TopAbs_EDGE
		names.emplace_back("Vertex");               //TopAbs_VERTEX
		names.emplace_back("Shape");                //TopAbs_SHAPE
		return names;
	}

	std::vector<std::string> buildBOPCheckResultVector()
	{
		std::vector<std::string> results;
		results.emplace_back("BOPAlgo CheckUnknown");               //BOPAlgo_CheckUnknown
		results.emplace_back("BOPAlgo BadType");                    //BOPAlgo_BadType
		results.emplace_back("BOPAlgo SelfIntersect");              //BOPAlgo_SelfIntersect
		results.emplace_back("BOPAlgo TooSmallEdge");               //BOPAlgo_TooSmallEdge
		results.emplace_back("BOPAlgo NonRecoverableFace");         //BOPAlgo_NonRecoverableFace
		results.emplace_back("BOPAlgo IncompatibilityOfVertex");    //BOPAlgo_IncompatibilityOfVertex
		results.emplace_back("BOPAlgo IncompatibilityOfEdge");      //BOPAlgo_IncompatibilityOfEdge
		results.emplace_back("BOPAlgo IncompatibilityOfFace");      //BOPAlgo_IncompatibilityOfFace
		results.emplace_back("BOPAlgo OperationAborted");           //BOPAlgo_OperationAborted
		results.emplace_back("BOPAlgo GeomAbs_C0");                 //BOPAlgo_GeomAbs_C0
		results.emplace_back("BOPAlgo_InvalidCurveOnSurface");      //BOPAlgo_InvalidCurveOnSurface
		results.emplace_back("BOPAlgo NotValid");                   //BOPAlgo_NotValid
		return results;
	}
	namespace
	{
		void addShapesToBuilder(const std::vector<TopoShape>& shapes,
			BRep_Builder& builder,
			TopoDS_Compound& comp)
		{
			int count = 0;
			for (auto& topoShape : shapes) {
				if (topoShape.isNull()) {
					//FC_WARN("Null input shape");  // NOLINT
					continue;
				}
				builder.Add(comp, topoShape.getShape());
				++count;
			}
			if (count == 0) {
				//FC_THROWM(NullShapeException, "Null shape");
			}
		}
	}
	namespace
	{
		size_t checkSubshapeCount(const TopoShape& topoShape1,
			const TopoShape& topoShape2,
			TopAbs_ShapeEnum elementType)
		{
			auto count = topoShape1.countSubShapes(elementType);
			auto other = topoShape2.countSubShapes(elementType);
			if (count != other) {
				//FC_WARN("sub shape mismatch");  // NOLINT
				if (count > other) {
					count = other;
				}
			}
			return count;
		}

	}
	struct ShapeInfo
	{
		const TopoDS_Shape& shape;
		TopoShapeCache::Ancestry& cache;
		TopAbs_ShapeEnum type;
		const char* shapetype;

		ShapeInfo(const TopoDS_Shape& shape, TopAbs_ShapeEnum type, TopoShapeCache::Ancestry& cache)
			: shape(shape)
			, cache(cache)
			, type(type)
			, shapetype(TopoShape::shapeName(type).c_str())
		{
		}

		[[nodiscard]] int count() const
		{
			return cache.count();
		}

		TopoDS_Shape find(int index)
		{
			return cache.find(shape, index);
		}

		int find(const TopoDS_Shape& subshape)
		{
			return cache.find(shape, subshape);
		}
	};
	struct NameKey
	{
		Data::MappedName name;
		long tag = 0;
		int shapetype = 0;

		NameKey() = default;
		explicit NameKey(Data::MappedName n)
			: name(std::move(n))
		{
		}
		NameKey(int type, Data::MappedName n)
			: name(std::move(n))
		{
			// Order the shape type from vertex < edge < face < other.  We'll rely
			// on this for sorting when we name the geometry element.
			switch (type) {
			case TopAbs_VERTEX:
				shapetype = 0;
				break;
			case TopAbs_EDGE:
				shapetype = 1;
				break;
			case TopAbs_FACE:
				shapetype = 2;
				break;
			default:
				shapetype = 3;
			}
		}
		bool operator<(const NameKey& other) const
		{
			if (shapetype < other.shapetype) {
				return true;
			}
			if (shapetype > other.shapetype) {
				return false;
			}
			if (tag < other.tag) {
				return true;
			}
			if (tag > other.tag) {
				return false;
			}
			return name < other.name;
		}
	};

	struct NameInfo
	{
		int index{};
		Data::ElementIDRefs sids;
		const char* shapetype{};
	};

	const std::string& modPostfix()
	{
		static std::string postfix(Data::POSTFIX_MOD);
		return postfix;
	}

	const std::string& modgenPostfix()
	{
		static std::string postfix(Data::POSTFIX_MODGEN);
		return postfix;
	}

	const std::string& genPostfix()
	{
		static std::string postfix(Data::POSTFIX_GEN);
		return postfix;
	}

	const std::string& upperPostfix()
	{
		static std::string postfix(Data::POSTFIX_UPPER);
		return postfix;
	}

	const std::string& lowerPostfix()
	{
		static std::string postfix(Data::POSTFIX_LOWER);
		return postfix;
	}

	// TODO: Refactor checkForParallelOrCoplanar to reduce complexity
	void checkForParallelOrCoplanar(const TopoDS_Shape& newShape,
		const ShapeInfo& newInfo,
		std::vector<TopoDS_Shape>& newShapes,
		const gp_Pln& pln,
		int& parallelFace,
		int& coplanarFace,
		int& checkParallel)
	{
		for (TopExp_Explorer xp(newShape, newInfo.type); xp.More(); xp.Next()) {
			newShapes.push_back(xp.Current());

			if ((parallelFace < 0 || coplanarFace < 0) && checkParallel > 0) {
				// Specialized checking for high level mapped
				// face that are either coplanar or parallel
				// with the source face, which are common in
				// operations like extrusion. Once found, the
				// first coplanar face will assign an index of
				// INT_MIN+1, and the first parallel face
				// INT_MIN. The purpose of these special
				// indexing is to make the name more stable for
				// those generated faces.
				//
				// For example, the top or bottom face of an
				// extrusion will be named using the extruding
				// face. With a fixed index, the name is no
				// longer affected by adding/removing of holes
				// inside the extruding face/sketch.
				gp_Pln plnOther;
				if (TopoShape(newShapes.back()).findPlane(plnOther)) {
					if (pln.Axis().IsParallel(plnOther.Axis(), Precision::Angular())) {
						if (coplanarFace < 0) {
							gp_Vec vec(pln.Axis().Location(), plnOther.Axis().Location());
							Standard_Real D1 = gp_Vec(pln.Axis().Direction()).Dot(vec);
							if (D1 < 0) {
								D1 = -D1;
							}
							Standard_Real D2 = gp_Vec(plnOther.Axis().Direction()).Dot(vec);
							if (D2 < 0) {
								D2 = -D2;
							}
							if (D1 <= Precision::Confusion() && D2 <= Precision::Confusion()) {
								coplanarFace = (int)newShapes.size();
								continue;
							}
						}
						if (parallelFace < 0) {
							parallelFace = (int)newShapes.size();
						}
					}
				}
			}
		}
	}
	TopoShape& TopoShape::makeElementCopy(const TopoShape& shape, const char* op, bool copyGeom, bool copyMesh)
	{
		if (shape.isNull()) {
			return *this;
		}

		TopoShape tmp(shape);
		tmp.setShape(BRepBuilderAPI_Copy(shape.getShape(), copyGeom, copyMesh).Shape(), false);
		tmp.setTransform(shape.getTransform());
		if (op || (shape.Tag && shape.Tag != Tag)) {
			setShape(tmp._Shape);
			initCache();
			if (!Hasher) {
				Hasher = tmp.Hasher;
			}
			copyElementMap(tmp, op);
		}
		else {
			*this = tmp;
		}
		return *this;
	}
	bool TopoShape::fix()
	{
		if (this->_Shape.IsNull()) {
			return false;
		}

		// First, we do fix regardless if the current shape is valid or not,
		// because not all problems that are handled by ShapeFix_Shape can be
		// recognized by BRepCheck_Analyzer.
		//
		// Second, for some reason, a failed fix (i.e. a fix that produces invalid shape)
		// will affect the input shape. (See // https://github.com/realthunder/FreeCAD/issues/585,
		// BTW, the file attached in the issue also shows that ShapeFix_Shape may
		// actually make a valid input shape invalid). So, it actually change the
		// underlying shape data. Therefore, we try with a copy first.
		auto copy = makeElementCopy();
		ShapeFix_Shape fix(copy._Shape);
		fix.Perform();

		if (fix.Shape().IsSame(copy._Shape)) {
			return false;
		}

		BRepCheck_Analyzer aChecker(fix.Shape());
		if (!aChecker.IsValid()) {
			return false;
		}

		// If the above fix produces a valid shape, then we fix the original shape,
		// because BRepBuilderAPI_Copy has some undesired side effect (e.g. flatten
		// underlying shape, and thus break internal shape sharing).
		ShapeFix_Shape fixThis(this->_Shape);
		fixThis.Perform();

		aChecker.Init(fixThis.Shape());
		if (aChecker.IsValid()) {
			// Must call makESHAPE() (which calls mapSubElement()) to remap element
			// names because ShapeFix_Shape may delete (e.g. small edges) or modify
			// the input shape.
			//
			// See https://github.com/realthunder/FreeCAD/issues/595. Sketch001
			// has small edges. Simply recompute the sketch to trigger call of fix()
			// through makEWires(), and it will remove those edges. Without
			// remapping, there will be invalid index jumpping in reference in
			// Sketch002.ExternalEdge5.
			makeShapeWithElementMap(fixThis.Shape(), MapperHistory(fixThis), { *this });
		}
		else {
			makeShapeWithElementMap(fix.Shape(), MapperHistory(fix), { copy });
		}
		return true;
	}
	TopoShape& TopoShape::makeElementWires(const std::vector<TopoShape>& shapes, const char* op, double tol, ConnectionPolicy policy, TopoShapeMap* output)
	{
		if (!op) {
			op = "WIR";
		}
		if (tol < Precision::Confusion()) {
			tol = Precision::Confusion();
		}

		if (policy == ConnectionPolicy::requireSharedVertex) {
			// Can't use ShapeAnalysis_FreeBounds if not shared. It seems the output
			// edges are modified somehow, and it is not obvious how to map the
			// resulting edges.
			Handle(TopTools_HSequenceOfShape) hEdges = new TopTools_HSequenceOfShape();
			Handle(TopTools_HSequenceOfShape) hWires = new TopTools_HSequenceOfShape();
			for (const auto& shape : shapes) {
				for (const auto& edge : shape.getSubShapes(TopAbs_EDGE)) {
					hEdges->Append(edge);
				}
			}
			if (hEdges->Length() == 0) {
				//FC_THROWM(NullShapeException, "Null shape");
			}
			ShapeAnalysis_FreeBounds::ConnectEdgesToWires(hEdges, tol, Standard_True, hWires);
			if (hWires->Length() == 0) {
				//FC_THROWM(NullShapeException, "Null shape");
			}

			std::vector<TopoShape> wires;
			for (int i = 1; i <= hWires->Length(); i++) {
				auto wire = hWires->Value(i);
				wires.emplace_back(Tag, Hasher, wire);
				wires.back().mapSubElement(shapes, op);
			}
			return makeElementCompound(wires, "", SingleShapeCompoundCreationPolicy::returnShape);
		}

		std::vector<TopoShape> wires;
		std::list<TopoShape> edgeList;

		for (const auto& shape : shapes) {
			for (const auto& e : shape.getSubTopoShapes(TopAbs_EDGE)) {
				edgeList.emplace_back(e);
			}
		}

		std::vector<TopoShape> edges;
		edges.reserve(edgeList.size());
		wires.reserve(edgeList.size());

		// sort them together to wires
		while (!edgeList.empty()) {
			BRepBuilderAPI_MakeWire mkWire;
			// add and erase first edge
			edges.clear();
			edges.push_back(edgeList.front());
			mkWire.Add(TopoDS::Edge(edges.back().getShape()));
			edges.back().setShape(mkWire.Edge(), false);
			if (output) {
				(*output)[edges.back()] = edgeList.front();
			}
			edgeList.pop_front();

			TopoDS_Wire new_wire = mkWire.Wire();  // current new wire

			// try to connect each edge to the wire, the wire is complete if no more edges are
			// connectible
			bool found = true;
			while (found) {
				found = false;
				for (auto it = edgeList.begin(); it != edgeList.end(); ++it) {
					mkWire.Add(TopoDS::Edge(it->getShape()));
					if (mkWire.Error() != BRepBuilderAPI_DisconnectedWire) {
						// edge added ==> remove it from list
						found = true;
						edges.push_back(*it);
						// MakeWire will replace vertex of connected edge, which
						// effectively creat a new edge. So we need to update the
						// shape in order to preserve element mapping.
						edges.back().setShape(mkWire.Edge(), false);
						if (output) {
							(*output)[edges.back()] = *it;
						}
						edgeList.erase(it);
						new_wire = mkWire.Wire();
						break;
					}
				}
			}

			wires.emplace_back(new_wire);
			wires.back().mapSubElement(edges, op);
			wires.back().fix();
		}
		return makeElementCompound(wires, nullptr, SingleShapeCompoundCreationPolicy::returnShape);
	}
	TopoShape& TopoShape::makeElementWires(const TopoShape& shape, const char* op, double tol, ConnectionPolicy policy, TopoShapeMap* output)
	{
		return makeElementWires(std::vector<TopoShape>{shape}, op, tol, policy, output);
	}
	Data::MappedName TopoShape::getMappedName(const Data::IndexedName& element, bool allowUnmapped, Data::ElementIDRefs* sid) const
	{
		if (!element) {
			return {};
		}
		//flushElementMap();
		if (!_elementMap) {
			if (allowUnmapped) {
				return Data::MappedName(element);
			}
			return {};
		}

		Data::MappedName name = _elementMap->find(element, sid);
		if (allowUnmapped && !name) {
			return Data::MappedName(element);
		}
		return name;
	}

	TopoShape& TopoShape::makeShapeWithElementMap(const TopoDS_Shape& shape, const Mapper& mapper, const std::vector<TopoShape>& shapes, const char* op)
	{
		setShape(shape);
		if (shape.IsNull()) {
			//FC_THROWM(NullShapeException, "Null shape");
		}

		if (shapes.empty()) {
			return *this;
		}

		size_t canMap = 0;
		for (auto& incomingShape : shapes) {
			if (canMapElement(incomingShape)) {
				++canMap;
			}
		}
		if (canMap == 0U) {
			return *this;
		}
		//if (canMap != shapes.size() && FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG)) {
		//	FC_WARN("Not all input shapes are mappable");  // NOLINT
		//}

		if (!op) {
			op ="MAK";
		}
		std::string _op = op;
		_op += '_';

		initCache();
		ShapeInfo vertexInfo(_Shape, TopAbs_VERTEX, _cache->getAncestry(TopAbs_VERTEX));
		ShapeInfo edgeInfo(_Shape, TopAbs_EDGE, _cache->getAncestry(TopAbs_EDGE));
		ShapeInfo faceInfo(_Shape, TopAbs_FACE, _cache->getAncestry(TopAbs_FACE));
		mapSubElement(shapes);  // Intentionally leave the op off here

		std::array<ShapeInfo*, 3> infos = { &vertexInfo, &edgeInfo, &faceInfo };

		std::array<ShapeInfo*, TopAbs_SHAPE> infoMap{};
		infoMap[TopAbs_VERTEX] = &vertexInfo;
		infoMap[TopAbs_EDGE] = &edgeInfo;
		infoMap[TopAbs_WIRE] = &edgeInfo;
		infoMap[TopAbs_FACE] = &faceInfo;
		infoMap[TopAbs_SHELL] = &faceInfo;
		infoMap[TopAbs_SOLID] = &faceInfo;
		infoMap[TopAbs_COMPOUND] = &faceInfo;
		infoMap[TopAbs_COMPSOLID] = &faceInfo;

		std::ostringstream ss;
		std::string postfix;
		Data::MappedName newName;

		std::map<Data::IndexedName, std::map<NameKey, NameInfo>> newNames;

		// First, collect names from other shapes that generates or modifies the
		// new shape
		for (auto& pinfo : infos) {  // Walk Vertexes, then Edges, then Faces
			auto& info = *pinfo;
			for (const auto& incomingShape : shapes) {
				if (!canMapElement(incomingShape)) {
					continue;
				}
				auto& otherMap = incomingShape._cache->getAncestry(info.type);
				if (otherMap.count() == 0) {
					continue;
				}
				for (int i = 1; i <= otherMap.count(); i++) {
					const auto& otherElement = otherMap.find(incomingShape._Shape, i);
					// Find all new objects that are a modification of the old object
					Data::ElementIDRefs sids;
					NameKey key(
						info.type,
						incomingShape.getMappedName(Data::IndexedName::fromConst(info.shapetype, i),
							true,
							&sids));

					int newShapeCounter = 0;
					for (auto& newShape : mapper.modified(otherElement)) {
						++newShapeCounter;
						if (newShape.ShapeType() >= TopAbs_SHAPE) {
							// NOLINTNEXTLINE
							//FC_ERR("unknown modified shape type " << newShape.ShapeType() << " from "
							//	<< info.shapetype << i);
							continue;
						}
						auto& newInfo = *infoMap.at(newShape.ShapeType());
						if (newInfo.type != newShape.ShapeType()) {
							//if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG)) {
							//	// TODO: it seems modified shape may report higher
							//	// level shape type just like generated shape below.
							//	// Maybe we shall do the same for name construction.
							//	// NOLINTNEXTLINE
							//	FC_WARN("modified shape type " << shapeName(newShape.ShapeType())
							//		<< " mismatch with " << info.shapetype
							//		<< i);
							//}
							continue;
						}
						int newShapeIndex = newInfo.find(newShape);
						if (newShapeIndex == 0) {
							// This warning occurs in makeElementRevolve. It generates
							// some shape from a vertex that never made into the
							// final shape. There may be incomingShape cases there.
							//if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG)) {
							//	// NOLINTNEXTLINE
							//	FC_WARN("Cannot find " << op << " modified " << newInfo.shapetype
							//		<< " from " << info.shapetype << i);
							//}
							continue;
						}

						Data::IndexedName element =
							Data::IndexedName::fromConst(newInfo.shapetype, newShapeIndex);
						if (getMappedName(element)) {
							continue;
						}

						key.tag = incomingShape.Tag;
						auto& name_info = newNames[element][key];
						name_info.sids = sids;
						name_info.index = newShapeCounter;
						name_info.shapetype = info.shapetype;
					}

					int checkParallel = -1;
					gp_Pln pln;

					// Find all new objects that were generated from an old object
					// (e.g. a face generated from an edge)
					newShapeCounter = 0;
					for (auto& newShape : mapper.generated(otherElement)) {
						if (newShape.ShapeType() >= TopAbs_SHAPE) {
							//// NOLINTNEXTLINE
							//FC_ERR("unknown generated shape type " << newShape.ShapeType() << " from "
							//	<< info.shapetype << i);
							continue;
						}

						int parallelFace = -1;
						int coplanarFace = -1;
						auto& newInfo = *infoMap.at(newShape.ShapeType());
						std::vector<TopoDS_Shape> newShapes;
						int shapeOffset = 0;
						if (newInfo.type == newShape.ShapeType()) {
							newShapes.push_back(newShape);
						}
						else {
							// It is possible for the maker to report generating a
							// higher level shape, such as shell or solid. For
							// example, when extruding, OCC will report the
							// extruding face generating the entire solid. However,
							// it will also report the edges of the extruding face
							// generating the side faces. In this case, too much
							// information is bad for us. We don't want the name of
							// the side face (and its edges) to be coupled with
							// incomingShape (unrelated) edges in the extruding face.
							//
							// shapeOffset below is used to make sure the higher
							// level mapped names comes late after sorting. We'll
							// ignore those names if there are more precise mapping
							// available.
							shapeOffset = 3;

							if (info.type == TopAbs_FACE && checkParallel < 0) {
								if (!TopoShape(otherElement).findPlane(pln)) {
									checkParallel = 0;
								}
								else {
									checkParallel = 1;
								}
							}
							checkForParallelOrCoplanar(newShape,
								newInfo,
								newShapes,
								pln,
								parallelFace,
								coplanarFace,
								checkParallel);
						}
						key.shapetype += shapeOffset;
						for (auto& workingShape : newShapes) {
							++newShapeCounter;
							int workingShapeIndex = newInfo.find(workingShape);
							if (workingShapeIndex == 0) {
								//if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG)) {
								//	// NOLINTNEXTLINE
								//	FC_WARN("Cannot find " << op << " generated " << newInfo.shapetype
								//		<< " from " << info.shapetype << i);
								//}
								continue;
							}

							Data::IndexedName element =
								Data::IndexedName::fromConst(newInfo.shapetype, workingShapeIndex);
							auto mapped = getMappedName(element);
							if (mapped) {
								continue;
							}

							key.tag = incomingShape.Tag;
							auto& name_info = newNames[element][key];
							name_info.sids = sids;
							if (newShapeCounter == parallelFace) {
								name_info.index = std::numeric_limits<int>::min();
							}
							else if (newShapeCounter == coplanarFace) {
								name_info.index = std::numeric_limits<int>::min() + 1;
							}
							else {
								name_info.index = -newShapeCounter;
							}
							name_info.shapetype = info.shapetype;
						}
						key.shapetype -= shapeOffset;
					}
				}
			}
		}

		// We shall first exclude those names generated from high level mapping. If
		// there are still any unnamed elements left after we go through the process
		// below, we set delayed=true, and start using those excluded names.
		bool delayed = false;

		while (true) {

			// Construct the names for modification/generation info collected in
			// the previous step
			for (auto itName = newNames.begin(), itNext = itName; itNext != newNames.end();
				itName = itNext) {
				// We treat the first modified/generated source shape name specially.
				// If case there are more than one source shape. We hash the first
				// source name separately, and then obtain the second string id by
				// hashing all the source names together.  We then use the second
				// string id as the postfix for our name.
				//
				// In this way, we can associate the same source that are modified by
				// multiple other shapes.

				++itNext;

				auto& element = itName->first;
				auto& names = itName->second;
				const auto& first_key = names.begin()->first;
				auto& first_info = names.begin()->second;

				if (!delayed && first_key.shapetype >= 3 && first_info.index > INT_MIN + 1) {
					// This name is mapped from high level (shell, solid, etc.)
					// Delay till next round.
					//
					// index>INT_MAX+1 is for checking generated coplanar and
					// parallel face mapping, which has special fixed index to make
					// name stable.  These names are not delayed.
					continue;
				}
				if (!delayed && getMappedName(element)) {
					newNames.erase(itName);
					continue;
				}

				int name_type =
					first_info.index > 0 ? 1 : 2;  // index>0 means modified, or else generated
				Data::MappedName first_name = first_key.name;

				Data::ElementIDRefs sids(first_info.sids);

				postfix.clear();
				if (names.size() > 1) {
					ss.str("");
					ss << '(';
					bool first = true;
					auto it = names.begin();
					int count = 0;
					for (++it; it != names.end(); ++it) {
						auto& other_key = it->first;
						if (other_key.shapetype >= 3 && first_key.shapetype < 3) {
							// shapetype>=3 means it's a high level mapping (e.g. a face
							// generates a solid). We don't want that if there are more
							// precise low level mapping available. See comments above
							// for more details.
							break;
						}
						if (first) {
							first = false;
						}
						else {
							ss << '|';
						}
						auto& other_info = it->second;
						std::ostringstream ss2;
						if (other_info.index != 1) {
							// 'K' marks the additional source shape of this
							// generate (or modified) shape.
							ss2 << ";" << 'K';
							if (other_info.index == INT_MIN) {
								ss2 << '0';
							}
							else if (other_info.index == INT_MIN + 1) {
								ss2 << "00";
							}
							else {
								// The same source shape may generate or modify
								// more than one shape. The index here marks the
								// position it is reported by OCC. Including the
								// index here is likely to degrade name stablilty,
								// but is unfortunately a necessity to avoid
								// duplicate names.
								ss2 << other_info.index;
							}
						}
						Data::MappedName other_name = other_key.name;

						ensureElementMap()->encodeElementName(*other_info.shapetype,
							other_name,
							ss2,
							&sids,
							Tag,
							nullptr,
							other_key.tag);
						ss << other_name;
						if ((name_type == 1 && other_info.index < 0)
							|| (name_type == 2 && other_info.index > 0)) {
							//if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG)) {
							//	FC_WARN("element is both generated and modified");  // NOLINT
							//}
							name_type = 0;
						}
						sids += other_info.sids;
						// To avoid the name becoming to long, just put some limit here
						if (++count == 4) {
							break;
						}
					}
					if (!first) {
						ss << ')';
						if (Hasher) {
							sids.push_back(Hasher->getID(ss.str().c_str()));
							ss.str("");
							ss << sids.back().toString();
						}
						postfix = ss.str();
					}
				}

				ss.str("");
				if (name_type == 2) {
					ss << genPostfix();
				}
				else if (name_type == 1) {
					ss << modPostfix();
				}
				else {
					ss << modgenPostfix();
				}
				if (first_info.index == INT_MIN) {
					ss << '0';
				}
				else if (first_info.index == INT_MIN + 1) {
					ss << "00";
				}
				else if (abs(first_info.index) > 1) {
					ss << abs(first_info.index);
				}
				ss << postfix;

				ensureElementMap()
					->encodeElementName(element[0], first_name, ss, &sids, Tag, op, first_key.tag);
				elementMap()->setElementName(element, first_name, Tag, &sids);
				if (!delayed && first_key.shapetype < 3) {
					newNames.erase(itName);
				}
			}

			// The reverse pass. Starting from the highest level element, i.e.
			// Face, for any element that are named, assign names for its lower unnamed
			// elements. For example, if Edge1 is named E1, and its vertexes are not
			// named, then name them as E1;U1, E1;U2, etc.
			//
			// In order to make the name as stable as possible, we may assign multiple
			// names (which must be sorted, because we may use the first one to name
			// upper element in the final pass) to lower element if it appears in
			// multiple higher elements, e.g. same edge in multiple faces.

			for (size_t infoIndex = infos.size() - 1; infoIndex != 0; --infoIndex) {
				std::map<Data::IndexedName,
					std::map<Data::MappedName, NameInfo, Data::ElementNameComparator>>
					names;
				auto& info = *infos.at(infoIndex);
				auto& next = *infos.at(infoIndex - 1);
				int elementCounter = 1;
				auto it = newNames.end();
				if (delayed) {
					it = newNames.upper_bound(Data::IndexedName::fromConst(info.shapetype, 0));
				}
				for (;; ++elementCounter) {
					Data::IndexedName element;
					if (!delayed) {
						if (elementCounter > info.count()) {
							break;
						}
						element = Data::IndexedName::fromConst(info.shapetype, elementCounter);
						if (newNames.count(element) != 0U) {
							continue;
						}
					}
					else if (it == newNames.end()
						|| !boost::starts_with(it->first.getType(), info.shapetype)) {
						break;
					}
					else {
						element = it->first;
						++it;
						elementCounter = element.getIndex();
						if (elementCounter == 0 || elementCounter > info.count()) {
							continue;
						}
					}
					Data::ElementIDRefs sids;
					Data::MappedName mapped = getMappedName(element, false, &sids);
					if (!mapped) {
						continue;
					}

					TopTools_IndexedMapOfShape submap;
					TopExp::MapShapes(info.find(elementCounter), next.type, submap);
					for (int submapIndex = 1, infoCounter = 1; submapIndex <= submap.Extent();
						++submapIndex) {
						ss.str("");
						int elementIndex = next.find(submap(submapIndex));
						assert(elementIndex);
						Data::IndexedName indexedName =
							Data::IndexedName::fromConst(next.shapetype, elementIndex);
						if (getMappedName(indexedName)) {
							continue;
						}
						auto& infoRef = names[indexedName][mapped];
						infoRef.index = infoCounter++;
						infoRef.sids = sids;
					}
				}
				// Assign the actual names
				for (auto& [indexedName, nameInfoMap] : names) {
					// Do we really want multiple names for an element in this case?
					// If not, we just pick the name in the first sorting order here.
					auto& nameInfoMapEntry = *nameInfoMap.begin();
					{
						auto& nameInfo = nameInfoMapEntry.second;
						auto& sids = nameInfo.sids;
						newName = nameInfoMapEntry.first;
						ss.str("");
						ss << upperPostfix();
						if (nameInfo.index > 1) {
							ss << nameInfo.index;
						}

						ensureElementMap()->encodeElementName(indexedName[0], newName, ss, &sids, Tag, op);
						elementMap()->setElementName(indexedName, newName, Tag, &sids);
					}
				}
			}

			// The forward pass. For any elements that are not named, try construct its
			// name from the lower elements
			bool hasUnnamed = false;
			for (size_t ifo = 1; ifo < infos.size(); ++ifo) {
				auto& info = *infos.at(ifo);
				auto& prev = *infos.at(ifo - 1);
				for (int i = 1; i <= info.count(); ++i) {
					Data::IndexedName element = Data::IndexedName::fromConst(info.shapetype, i);
					if (getMappedName(element)) {
						continue;
					}

					Data::ElementIDRefs sids;
					std::map<Data::MappedName, Data::IndexedName, Data::ElementNameComparator> names;
					TopExp_Explorer xp;
					if (info.type == TopAbs_FACE) {
						xp.Init(BRepTools::OuterWire(TopoDS::Face(info.find(i))), TopAbs_EDGE);
					}
					else {
						xp.Init(info.find(i), prev.type);
					}
					for (; xp.More(); xp.Next()) {
						int previousElementIndex = prev.find(xp.Current());
						assert(previousElementIndex);
						Data::IndexedName prevElement =
							Data::IndexedName::fromConst(prev.shapetype, previousElementIndex);
						if (!delayed && (newNames.count(prevElement) != 0U)) {
							names.clear();
							break;
						}
						Data::ElementIDRefs sid;
						Data::MappedName name = getMappedName(prevElement, false, &sid);
						if (!name) {
							// only assign name if all lower elements are named
							//if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG)) {
							//	FC_WARN("unnamed lower element " << prevElement);  // NOLINT
							//}
							names.clear();
							break;
						}
						auto res = names.emplace(name, prevElement);
						if (res.second) {
							sids += sid;
						}
						else if (prevElement != res.first->second) {
							// The seam edge will appear twice, which is normal. We
							// only warn if the mapped element names are different.
							// NOLINTNEXTLINE
							//FC_WARN("lower element " << prevElement << " and " << res.first->second
							//	<< " has duplicated name " << name << " for "
							//	<< info.shapetype << i);
						}
					}
					if (names.empty()) {
						hasUnnamed = true;
						continue;
					}
					auto it = names.begin();
					newName = it->first;
					if (names.size() == 1) {
						ss << lowerPostfix();
					}
					else {
						bool first = true;
						ss.str("");
						if (!Hasher) {
							ss << lowerPostfix();
						}
						ss << '(';
						int count = 0;
						for (++it; it != names.end(); ++it) {
							if (first) {
								first = false;
							}
							else {
								ss << '|';
							}
							ss << it->first;

							// To avoid the name becoming to long, just put some limit here
							if (++count == 4) {
								break;
							}
						}
						ss << ')';
						if (Hasher) {
							sids.push_back(Hasher->getID(ss.str().c_str()));
							ss.str("");
							ss << lowerPostfix() << sids.back().toString();
						}
					}

					ensureElementMap()->encodeElementName(element[0], newName, ss, &sids, Tag, op);
					elementMap()->setElementName(element, newName, Tag, &sids);
				}
			}
			if (!hasUnnamed || delayed || newNames.empty()) {
				break;
			}
			delayed = true;
		}
		return *this;
	}
	TopoShape& TopoShape::makeElementShape(BRepBuilderAPI_MakeShape& mkShape, const std::vector<TopoShape>& shapes, const char* op)
	{
		TopoDS_Shape shape;
		// OCCT 7.3.x requires calling Solid() and not Shape() to function correctly
		if (typeid(mkShape) == typeid(BRepPrimAPI_MakeHalfSpace)) {
			shape = static_cast<BRepPrimAPI_MakeHalfSpace&>(mkShape).Solid();
		}
		else {
			shape = mkShape.Shape();
		}
		return makeShapeWithElementMap(shape, MapperMaker(mkShape), shapes, op);
	}
	TopoShape& TopoShape::makeElementShape(BRepBuilderAPI_MakeShape& mkShape, const TopoShape& source, const char* op)
	{
		std::vector<TopoShape> sources(1, source);
		return makeElementShape(mkShape, sources, op);
	}
	TopAbs_ShapeEnum TopoShape::shapeType(bool silent) const
	{
		if (isNull()) {
			//if (!silent)
				//FC_THROWM(NullShapeException, "Input shape is null");
			return TopAbs_SHAPE;
		}
		return getShape().ShapeType();
	}
	Data::ElementMapPtr TopoShape::ensureElementMap(bool flush)
	{
		if (!_elementMap) {
			resetElementMap(std::make_shared<Data::ElementMap>());
		}
		return elementMap(flush);
	}
	std::vector<std::pair<Data::MappedName, Data::ElementIDRefs>> TopoShape::getElementMappedNames(const Data::IndexedName& element, bool needUnmapped) const
	{
		//flushElementMap();
		if (_elementMap) {
			auto res = _elementMap->findAll(element);
			if (!res.empty()) {
				return res;
			}
		}

		if (!needUnmapped) {
			return {};
		}
		return { std::make_pair(Data::MappedName(element), Data::ElementIDRefs()) };
	}
	void TopoShape::setMappedChildElements(const std::vector<Data::ElementMap::MappedChildElements>& children)
	{
		// DO NOT reset element map if there is one. Because we allow mixing child
// mapping and normal mapping
		if (!_elementMap) {
			resetElementMap(std::make_shared<Data::ElementMap>());
		}
		_elementMap->addChildElements(Tag, children);
	}
	void TopoShape::setupChild(Data::ElementMap::MappedChildElements& child, TopAbs_ShapeEnum elementType, const TopoShape& topoShape, size_t shapeCount, const char* op)
	{
		child.indexedName = Data::IndexedName::fromConst(TopoShape::shapeName(elementType).c_str(), 1);
		child.offset = 0;
		child.count = static_cast<int>(shapeCount);
		child.elementMap = topoShape.elementMap();
		if (this->Tag != topoShape.Tag) {
			child.tag = topoShape.Tag;
		}
		else {
			child.tag = 0;
		}
		if (op) {
			child.postfix = op;
		}
	}
	// namespace
	void TopoShape::copyElementMap(const TopoShape& topoShape, const char* op)
	{
		if (topoShape.isNull() || isNull()) {
			return;
		}
		std::vector<Data::ElementMap::MappedChildElements> children;
		std::array<TopAbs_ShapeEnum, 3> elementTypes = { TopAbs_VERTEX, TopAbs_EDGE, TopAbs_FACE };
		for (const auto elementType : elementTypes) {
			auto count = checkSubshapeCount(*this, topoShape, elementType);
			if (count == 0) {
				continue;
			}
			children.emplace_back();
			auto& child = children.back();
			setupChild(child, elementType, topoShape, count, op);
		}
		resetElementMap();
		if (!Hasher) {
			Hasher = topoShape.Hasher;
		}
		setMappedChildElements(children);
	}
	size_t TopoShape::getElementMapSize(bool flush) const
	{
		return _elementMap ? _elementMap->size() : 0;
	}
	bool TopoShape::hasPendingElementMap() const
	{
		return !elementMap(false) && this->_cache
			&& (this->_parentCache || this->_cache->cachedElementMap);
	}
	bool TopoShape::canMapElement(const TopoShape& other) const
	{
		if (isNull() || other.isNull() || this == &other || other.Tag == -1 || Tag == -1) {
			return false;
		}
		if ((other.Tag == 0) && !other.elementMap(false) && !other.hasPendingElementMap()) {
			return false;
		}
		initCache();
		other.initCache();
		_cache->relations.clear();
		return true;
	}
	void TopoShape::mapSubElement(const TopoShape& other, const char* op, bool forceHasher)
	{
		if (!canMapElement(other)) {
			return;
		}

		if (!getElementMapSize(false) && this->_Shape.IsPartner(other._Shape)) {
			if (!this->Hasher) {
				this->Hasher = other.Hasher;
			}
			copyElementMap(other, op);
			return;
		}

		bool warned = false;
		static const std::array<TopAbs_ShapeEnum, 3> types = { TopAbs_VERTEX, TopAbs_EDGE, TopAbs_FACE };

		auto checkHasher = [this](const TopoShape& other) {
			if (Hasher) {
				if (other.Hasher != Hasher) {
					if (!getElementMapSize(false)) {
						//if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG)) {
						//	FC_WARN("hasher mismatch");
						//}
					}
					else {
						// FC_THROWM(Base::RuntimeError, "hasher mismatch");
						//FC_ERR("hasher mismatch");
					}
					Hasher = other.Hasher;
				}
			}
			else {
				Hasher = other.Hasher;
			}
			};

		for (auto type : types) {
			auto& shapeMap = _cache->getAncestry(type);
			auto& otherMap = other._cache->getAncestry(type);
			if (!shapeMap.count() || !otherMap.count()) {
				continue;
			}
			if (!forceHasher && other.Hasher) {
				forceHasher = true;
				checkHasher(other);
			}
			const char* shapetype = shapeName(type).c_str();
			std::ostringstream ss;

			bool forward;
			int count;
			if (otherMap.count() <= shapeMap.count()) {
				forward = true;
				count = otherMap.count();
			}
			else {
				forward = false;
				count = shapeMap.count();
			}
			for (int k = 1; k <= count; ++k) {
				int i, idx;
				if (forward) {
					i = k;
					idx = shapeMap.find(_Shape, otherMap.find(other._Shape, k));
					if (!idx) {
						continue;
					}
				}
				else {
					idx = k;
					i = otherMap.find(other._Shape, shapeMap.find(_Shape, k));
					if (!i) {
						continue;
					}
				}
				Data::IndexedName element = Data::IndexedName::fromConst(shapetype, idx);
				for (auto& v :
					other.getElementMappedNames(Data::IndexedName::fromConst(shapetype, i), true)) {
					auto& name = v.first;
					auto& sids = v.second;
					if (sids.size()) {
						if (!Hasher) {
							Hasher = sids[0].getHasher();
						}
						else if (!sids[0].isFromSameHasher(Hasher)) {
							if (!warned) {
								warned = true;
								//FC_WARN("hasher mismatch");
							}
							sids.clear();
						}
					}
					ss.str("");

					ensureElementMap()->encodeElementName(shapetype[0], name, ss, &sids, Tag, op, other.Tag);
					elementMap()->setElementName(element, name, Tag, &sids);
				}
			}
		}
	}
	void TopoShape::mapSubElement(const std::vector<TopoShape>& shapes, const char* op)
	{
		if (shapes.empty()) {
			return;
		}

		if (shapeType(true) == TopAbs_COMPOUND) {
			int count = 0;
			for (auto& s : shapes) {
				if (s.isNull()) {
					continue;
				}
				if (!getSubShape(TopAbs_SHAPE, ++count, true).IsPartner(s._Shape)) {
					count = 0;
					break;
				}
			}
			if (count) {
				std::vector<Data::ElementMap::MappedChildElements> children;
				children.reserve(count * 3);
				TopAbs_ShapeEnum types[] = { TopAbs_VERTEX, TopAbs_EDGE, TopAbs_FACE };
				for (unsigned i = 0; i < sizeof(types) / sizeof(types[0]); ++i) {
					int offset = 0;
					for (auto& s : shapes) {
						if (s.isNull()) {
							continue;
						}
						int count = s.countSubShapes(types[i]);
						if (!count) {
							continue;
						}
						children.emplace_back();
						auto& child = children.back();
						child.indexedName =
							Data::IndexedName::fromConst(shapeName(types[i]).c_str(), 1);
						child.offset = offset;
						offset += count;
						child.count = count;
						child.elementMap = s.elementMap();
						child.tag = s.Tag;
						if (op) {
							child.postfix = op;
						}
					}
				}
				setMappedChildElements(children);
				return;
			}
		}

		for (auto& shape : shapes) {
			mapSubElement(shape, op);
		}
	}
	// namespace
	TopoShape& TopoShape::makeElementCompound(const std::vector<TopoShape>& shapes, const char* op, SingleShapeCompoundCreationPolicy policy)
	{
		if (policy == SingleShapeCompoundCreationPolicy::returnShape && shapes.size() == 1) {
			*this = shapes[0];
			return *this;
		}

		BRep_Builder builder;
		TopoDS_Compound comp;
		builder.MakeCompound(comp);

		if (shapes.empty()) {
			setShape(comp);
			return *this;
		}
		addShapesToBuilder(shapes, builder, comp);
		setShape(comp);
		initCache();

		mapSubElement(shapes, op);
		return *this;
	}
	void TopoShape::initCache(int reset) const
	{
		if (reset > 0 || !_cache || _cache->isTouched(_Shape)) {
			if (_parentCache) {
				_parentCache.reset();
				_subLocation.Identity();
			}
			_cache = std::make_shared<TopoShapeCache>(_Shape);
		}
	}
	int TopoShape::findShape(const TopoDS_Shape& subshape) const
	{
		initCache();
		return _cache->findShape(_Shape, subshape);
	}
	TopoDS_Shape TopoShape::findAncestorShape(const TopoDS_Shape& subshape, TopAbs_ShapeEnum type) const
	{
		initCache();
		return _cache->findAncestor(_Shape, subshape, type);
	}
	std::vector<TopoDS_Shape> TopoShape::findAncestorsShapes(const TopoDS_Shape& subshape, TopAbs_ShapeEnum type) const
	{
		initCache();
		std::vector<TopoDS_Shape> shapes;
		_cache->findAncestor(_Shape, subshape, type, &shapes);
		return shapes;
	}
	void TopoShape::setShape(const TopoDS_Shape& shape, bool resetElementMap)
	{
		if (resetElementMap) {
			this->resetElementMap();
		}
		else if (_cache && _cache->isTouched(shape)) {
			//this->flushElementMap();
		}
		//_Shape._Shape = shape; // TODO: Replace the next line with this once ShapeProtector is
		// available.
		_Shape = shape;
		if (_cache) {
			initCache();
		}
	}
	TopoShape TopoShape::getSubTopoShape(TopAbs_ShapeEnum type, int idx, bool silent) const
	{
		if (isNull()) {
			if (!silent) {
				//FC_THROWM(NullShapeException, "null shape");
			}
			return TopoShape();
		}
		if (idx <= 0) {
			if (!silent) {
				//FC_THROWM(Base::ValueError, "Invalid shape index " << idx);
			}
			return TopoShape();
		}
		if (type < 0 || type > TopAbs_SHAPE) {
			if (!silent) {
				//FC_THROWM(Base::ValueError, "Invalid shape type " << type);
			}
			return TopoShape();
		}
		//initCache();
		//auto& shapeMap = _cache->getAncestry(type);
		//if (idx > shapeMap.count()) {
		//	if (!silent) {
		//		FC_THROWM(Base::IndexError,
		//			"Shape index " << idx << " out of bound " << shapeMap.count());
		//	}
		//	return TopoShape();
		//}

		//return shapeMap.getTopoShape(*this, idx);
		return TopoShape();
	}
	TopoDS_Shape TopoShape::getSubShape(TopAbs_ShapeEnum type, int idx, bool silent) const
	{
		TopoShape s(*this);
		s.Tag = 0;
		return s.getSubTopoShape(type, idx, silent).getShape();
	}
	bool TopoShape::hasSubShape(TopAbs_ShapeEnum type) const
	{
		if (type == TopAbs_SHAPE) {
			TopoDS_Iterator it(_Shape);
			return !!it.More();
		}
		TopExp_Explorer exp(_Shape, type);
		return !!exp.More();
	}
	std::vector<TopoDS_Shape> TopoShape::getSubShapes(TopAbs_ShapeEnum type, TopAbs_ShapeEnum avoid) const
	{
		std::vector<TopoDS_Shape> ret;
		if (isNull()) {
			return ret;
		}
		if (avoid != TopAbs_SHAPE) {
			for (TopExp_Explorer exp(getShape(), type, avoid); exp.More(); exp.Next()) {
				ret.push_back(exp.Current());
			}
			return ret;
		}
		initCache();
        auto& ancestry = _cache->getAncestry(type);
		int count = ancestry.count();
		ret.reserve(count);
		for (int i = 1; i <= count; ++i) {
			ret.push_back(ancestry.find(_Shape, i));
		}
		return ret;
	}
	std::vector<TopoShape> TopoShape::getSubTopoShapes(TopAbs_ShapeEnum type, TopAbs_ShapeEnum avoid) const
	{
		if (isNull()) {
			return std::vector<TopoShape>();
		}
		initCache();

		auto res = _cache->getAncestry(type).getTopoShapes(*this);
		if (avoid != TopAbs_SHAPE && hasSubShape(avoid)) {
			for (auto it = res.begin(); it != res.end();) {
				if (_cache->findAncestor(_Shape, it->getShape(), avoid).IsNull()) {
					++it;
				}
				else {
					it = res.erase(it);
				}
			}
		}
		return res;
	}
	unsigned long TopoShape::countSubShapes(TopAbs_ShapeEnum Type) const
	{
		if (Type == TopAbs_SHAPE) {
			int count = 0;
			for (TopoDS_Iterator it(_Shape);it.More();it.Next())
				++count;
			return count;
		}
		TopTools_IndexedMapOfShape anIndices;
		TopExp::MapShapes(this->_Shape, Type, anIndices);
		return anIndices.Extent();
	}
	bool TopoShape::isNull() const
	{
		return this->_Shape.IsNull() ? true : false;
	}
	bool TopoShape::isValid() const
	{
		BRepCheck_Analyzer aChecker(this->_Shape);
		return aChecker.IsValid() ? true : false;
	}
	bool TopoShape::analyze(bool runBopCheck, std::ostream&str) const
	{
		if (!this->_Shape.IsNull()) {
			BRepCheck_Analyzer aChecker(this->_Shape);
			if (!aChecker.IsValid()) {
				std::vector<TopoDS_Shape> shapes;

				TopTools_IndexedMapOfShape vertexOfShape;
				TopExp::MapShapes(this->_Shape, TopAbs_VERTEX, vertexOfShape);
				for (int i = 1; i <= vertexOfShape.Extent();++i)
					shapes.push_back(vertexOfShape(i));

				TopTools_IndexedMapOfShape edgeOfShape;
				TopExp::MapShapes(this->_Shape, TopAbs_EDGE, edgeOfShape);
				for (int i = 1; i <= edgeOfShape.Extent();++i)
					shapes.push_back(edgeOfShape(i));

				TopTools_IndexedMapOfShape wireOfShape;
				TopExp::MapShapes(this->_Shape, TopAbs_WIRE, wireOfShape);
				for (int i = 1; i <= wireOfShape.Extent();++i)
					shapes.push_back(wireOfShape(i));

				TopTools_IndexedMapOfShape faceOfShape;
				TopExp::MapShapes(this->_Shape, TopAbs_FACE, faceOfShape);
				for (int i = 1; i <= faceOfShape.Extent();++i)
					shapes.push_back(faceOfShape(i));

				TopTools_IndexedMapOfShape shellOfShape;
				TopExp::MapShapes(this->_Shape, TopAbs_SHELL, shellOfShape);
				for (int i = 1; i <= shellOfShape.Extent();++i)
					shapes.push_back(shellOfShape(i));

				TopTools_IndexedMapOfShape solidOfShape;
				TopExp::MapShapes(this->_Shape, TopAbs_SOLID, solidOfShape);
				for (int i = 1; i <= solidOfShape.Extent();++i)
					shapes.push_back(solidOfShape(i));

				TopTools_IndexedMapOfShape compOfShape;
				TopExp::MapShapes(this->_Shape, TopAbs_COMPOUND, compOfShape);
				for (int i = 1; i <= compOfShape.Extent();++i)
					shapes.push_back(compOfShape(i));

				TopTools_IndexedMapOfShape compsOfShape;
				TopExp::MapShapes(this->_Shape, TopAbs_COMPSOLID, compsOfShape);
				for (int i = 1; i <= compsOfShape.Extent();++i)
					shapes.push_back(compsOfShape(i));

				for (const auto& shape : shapes) {
					if (!aChecker.IsValid(shape)) {
						const Handle(BRepCheck_Result)& result = aChecker.Result(shape);
						if (result.IsNull())
							continue;
						const BRepCheck_ListOfStatus& status = result->StatusOnShape(shape);

						BRepCheck_ListIteratorOfListOfStatus it(status);
						while (it.More()) {
							BRepCheck_Status& val = it.Value();
							switch (val)
							{
							case BRepCheck_NoError:
								str << "No error" << std::endl;
								break;
							case BRepCheck_InvalidPointOnCurve:
								str << "Invalid point on curve" << std::endl;
								break;
							case BRepCheck_InvalidPointOnCurveOnSurface:
								str << "Invalid point on curve on surface" << std::endl;
								break;
							case BRepCheck_InvalidPointOnSurface:
								str << "Invalid point on surface" << std::endl;
								break;
							case BRepCheck_No3DCurve:
								str << "No 3D curve" << std::endl;
								break;
							case BRepCheck_Multiple3DCurve:
								str << "Multiple 3D curve" << std::endl;
								break;
							case BRepCheck_Invalid3DCurve:
								str << "Invalid 3D curve" << std::endl;
								break;
							case BRepCheck_NoCurveOnSurface:
								str << "No curve on surface" << std::endl;
								break;
							case BRepCheck_InvalidCurveOnSurface:
								str << "Invalid curve on surface" << std::endl;
								break;
							case BRepCheck_InvalidCurveOnClosedSurface:
								str << "Invalid curve on closed surface" << std::endl;
								break;
							case BRepCheck_InvalidSameRangeFlag:
								str << "Invalid same-range flag" << std::endl;
								break;
							case BRepCheck_InvalidSameParameterFlag:
								str << "Invalid same-parameter flag" << std::endl;
								break;
							case BRepCheck_InvalidDegeneratedFlag:
								str << "Invalid degenerated flag" << std::endl;
								break;
							case BRepCheck_FreeEdge:
								str << "Free edge" << std::endl;
								break;
							case BRepCheck_InvalidMultiConnexity:
								str << "Invalid multi-connexity" << std::endl;
								break;
							case BRepCheck_InvalidRange:
								str << "Invalid range" << std::endl;
								break;
							case BRepCheck_EmptyWire:
								str << "Empty wire" << std::endl;
								break;
							case BRepCheck_RedundantEdge:
								str << "Redundant edge" << std::endl;
								break;
							case BRepCheck_SelfIntersectingWire:
								str << "Self-intersecting wire" << std::endl;
								break;
							case BRepCheck_NoSurface:
								str << "No surface" << std::endl;
								break;
							case BRepCheck_InvalidWire:
								str << "Invalid wires" << std::endl;
								break;
							case BRepCheck_RedundantWire:
								str << "Redundant wires" << std::endl;
								break;
							case BRepCheck_IntersectingWires:
								str << "Intersecting wires" << std::endl;
								break;
							case BRepCheck_InvalidImbricationOfWires:
								str << "Invalid imbrication of wires" << std::endl;
								break;
							case BRepCheck_EmptyShell:
								str << "Empty shell" << std::endl;
								break;
							case BRepCheck_RedundantFace:
								str << "Redundant face" << std::endl;
								break;
							case BRepCheck_UnorientableShape:
								str << "Unorientable shape" << std::endl;
								break;
							case BRepCheck_NotClosed:
								str << "Not closed" << std::endl;
								break;
							case BRepCheck_NotConnected:
								str << "Not connected" << std::endl;
								break;
							case BRepCheck_SubshapeNotInShape:
								str << "Sub-shape not in shape" << std::endl;
								break;
							case BRepCheck_BadOrientation:
								str << "Bad orientation" << std::endl;
								break;
							case BRepCheck_BadOrientationOfSubshape:
								str << "Bad orientation of sub-shape" << std::endl;
								break;
							case BRepCheck_InvalidToleranceValue:
								str << "Invalid tolerance value" << std::endl;
								break;
							case BRepCheck_CheckFail:
								str << "Check failed" << std::endl;
								break;
							default:
								str << "Undetermined error" << std::endl;
								break;
							}

							it.Next();
						}
					}
				}

				return false; // errors detected
			}
			else if (runBopCheck) {

				TopoDS_Shape BOPCopy = BRepBuilderAPI_Copy(this->_Shape).Shape();
				BOPAlgo_ArgumentAnalyzer BOPCheck;
				BOPCheck.SetShape1(BOPCopy);
				//all settings are false by default. so only turn on what we want.
				BOPCheck.ArgumentTypeMode() = true;
				BOPCheck.SelfInterMode() = true;
				BOPCheck.SmallEdgeMode() = true;
				BOPCheck.RebuildFaceMode() = true;
				BOPCheck.ContinuityMode() = true;
				BOPCheck.SetParallelMode(true); //this doesn't help for speed right now(occt 6.9.1).
				BOPCheck.SetRunParallel(true); //performance boost, use all available cores
				BOPCheck.TangentMode() = true; //these 4 new tests add about 5% processing time.
				BOPCheck.MergeVertexMode() = true;
				BOPCheck.CurveOnSurfaceMode() = true;
				BOPCheck.MergeEdgeMode() = true;
				BOPCheck.Perform();
				if (!BOPCheck.HasFaulty())
					return true;

				str << "BOP check found the following errors:" << std::endl;
				static std::vector<std::string> shapeEnumToString = buildShapeEnumVector();
				static std::vector<std::string> bopEnumToString = buildBOPCheckResultVector();
				const BOPAlgo_ListOfCheckResult& BOPResults = BOPCheck.GetCheckResult();
				BOPAlgo_ListIteratorOfListOfCheckResult BOPResultsIt(BOPResults);
				for (; BOPResultsIt.More(); BOPResultsIt.Next()) {
					const BOPAlgo_CheckResult& current = BOPResultsIt.Value();

					const TopTools_ListOfShape& faultyShapes1 = current.GetFaultyShapes1();
					TopTools_ListIteratorOfListOfShape faultyShapes1It(faultyShapes1);
					for (;faultyShapes1It.More(); faultyShapes1It.Next()) {
						const TopoDS_Shape& faultyShape = faultyShapes1It.Value();
						str << "Error in " << shapeEnumToString[faultyShape.ShapeType()] << ": ";
						str << bopEnumToString[current.GetCheckStatus()] << std::endl;
					}
				}
				return false;
			}
		}
		return true;
	}
	bool TopoShape::isClosed() const
	{
		if (this->_Shape.IsNull())
			return false;
		bool closed = false;
		switch (this->_Shape.ShapeType()) {
		case TopAbs_SHELL:
		case TopAbs_WIRE:
		case TopAbs_EDGE:
			closed = BRep_Tool::IsClosed(this->_Shape) ? true : false;
			break;
		case TopAbs_COMPSOLID:
		case TopAbs_SOLID:
		{
			closed = true;
			TopExp_Explorer xp(this->_Shape, TopAbs_SHELL);
			while (xp.More()) {
				closed &= BRep_Tool::IsClosed(xp.Current()) ? true : false;
				xp.Next();
			}
		}
		break;
		case TopAbs_COMPOUND:
		{
			closed = true;
			TopExp_Explorer xp;
			for (xp.Init(this->_Shape, TopAbs_SHELL); xp.More(); xp.Next()) {
				closed &= BRep_Tool::IsClosed(xp.Current()) ? true : false;
			}
			for (xp.Init(this->_Shape, TopAbs_FACE, TopAbs_SHELL); xp.More(); xp.Next()) {
				closed &= BRep_Tool::IsClosed(xp.Current()) ? true : false;
			}
			for (xp.Init(this->_Shape, TopAbs_WIRE, TopAbs_FACE); xp.More(); xp.Next()) {
				closed &= BRep_Tool::IsClosed(xp.Current()) ? true : false;
			}
			for (xp.Init(this->_Shape, TopAbs_EDGE, TopAbs_WIRE); xp.More(); xp.Next()) {
				closed &= BRep_Tool::IsClosed(xp.Current()) ? true : false;
			}
			for (xp.Init(this->_Shape, TopAbs_VERTEX, TopAbs_EDGE); xp.More(); xp.Next()) {
				closed &= BRep_Tool::IsClosed(xp.Current()) ? true : false;
			}
		}
		break;
		case TopAbs_FACE:
		case TopAbs_VERTEX:
		case TopAbs_SHAPE:
			closed = BRep_Tool::IsClosed(this->_Shape) ? true : false;
			break;
		}
		return closed;
	}
	bool TopoShape::isCoplanar(const TopoShape& other, double tol) const
	{
		if (isNull() || other.isNull())
			return false;
		if (_Shape.IsEqual(other._Shape))
			return true;
		gp_Pln pln1, pln2;
		if (!findPlane(pln1, tol) || !other.findPlane(pln2, tol))
			return false;
		if (tol < 0.0)
			tol = Precision::Confusion();
		return pln1.Position().IsCoplanar(pln2.Position(), tol, tol);
	}
	bool TopoShape::findPlane(gp_Pln& pln, double tol, double atol) const
	{
		if (_Shape.IsNull()) {
			return false;
		}
		if (tol < 0.0) {
			tol = Precision::Confusion();
		}
		if (atol < 0.0) {
			atol = Precision::Angular();
		}
		TopoDS_Shape shape;
		if (countSubShapes(TopAbs_EDGE) == 1) {
			// To deal with OCCT bug of wrong edge transformation
			shape = BRepBuilderAPI_Copy(_Shape).Shape();
		}
		else {
			shape = _Shape;
		}     
		bool found = false;
		// BRepLib_FindSurface only really works on edges. We'll deal face first
		for (auto& shape : getSubShapes(TopAbs_FACE)) {
			gp_Pln plane;
			auto face = TopoDS::Face(shape);
			BRepAdaptor_Surface adapt(face);
			if (adapt.GetType() == GeomAbs_Plane) {
				plane = adapt.Plane();
			}
			else {
				TopLoc_Location loc;
				Handle(Geom_Surface) surf = BRep_Tool::Surface(face, loc);
				GeomLib_IsPlanarSurface check(surf);
				if (check.IsPlanar()) {
					plane = check.Plan();
				}
				else {
					return false;
				}
			}
			if (!found) {
				found = true;
				pln = plane;
			}
			else if (!pln.Position().IsCoplanar(plane.Position(), tol, atol)) {
				return false;
			}
		}

		// Check if there is free edges (i.e. edges does not belong to any face)
		if (TopExp_Explorer(getShape(), TopAbs_EDGE, TopAbs_FACE).More()) {
			// Copy shape to work around OCC transformation bug, that is, if
			// edge has transformation, but underlying geometry does not (or the
			// other way round), BRepLib_FindSurface returns a plane with the
			// wrong transformation
			BRepLib_FindSurface finder(BRepBuilderAPI_Copy(shape).Shape(), tol, Standard_True);
			if (!finder.Found()) {
				return false;
			}
			pln = GeomAdaptor_Surface(finder.Surface()).Plane();
			found = true;
		}

		// Check for free vertexes
		auto vertexes = getSubShapes(TopAbs_VERTEX, TopAbs_EDGE);
		if (vertexes.size()) {
			if (!found && vertexes.size() > 2) {
				BRep_Builder builder;
				TopoDS_Compound comp;
				builder.MakeCompound(comp);
				for (int i = 0, c = (int)vertexes.size() - 1; i < c; ++i) {
					builder.Add(comp,
						BRepBuilderAPI_MakeEdge(TopoDS::Vertex(vertexes[i]),
							TopoDS::Vertex(vertexes[i + 1]))
						.Edge());
				}
				BRepLib_FindSurface finder(comp, tol, Standard_True);
				if (!finder.Found()) {
					return false;
				}
				pln = GeomAdaptor_Surface(finder.Surface()).Plane();
				return true;
			}

			double tt = tol * tol;
			for (auto& v : vertexes) {
				if (pln.SquareDistance(BRep_Tool::Pnt(TopoDS::Vertex(v))) > tt) {
					return false;
				}
			}
		}

		// To make the returned plane normal more stable, if the shape has any
		// face, use the normal of the first face.
		if (hasSubShape(TopAbs_FACE)) {
			shape = getSubShape(TopAbs_FACE, 1);
			BRepAdaptor_Surface adapt(TopoDS::Face(shape));
			double u =
				adapt.FirstUParameter() + (adapt.LastUParameter() - adapt.FirstUParameter()) / 2.;
			double v =
				adapt.FirstVParameter() + (adapt.LastVParameter() - adapt.FirstVParameter()) / 2.;
			BRepLProp_SLProps prop(adapt, u, v, 2, Precision::Confusion());
			if (prop.IsNormalDefined()) {
				gp_Pnt pnt;
				gp_Vec vec;
				// handles the orientation state of the shape
				BRepGProp_Face(TopoDS::Face(shape)).Normal(u, v, pnt, vec);
				pln = gp_Pln(pnt, gp_Dir(vec));
			}
		}
		return true;
	}
	bool TopoShape::isInfinite() const
	{
		if (_Shape.IsNull())
			return false;
		// If the shape is empty an exception may be thrown
		Bnd_Box bounds;
		BRepBndLib::Add(_Shape, bounds);
		bounds.SetGap(0.0);
		Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
		bounds.Get(xMin, yMin, zMin, xMax, yMax, zMax);

		if (Precision::IsInfinite(xMax - xMin))
			return true;
		if (Precision::IsInfinite(yMax - yMin))
			return true;
		if (Precision::IsInfinite(zMax - zMin))
			return true;

		return false;
	}
	bool TopoShape::isPlanar(double tol) const
	{
		if (_Shape.IsNull() || _Shape.ShapeType() != TopAbs_FACE) {
			return false;
		}

		BRepAdaptor_Surface adapt(TopoDS::Face(_Shape));
		if (adapt.GetType() == GeomAbs_Plane) {
			return true;
		}

		TopLoc_Location loc;
		Handle(Geom_Surface) surf = BRep_Tool::Surface(TopoDS::Face(_Shape), loc);
		if (surf.IsNull()) {
			return false;
		}

		GeomLib_IsPlanarSurface check(surf, tol);
		return check.IsPlanar();
	}
	bool TopoShape::isPlanarFace(double tol) const
	{
		if (isNull() || getShape().ShapeType() != TopAbs_FACE) {
			return false;
		}
		GeomLib_IsPlanarSurface check(BRepAdaptor_Surface(TopoDS::Face(getShape())).Surface().Surface(), tol);
		if (!check.IsPlanar())
			return false;
		return true;
	}
	TopoDS_Shape TopoShape::cut(TopoDS_Shape shape) const
	{
		if (this->_Shape.IsNull())
			return this->_Shape;
		if (shape.IsNull())
			return this->_Shape;
		FCBRepAlgoAPI_Cut mkCut(this->_Shape, shape);
		return makeShell(mkCut.Shape());
	}

	TopoDS_Shape TopoShape::cut(const std::vector<TopoDS_Shape>& shapes, Standard_Real tolerance) const
	{
		if (this->_Shape.IsNull())
			return this->_Shape;
		FCBRepAlgoAPI_Cut mkCut;
		mkCut.SetRunParallel(true);
		TopTools_ListOfShape shapeArguments, shapeTools;
		shapeArguments.Append(this->_Shape);
		for (const auto& shape : shapes) {
			if (shape.IsNull()) {
				throw Base::ValueError("Tool shape is null");
			}
				
			shapeTools.Append(shape);
		}

		mkCut.SetArguments(shapeArguments);
		mkCut.SetTools(shapeTools);
		if (tolerance > 0.0) {
			mkCut.SetFuzzyValue(tolerance);
		}
		else if (tolerance < 0.0) {
			mkCut.setAutoFuzzy();
		}
		mkCut.Build();
		if (!mkCut.IsDone()) {
			throw Base::RuntimeError("Multi cut failed");
		}
			

		TopoDS_Shape resShape = mkCut.Shape();
		return makeShell(resShape);
	}

	TopoDS_Shape TopoShape::common(TopoDS_Shape shape) const
	{
		if (this->_Shape.IsNull())
			return this->_Shape;
		if (shape.IsNull())
			return shape;
		FCBRepAlgoAPI_Common mkCommon(this->_Shape, shape);
		return makeShell(mkCommon.Shape());
	}
	TopoDS_Shape TopoShape::common(const std::vector<TopoDS_Shape>& shapes, Standard_Real tolerance) const
	{
		if (this->_Shape.IsNull())
			return this->_Shape;
		FCBRepAlgoAPI_Common mkCommon;
		mkCommon.SetRunParallel(true);
		TopTools_ListOfShape shapeArguments, shapeTools;
		shapeArguments.Append(this->_Shape);
		for (const auto& shape : shapes) {
			if (shape.IsNull()) {
				throw Base::ValueError("Tool shape is null");
			}

			shapeTools.Append(shape);
		}

		mkCommon.SetArguments(shapeArguments);
		mkCommon.SetTools(shapeTools);
		if (tolerance > 0.0) {
			mkCommon.SetFuzzyValue(tolerance);
		}
		else if (tolerance < 0.0) {
			mkCommon.setAutoFuzzy();
		}
		mkCommon.Build();
		if (!mkCommon.IsDone())
		{
			throw Base::RuntimeError("Multi common failed");
		}


		TopoDS_Shape resShape = mkCommon.Shape();
		return makeShell(resShape);
	}

	TopoDS_Shape TopoShape::fuse(TopoDS_Shape shape) const
	{
		if (this->_Shape.IsNull())
			return shape;
		if (shape.IsNull())
			return this->_Shape;
		FCBRepAlgoAPI_Fuse mkFuse(this->_Shape, shape);
		return makeShell(mkFuse.Shape());
	}

	TopoDS_Shape TopoShape::fuse(const std::vector<TopoDS_Shape>& shapes, Standard_Real tolerance) const
	{
		if (this->_Shape.IsNull())
			Standard_Failure::Raise("Base shape is null");

		FCBRepAlgoAPI_Fuse mkFuse;
		mkFuse.SetRunParallel(true);
		TopTools_ListOfShape shapeArguments, shapeTools;
		shapeArguments.Append(this->_Shape);
		for (const auto& shape : shapes) {
			if (shape.IsNull())
			{
				//throw NullShapeException("Tool shape is null");
			}

			shapeTools.Append(shape);
		}
		mkFuse.SetArguments(shapeArguments);
		mkFuse.SetTools(shapeTools);
		if (tolerance > 0.0) {
			mkFuse.SetFuzzyValue(tolerance);
		}
		else if (tolerance < 0.0) {
			mkFuse.setAutoFuzzy();
		}
		mkFuse.Build();
		if (!mkFuse.IsDone())
		{
			throw Base::RuntimeError("Multi fuse failed");
		}


		TopoDS_Shape resShape = mkFuse.Shape();
		return makeShell(resShape);
	}

	TopoDS_Shape TopoShape::oldFuse(TopoDS_Shape shape) const
	{
		if (this->_Shape.IsNull())
			Standard_Failure::Raise("Base shape is null");
		if (shape.IsNull())
			Standard_Failure::Raise("Tool shape is null");

		throw Standard_Failure("BRepAlgo_Fuse is deprecated since OCCT 7.3");
	}

	TopoDS_Shape TopoShape::section(TopoDS_Shape shape, Standard_Boolean approximate) const
	{
		if (this->_Shape.IsNull())
			Standard_Failure::Raise("Base shape is null");
		if (shape.IsNull())
			Standard_Failure::Raise("Tool shape is null");
		FCBRepAlgoAPI_Section mkSection;
		mkSection.Init1(this->_Shape);
		mkSection.Init2(shape);
		mkSection.Approximation(approximate);
		mkSection.Build();
		if (!mkSection.IsDone())
			throw Base::RuntimeError("Section failed");
		return mkSection.Shape();
	}

	TopoDS_Shape TopoShape::section(const std::vector<TopoDS_Shape>& shapes,
		Standard_Real tolerance,
		Standard_Boolean approximate) const
	{
		if (this->_Shape.IsNull())
			Standard_Failure::Raise("Base shape is null");

		FCBRepAlgoAPI_Section mkSection;
		mkSection.SetRunParallel(true);
		mkSection.Approximation(approximate);
		TopTools_ListOfShape shapeArguments, shapeTools;
		shapeArguments.Append(this->_Shape);
		for (const auto& shape : shapes) {
			if (shape.IsNull())
			{
			  throw Base::ValueError("Tool shape is null");
			}
	
			shapeTools.Append(shape);
		}

		mkSection.SetArguments(shapeArguments);
		mkSection.SetTools(shapeTools);
		if (tolerance > 0.0) {
			mkSection.SetFuzzyValue(tolerance);
		}
		else if (tolerance < 0.0) {
			mkSection.setAutoFuzzy();
		}
		mkSection.Build();
		if (!mkSection.IsDone())
		{
			throw Base::RuntimeError("Multi section failed");
		}


		TopoDS_Shape resShape = mkSection.Shape();
		return resShape;
	}

	std::list<TopoDS_Wire> TopoShape::slice(const Eigen::Vector3f&dir, double d) const
	{
		return slice(Maths::FVector3(dir.x(),dir.y(),dir.z()),d);
	}

	std::list<TopoDS_Wire> TopoShape::slice(const Maths::FVector3&dir, double d) const
	{
		CrossSection cs(dir.x, dir.y, dir.z, this->_Shape);
		return cs.slice(d);
	}

	TopoDS_Compound TopoShape::slices(const Maths::FVector3&dir, const std::vector<double>&d) const
	{
		std::vector< std::list<TopoDS_Wire> > wire_list;
		CrossSection cs(dir.x, dir.y, dir.z, this->_Shape);
		for (double jt : d) {
			wire_list.push_back(cs.slice(jt));
		}

		std::vector< std::list<TopoDS_Wire> >::const_iterator ft;
		TopoDS_Compound comp;
		BRep_Builder builder;
		builder.MakeCompound(comp);

		for (ft = wire_list.begin(); ft != wire_list.end(); ++ft) {
			const std::list<TopoDS_Wire>& w = *ft;
			for (const auto& wt : w) {
				if (!wt.IsNull())
					builder.Add(comp, wt);
			}
		}
		return comp;
	}

	TopoDS_Shape TopoShape::generalFuse(const std::vector<TopoDS_Shape>& sOthers, Standard_Real tolerance, std::vector<TopTools_ListOfShape>* mapInOut) const
	{
		if (this->_Shape.IsNull())
			Standard_Failure::Raise("Base shape is null");

		BRepAlgoAPI_BuilderAlgo mkGFA;
		mkGFA.SetRunParallel(true);
		TopTools_ListOfShape GFAArguments;
		GFAArguments.Append(this->_Shape);
		for (const TopoDS_Shape& it : sOthers) {
			if (it.IsNull())
				throw NullShapeException("Tool shape is null");
			GFAArguments.Append(it);
		}
		mkGFA.SetArguments(GFAArguments);
		if (tolerance > 0.0) {
			mkGFA.SetFuzzyValue(tolerance);
		}
		else if (tolerance < 0.0) {
			FCBRepAlgoAPIHelper::setAutoFuzzy(&mkGFA);
		}
		mkGFA.SetNonDestructive(Standard_True);
		mkGFA.Build();
		if (!mkGFA.IsDone())
			throw BooleanException("MultiFusion failed");
		TopoDS_Shape resShape = mkGFA.Shape();
		if (mapInOut) {
			for (TopTools_ListIteratorOfListOfShape it(GFAArguments); it.More(); it.Next()) {
				mapInOut->push_back(mkGFA.Modified(it.Value()));
			}
		}
		return resShape;
	}


	const std::vector<TopoDS_Shape>& MapperMaker::modified(const TopoDS_Shape& s) const
	{
		_res.clear();
		try {
			TopTools_ListIteratorOfListOfShape it;
			for (it.Initialize(maker.Modified(s)); it.More(); it.Next()) {
				_res.push_back(it.Value());
			}
		}
		catch (const Standard_Failure& e) {
			//if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG)) {
			//	FC_WARN("Exception on shape mapper: " << e.GetMessageString());
			//}
		}
		return _res;
	}

	const std::vector<TopoDS_Shape>& MapperMaker::generated(const TopoDS_Shape& s) const
	{
		_res.clear();
		try {
			TopTools_ListIteratorOfListOfShape it;
			for (it.Initialize(maker.Generated(s)); it.More(); it.Next()) {
				_res.push_back(it.Value());
			}
		}
		catch (const Standard_Failure& e) {
			//if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG)) {
			//	FC_WARN("Exception on shape mapper: " << e.GetMessageString());
			//}
		}
		return _res;
	}

	MapperHistory::MapperHistory(const Handle(BRepTools_History)& history) : history(history)
	{

	}

	MapperHistory::MapperHistory(const Handle(BRepTools_ReShape)& reshape)
	{		
		if (reshape) {
			history = reshape->History();
		}
	}

	MapperHistory::MapperHistory(ShapeFix_Root& fix)
	{
		if (fix.Context()) {
			history = fix.Context()->History();
		}
	}

	const std::vector<TopoDS_Shape>& MapperHistory::modified(const TopoDS_Shape& s) const
	{
		_res.clear();
		try {
			if (history) {
				TopTools_ListIteratorOfListOfShape it;
				for (it.Initialize(history->Modified(s)); it.More(); it.Next()) {
					_res.push_back(it.Value());
				}
			}
		}
		catch (const Standard_Failure& e) {
			//if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG)) {
			//	FC_WARN("Exception on shape mapper: " << e.GetMessageString());
			//}
		}
		return _res;
	}

	const std::vector<TopoDS_Shape>& MapperHistory::generated(const TopoDS_Shape& s) const
	{
		_res.clear();
		try {
			if (history) {
				TopTools_ListIteratorOfListOfShape it;
				for (it.Initialize(history->Generated(s)); it.More(); it.Next()) {
					_res.push_back(it.Value());
				}
			}
		}
		catch (const Standard_Failure& e) {
			///*
			//if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG)) {
			//	FC_WARN("Exception on shape mapper: " << e.GetMessageString());
			//}*/
		}
		return _res;
	}

	NullShapeException::NullShapeException() : ValueError()
	{
	}

	NullShapeException::NullShapeException(const char* sMessage) : ValueError(sMessage)
	{
	}

	NullShapeException::NullShapeException(const std::string& sMessage) : ValueError(sMessage)
	{
	}

	BooleanException::BooleanException() : CADKernelError()
	{
	}

	BooleanException::BooleanException(const char* sMessage) : CADKernelError(sMessage)
	{
	}

	BooleanException::BooleanException(const std::string& sMessage) : CADKernelError(sMessage)
	{
	}

}