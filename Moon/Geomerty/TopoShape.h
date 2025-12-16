#pragma once
#include "Maths/FVector3.h"
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

class gp_Ax1;
class gp_Ax2;
class gp_Pln;
class gp_Vec;

namespace MOON {
	class TopoShape {
	public:
		struct Line { uint32_t I1; uint32_t I2; };
		struct Facet { uint32_t I1; uint32_t I2; uint32_t I3; };
		struct Domain {
			std::vector<Maths::FVector3> points;
			std::vector<Facet> facets;
		};
		TopoShape(const TopoDS_Shape& shape = TopoDS_Shape());
		~TopoShape();
		/** Get points from object with given accuracy */
		void getPoints(std::vector<Maths::FVector3>& Points,
			std::vector<Maths::FVector3>& Normals,
			double Accuracy,
			uint16_t flags = 0) const ;
		void getFaces(std::vector<Maths::FVector3>& Points,
			std::vector<Facet>& faces,
			double Accuracy,
			uint16_t flags = 0) const ;
		void setFaces(const std::vector<Maths::FVector3>& Points,
			const std::vector<Facet>& faces,
			double tolerance = 1.0e-06);  // NOLINT
		void getDomains(std::vector<Domain>&) const;

        static TopoDS_Shape& move(TopoDS_Shape& tds, const TopLoc_Location& loc);
        static TopoDS_Shape& locate(TopoDS_Shape& tds, const TopLoc_Location& loc);
	private:

        class ShapeProtector : public TopoDS_Shape
        {
        public:
            using TopoDS_Shape::TopoDS_Shape;
            using TopoDS_Shape::operator=;

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
                    //_owner->resetElementMap();
                    //_owner->_cache.reset();
                    //_owner->_parentCache.reset();
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
                //if (_owner->_cache) {
                   // _owner->initCache();
                //}
            }

            void Reverse()
            {
               // _owner->flushElementMap();
                TopoDS_Shape::Reverse();
               // if (_owner->_cache) {
                 ///   _owner->initCache();
               // }
            }

            void Complement()
            {
               /// _owner->flushElementMap();
                TopoDS_Shape::Complement();
                //if (_owner->_cache) {
                ///    _owner->initCache();
                //}
            }

            void Compose(const TopAbs_Orientation Orient)
            {
                //_owner->flushElementMap();
                TopoDS_Shape::Compose(Orient);
                //if (_owner->_cache) {
                //    _owner->initCache();
                //}
            }

            void EmptyCopy()
            {
              //_owner->flushElementMap();
                TopoDS_Shape::EmptyCopy();
               // if (_owner->_cache) {
               //     _owner->initCache();
               // }
            }

            void TShape(const Handle(TopoDS_TShape)& T)
            {
                //_owner->flushElementMap();
                TopoDS_Shape::TShape(T);
                //if (_owner->_cache) {
                //    _owner->initCache();
                //}
            }

            TopoShape* _owner;
        };

        ShapeProtector _Shape;

	};
}