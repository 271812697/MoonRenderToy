

#ifndef FREECAD_TOPOSHAPECACHE_H
#define FREECAD_TOPOSHAPECACHE_H

#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <utility>

#include <ElementMap.h>

#include "TopoShape.h"

namespace Part
{

struct  ShapeRelationKey
{
    Data::MappedName name;
    HistoryTraceType historyTraceType;

    ShapeRelationKey(Data::MappedName name, HistoryTraceType historyTraceType);
    bool operator<(const ShapeRelationKey& other) const;
};

class  TopoShapeCache: public std::enable_shared_from_this<TopoShapeCache>
{
public:
    /// Reference counted element map for the owner TopoShape. The ElementMap of
    /// a TopoShape is normally accessed through the inherited member function
    /// ComplexGeoData::elementMap(). The extra shared pointer here is so that
    /// other TopoShape instances with the same Cache can reuse the map once
    /// generated.
    Data::ElementMapPtr cachedElementMap;

    /// Location of the original cached TopoDS_Shape.
    TopLoc_Location subLocation;

    /// The cached TopoDS_Shape stripped of any location (i.e. a null TopoDS_Shape::myLocation).
    TopoDS_Shape shape;

    /// Location of the last ancestor shape used to find this TopoShape. These two members are used
    /// to avoid repetitive inverting the location of the same ancestor.
    TopLoc_Location location;

    /// Inverse of location
    TopLoc_Location locationInverse;

    struct  AncestorInfo
    {
        bool initialized = false;
        TopTools_IndexedDataMapOfShapeListOfShape shapes;
    };

    /// Class for caching the ancestor and children shapes mapping
    class  Ancestry
    {
    private:
        TopoShapeCache* owner = nullptr;

        /// OCCT map from the owner TopoShape to a list of children (i.e. lower hierarchical)
        /// TopoDS_Shape
        TopTools_IndexedMapOfShape shapes;

        /// One-to-one corresponding TopoShape to each child TopoDS_Shape
        std::vector<TopoShape> topoShapes;

        /// Caches the OCCT ancestor shape maps, e.g.
        ///     Cache::shapeAncestryCache[TopAbs_FACE].ancestors[TopAbs_EDGE]
        /// stores an OCCT TopTools_IndexedDataMapOfShapeListOfShape that can return a list of
        /// faces containing a given edge.
        std::array<AncestorInfo, TopAbs_SHAPE + 1> ancestors;

        TopoShape _getTopoShape(const TopoShape& parent, int index);

    public:
        void clear();
        TopoShape getTopoShape(const TopoShape& parent, int index);
        std::vector<TopoShape> getTopoShapes(const TopoShape& parent);
        TopoDS_Shape stripLocation(const TopoDS_Shape& parent, const TopoDS_Shape& child);
        int find(const TopoDS_Shape& parent, const TopoDS_Shape& subShape);
        TopoDS_Shape find(const TopoDS_Shape& parent, int index);
        int count() const;
        bool empty() const;

        friend TopoShapeCache;
    };

    explicit TopoShapeCache(const TopoDS_Shape& tds);
    void insertRelation(const ShapeRelationKey& key, const QVector<Data::MappedElement>& value);
    bool isTouched(const TopoDS_Shape& tds) const;
    Ancestry& getAncestry(TopAbs_ShapeEnum type);
    int countShape(TopAbs_ShapeEnum type);
    int findShape(const TopoDS_Shape& parent, const TopoDS_Shape& subShape);
    TopoDS_Shape findShape(const TopoDS_Shape& parent, TopAbs_ShapeEnum type, int index);

    /// Given a parent shape and a child (sub) shape, call TopExp::MapShapesAndAncestors and cache
    /// the result. Subsequent calls to this method given unchanged geometry will use the cached
    /// data rather than re-running MapShapesAndAncestors.
    /// If ancestors is given, it is cleared and overwritten with the ancestry data.
    TopoDS_Shape findAncestor(
        const TopoDS_Shape& parent,
        const TopoDS_Shape& subShape,
        TopAbs_ShapeEnum type,
        std::vector<TopoDS_Shape>* ancestors = nullptr
    );

    /// Ancestor and children shape caches of all shape types. Note that
    /// shapeAncestryCache[TopAbs_SHAPE] is also valid and stores the direct children of a
    /// compound shape.
    std::array<Ancestry, TopAbs_SHAPE + 1> shapeAncestryCache;

    std::map<ShapeRelationKey, QVector<Data::MappedElement>> relations;
};

}  // namespace Part

#endif  // FREECAD_TOPOSHAPECACHE_H
