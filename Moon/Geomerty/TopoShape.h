#pragma once
#include "Geomerty/GeoData.h"
#include "Maths/FMatrix4.h"
#include "Rendering/Geometry/bbox.h"
#include <TopoDS_Compound.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BRepBuilderAPI_MakeShape.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <BRepFeat_MakePrism.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>
#include <BRepTools_History.hxx>
#include <BRepTools_ReShape.hxx>
#include <ShapeFix_Root.hxx>
#include "ElementMap.h"
#include "MappedName.h"
#include "base/Exception.h"
#include <utility>

class gp_Ax1;
class gp_Ax2;
class gp_Pln;
class gp_Vec;
using namespace Part;
namespace MOON {
    struct ShapeHasher;
    class TopoShape;
    class TopoShapeCache;
    using TopoShapeMap = std::unordered_map<TopoShape, TopoShape, ShapeHasher, ShapeHasher>;
    /* A special sub-class to indicate null shapes
 */
 // NOLINTNEXTLINE cppcoreguidelines-special-member-functions
    class  NullShapeException : public Base::ValueError
    {
    public:
        /// Construction
        NullShapeException();
        explicit NullShapeException(const char* sMessage);
        explicit NullShapeException(const std::string& sMessage);
        /// Destruction
        ~NullShapeException() noexcept override = default;
    };

    /* A special sub-class to indicate boolean failures
     */
     // NOLINTNEXTLINE cppcoreguidelines-special-member-functions
    class  BooleanException : public Base::CADKernelError
    {
    public:
        /// Construction
        BooleanException();
        explicit BooleanException(const char* sMessage);
        explicit BooleanException(const std::string& sMessage);
        /// Destruction
        ~BooleanException() noexcept override = default;
    };
    enum HistoryTraceType
    {
        stopOnTypeChange,
        followTypeChange
    };
	class TopoShape {
	public:
        struct Mapper {
            /// Helper vector for temporary storage of both generated and modified shapes
            mutable std::vector<TopoDS_Shape> _res;
            virtual ~Mapper() {}
            /// Return a list of shape generated from the given input shape
            virtual const std::vector<TopoDS_Shape>& generated(const TopoDS_Shape&) const {
                return _res;
            }
            /// Return a list of shape modified from the given input shape
            virtual const std::vector<TopoDS_Shape>& modified(const TopoDS_Shape&) const {
                return _res;
            }
        };
        TopoShape(long Tag=0,   // NOLINT google-explicit-constructor
            App::StringHasherRef hasher = App::StringHasherRef(),
            const TopoDS_Shape& shape = TopoDS_Shape());  // Cannot be made explicit
        TopoShape(const TopoDS_Shape&,  // NOLINT google-explicit-constructor
            long Tag = 0,
            App::StringHasherRef hasher = App::StringHasherRef());  // Cannot be made explicit
		TopoShape(const TopoShape& shape);
        void operator=(const TopoShape&);
		~TopoShape();
        Data::ElementMapPtr elementMap(bool flush = true) const;
        Data::ElementMapPtr resetElementMap(
            Data::ElementMapPtr elementMap = Data::ElementMapPtr()) ;

        double getAccuracy() const;
		void getPoints(std::vector<Vector3d>& Points,
			std::vector<Vector3d>& Normals,
			double Accuracy,
			uint16_t flags = 0) const ;
        void getFaces(std::vector<Vector3d>& Points,
            std::vector<Vector3d>& Normals,
            std::vector<unsigned int>&indices,
            double Accuracy,
            uint16_t flags = 0) const;
        /** Get lines from object with given accuracy */
        void getLines(std::vector<Vector3d>& Points,
            std::vector<Part::Line>& lines,
            double Accuracy,
            uint16_t flags = 0) const;
		void getFaces(std::vector<Vector3d>& Points,
			std::vector<Facet>& faces,
			double Accuracy,
			uint16_t flags = 0) const ;
		void setFaces(const std::vector<Vector3d>& Points,
			const std::vector<Facet>& faces,
			double tolerance = 1.0e-06);  // NOLINT
        void getDomainfaces(std::vector<Domain>&domains, double accuracy)const;
		void getDomains(std::vector<Domain>&) const;
        void setTransform(const Maths::FMatrix4& rclTrf) ;
        TopoDS_Shape makeShell(const TopoDS_Shape&)const;
        /// get the transformation of the CasCade Shape
        Maths::FMatrix4 getTransform() const ;
        Rendering::Geometry::bbox getBoundBox() const;
        bool getCenterOfGravity(Vector3d& center) const;
        static void convertTogpTrsf(const Maths::FMatrix4& mtrx, gp_Trsf& trsf);
        static void convertToMatrix(const gp_Trsf& trsf, Maths::FMatrix4& mtrx);
        static Maths::FMatrix4 convert(const gp_Trsf& trsf);
        static gp_Trsf convert(const Maths::FMatrix4& mtrx);
        static TopoDS_Shape moved(const TopoDS_Shape& tds, const TopLoc_Location& loc);
        static TopoDS_Shape& move(TopoDS_Shape& tds, const TopLoc_Location& loc);
        static TopoDS_Shape& locate(TopoDS_Shape& tds, const TopLoc_Location& loc);
        static TopoDS_Shape located(const TopoDS_Shape& tds, const TopLoc_Location& loc);
        static const std::string& shapeName(TopAbs_ShapeEnum type, bool silent = false);
        void importStep(const char* FileName);
        mutable long Tag{ 0 };
        mutable App::StringHasherRef Hasher;
    private:
        /** Get lines from sub-shape */
        void getLinesFromSubShape(const TopoDS_Shape& shape,
            std::vector<Vector3d>& vertices,
            std::vector<Line>& lines) const;
        void getFacesFromDomains(const std::vector<Domain>& domains,
            std::vector<Vector3d>& vertices,
            std::vector<Facet>& faces) const;
        friend class TopoShapeCache;
    private:
        // Cache storage
        mutable std::shared_ptr<TopoShapeCache> _parentCache;
        mutable std::shared_ptr<TopoShapeCache> _cache;
        mutable TopLoc_Location _subLocation;
        Data::ElementMapPtr _elementMap;
       
        class ShapeProtector : public TopoDS_Shape
        {
        public:
            using TopoDS_Shape::TopoDS_Shape;
            using TopoDS_Shape::operator=;

            TopoShape* _owner;
            explicit ShapeProtector(TopoShape& owner)
                : _owner(&owner)
            {
            }

            ShapeProtector(TopoShape& owner, const TopoDS_Shape& shape)
                : TopoDS_Shape(shape), _owner(&owner)
            {
            }

            void Nullify()
            {
                if (!this->IsNull()) {
                    _owner->resetElementMap();
                    _owner->_cache.reset();
                    _owner->_parentCache.reset();
                }
            }

            const TopLoc_Location& Location() const
            {
                // Some platforms do not support "using TopoDS_Shape::Location" here because of an
                // ambiguous lookup, so implement it manually.
                return TopoDS_Shape::Location();
            }

            void Location(const TopLoc_Location& Loc)
            {
                // Location does not affect element map or cache
               TopoShape::locate(*dynamic_cast<TopoDS_Shape*>(this), Loc);
            }

            void Move(const TopLoc_Location& position)
            {
                // Move does not affect element map or cache
                TopoShape::move(*dynamic_cast<TopoDS_Shape*>(this), position);
            }

            using TopoDS_Shape::Orientation;
            void Orientation(const TopAbs_Orientation Orient)
            {
                //owner->flushElementMap();
                TopoDS_Shape::Orientation(Orient);
                if (_owner->_cache) {
                    _owner->initCache();
                }
            }

            void Reverse()
            {
               // _owner->flushElementMap();
                TopoDS_Shape::Reverse();
                 if (_owner->_cache) {
                    _owner->initCache();
                }
            }

            void Complement()
            {
                //_owner->flushElementMap();
                TopoDS_Shape::Complement();
                if (_owner->_cache) {
                    _owner->initCache();
                }
            }

            void Compose(const TopAbs_Orientation Orient)
            {
                //_owner->flushElementMap();
                TopoDS_Shape::Compose(Orient);
                if (_owner->_cache) {
                    _owner->initCache();
                }
            }

            void EmptyCopy()
            {
              //_owner->flushElementMap();
                TopoDS_Shape::EmptyCopy();
                if (_owner->_cache) {
                    _owner->initCache();
                }
            }

            void TShape(const Handle(TopoDS_TShape)& T)
            {
                //_owner->flushElementMap();
                TopoDS_Shape::TShape(T);
                if (_owner->_cache) {
                    _owner->initCache();
                }
            }

        };

      
    public: 
        TopoShape& makeElementCopy(const TopoShape& source,
            const char* op = nullptr,
            bool copyGeom = true,
            bool copyMesh = false);

        /** Make a deep copy of the shape
         *
         * @param op: optional string to be encoded into topo naming for indicating
         *            the operation
         * @param copyGeom: whether to copy internal geometry of the shape
         * @param copyMesh: whether to copy internal meshes of the shape
         *
         * @return Return a deep copy of the shape. The shape itself is not
         *         modified
         */
        TopoShape
            makeElementCopy(const char* op = nullptr, bool copyGeom = true, bool copyMesh = false) const
        {
            return TopoShape(Tag, Hasher).makeElementCopy(*this, op, copyGeom, copyMesh);
        }
        bool fix();
        enum class ConnectionPolicy
        {
            requireSharedVertex,
            mergeWithTolerance
        };
        TopoShape& makeElementWires(const std::vector<TopoShape>& shapes,
            const char* op = nullptr,
            double tol = 0.0,
            ConnectionPolicy policy = ConnectionPolicy::mergeWithTolerance,
            TopoShapeMap* output = nullptr);
        TopoShape& makeElementWires(const TopoShape& shape,
            const char* op = nullptr,
            double tol = 0.0,
            ConnectionPolicy policy = ConnectionPolicy::mergeWithTolerance,
            TopoShapeMap* output = nullptr);
        TopoShape makeElementWires(const char* op = nullptr,
            double tol = 0.0,
            ConnectionPolicy policy = ConnectionPolicy::mergeWithTolerance,
            TopoShapeMap* output = nullptr) const
        {
            return TopoShape(0, Hasher).makeElementWires(*this, op, tol, policy, output);
        }
        Data::MappedName getMappedName(const Data::IndexedName& element,
            bool allowUnmapped = false,
            Data::ElementIDRefs* sid = nullptr) const;
        TopoShape& makeShapeWithElementMap(const TopoDS_Shape& shape,
            const Mapper& mapper,
            const std::vector<TopoShape>& sources,
            const char* op = nullptr);
        TopoShape& makeElementShape(BRepBuilderAPI_MakeShape& mkShape,
            const std::vector<TopoShape>& sources,
            const char* op = nullptr);
        TopoShape& makeElementShape(BRepBuilderAPI_MakeShape& mkShape,
            const TopoShape& source,
            const char* op = nullptr);
        TopAbs_ShapeEnum shapeType(bool silent = false) const;
        Data::ElementMapPtr ensureElementMap(bool flush = true);
        std::vector<std::pair<Data::MappedName, Data::ElementIDRefs> >
            getElementMappedNames(const Data::IndexedName& element, bool needUnmapped = false) const;
        void setMappedChildElements(const std::vector<Data::ElementMap::MappedChildElements>& children);
        void setupChild(Data::ElementMap::MappedChildElements& child,
            TopAbs_ShapeEnum elementType,
            const TopoShape& topoShape,
            size_t shapeCount,
            const char* op);
        void copyElementMap(const TopoShape& topoShape, const char* op = nullptr);
        /// Get the current element map size
        size_t getElementMapSize(bool flush = true) const;
        bool hasPendingElementMap() const;
        bool canMapElement(const TopoShape& other) const;
        void mapSubElement(const TopoShape& other, const char* op = nullptr, bool forceHasher = false);
        void mapSubElement(const std::vector<TopoShape>& shapes, const char* op = nullptr);
        /**
 * When given a single shape to create a compound, two results are possible: either to simply
 * return the shape as given, or to force it to be placed in a Compound.
 */
        enum class SingleShapeCompoundCreationPolicy {
            returnShape,
            forceCompound
        };
        struct BRepFillingParams;
        enum class Continuity {
            /// Only geometric continuity
            C0,
            /** for each point on the curve, the tangent vectors 'on the right' and 'on
            *  the left' are collinear with the same orientation.
            */
            G1,
            /** Continuity of the first derivative. The 'C1' curve is also 'G1' but, in
            *  addition, the tangent vectors 'on the right' and 'on the left' are equal.
            */
            C1,

            /** For each point on the curve, the normalized normal vectors 'on the
            *  right' and 'on the left' are equal.
            */
            G2,

            /// Continuity of the second derivative.
            C2,

            /// Continuity of the third derivative.
            C3,

            /** Continuity of the N-th derivative, whatever is the value given for N
            * (infinite order of continuity). Also provides information about the
            * continuity of a surface.
            */
            CN,
        };
        /** Make a compound shape
         *
         * @param shapes: input shapes
         * @param op: optional string to be encoded into topo naming for indicating
         *            the operation
         * @param policy: set behavior when only a single shape is given
         *
         * @return The original content of this TopoShape is discarded and replaced
         *         with the new shape. The function returns the TopoShape itself as
         *         a reference so that multiple operations can be carried out for
         *         the same shape in the same line of code.
         */
        TopoShape& makeElementCompound(const std::vector<TopoShape>& shapes,
            const char* op = nullptr,
            SingleShapeCompoundCreationPolicy policy =
            SingleShapeCompoundCreationPolicy::forceCompound);
        void initCache(int reset = 0) const;
        int findShape(const TopoDS_Shape& subshape) const;
        TopoDS_Shape findAncestorShape(const TopoDS_Shape& subshape, TopAbs_ShapeEnum type) const;
        std::vector<TopoDS_Shape> findAncestorsShapes(const TopoDS_Shape& subshape,
            TopAbs_ShapeEnum type) const;
        void setShape(const TopoDS_Shape& shape, bool resetElementMap = true);
        ShapeProtector _Shape;
        inline const TopoDS_Shape& getShape() const
        {
            return this->_Shape;
        }
        TopoShape getSubTopoShape(TopAbs_ShapeEnum type, int idx, bool silent = false) const;
        TopoDS_Shape getSubShape(TopAbs_ShapeEnum type, int idx, bool silent = false) const;
        bool hasSubShape(TopAbs_ShapeEnum type) const;
 /**
 * Locate all of the sub TopoDS_Shapes of a given type, while avoiding a given type
 * @param type The type to find
 * @param avoid The type to avoid
 * @return The sub TopoDS_Shapes.
 */
        std::vector<TopoDS_Shape> getSubShapes(TopAbs_ShapeEnum type = TopAbs_SHAPE, TopAbs_ShapeEnum avoid = TopAbs_SHAPE) const;
        /**
 * Locate all of the sub TopoShapes of a given type, while avoiding a given type
 * @param type The type to find
 * @param avoid The type to avoid
 * @return The sub TopoShapes.
 */
        std::vector<TopoShape> getSubTopoShapes(TopAbs_ShapeEnum type = TopAbs_SHAPE, TopAbs_ShapeEnum avoid = TopAbs_SHAPE) const;
        unsigned long countSubShapes(TopAbs_ShapeEnum type) const;
        bool isNull() const;
        bool isValid() const;
        bool analyze(bool runBopCheck, std::ostream&) const;
        bool isClosed() const;
        bool isCoplanar(const TopoShape& other, double tol = -1) const;
        bool findPlane(gp_Pln& plane, double tol = -1, double atol = -1) const;
        /// Returns true if the expansion of the shape is infinite, false otherwise
        bool isInfinite() const;
        /// Checks whether the shape is a planar face
        bool isPlanar(double tol = 1.0e-7) const;   // NOLINT
        /// Check if this shape is a single linear edge, works on BSplineCurve and BezierCurve
       // bool isLinearEdge(Base::Vector3d* dir = nullptr, Base::Vector3d* base = nullptr) const;
        /// Check if this shape is a single planar face, works on BSplineSurface and BezierSurface
        bool isPlanarFace(double tol = 1e-7) const;   // NOLINT
        TopoDS_Shape cut(TopoDS_Shape) const;
        TopoDS_Shape cut(const std::vector<TopoDS_Shape>&, Standard_Real tolerance = -1.0) const;
        TopoDS_Shape common(TopoDS_Shape) const;
        TopoDS_Shape common(const std::vector<TopoDS_Shape>&, Standard_Real tolerance = -1.0) const;
        TopoDS_Shape fuse(TopoDS_Shape) const;
        TopoDS_Shape fuse(const std::vector<TopoDS_Shape>&, Standard_Real tolerance = -1.0) const;
        TopoDS_Shape oldFuse(TopoDS_Shape) const;
        TopoDS_Shape section(TopoDS_Shape, Standard_Boolean approximate = Standard_False) const;
        TopoDS_Shape section(const std::vector<TopoDS_Shape>&,
            Standard_Real tolerance = -1.0,
            Standard_Boolean approximate = Standard_False) const;
        std::list<TopoDS_Wire> slice(const Eigen::Vector3f&, double) const;
        std::list<TopoDS_Wire> slice(const Maths::FVector3&, double) const;
        TopoDS_Compound slices(const Maths::FVector3&, const std::vector<double>&) const;
     
        TopoDS_Shape generalFuse(const std::vector<TopoDS_Shape>& sOthers,
            Standard_Real tolerance,
            std::vector<TopTools_ListOfShape>* mapInOut = nullptr) const;

    };
    struct MapperHistory : TopoShape::Mapper
    {
        Handle(BRepTools_History) history;
        explicit MapperHistory(const Handle(BRepTools_History)& history);
        explicit MapperHistory(const Handle(BRepTools_ReShape)& reshape);
        explicit MapperHistory(ShapeFix_Root& fix);
        const std::vector<TopoDS_Shape>& modified(const TopoDS_Shape& s) const override;
        const std::vector<TopoDS_Shape>& generated(const TopoDS_Shape& s) const override;
    };
    struct  MapperMaker : TopoShape::Mapper
    {
        BRepBuilderAPI_MakeShape& maker;
        explicit MapperMaker(BRepBuilderAPI_MakeShape& maker)
            : maker(maker)
        {
        }
        const std::vector<TopoDS_Shape>& modified(const TopoDS_Shape& s) const override;
        const std::vector<TopoDS_Shape>& generated(const TopoDS_Shape& s) const override;
    };
}