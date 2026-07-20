// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2025 Sayantan Deb <sayantandebin[at]gmail.com>           *
 *                                                                          *
 *   This file is part of FreeCAD.                                          *
 *                                                                          *
 *   FreeCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FreeCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with FreeCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ***************************************************************************/

#pragma once

#include <Base/Vector3D.h>
#include <Geometry.h>
#include <TopoShape.h>


struct  EdgeMidPointProps
{
    Base::Vector3d position;
    Base::Vector3d tangent;
    double middle;
};
EdgeMidPointProps  getEdgeMidPointProps(Part::TopoShape& edge);

Base::Vector3d  getCentreOfMassFromFace(TopoDS_Face& face);

struct  PointOnFaceNearEdgeProps
{
    enum class State : std::uint8_t
    {
        OnFace,
        OutsideFace,
        Undefined
    };

    Base::Vector3d position;
    Base::Vector3d normal;
    State state;
};
PointOnFaceNearEdgeProps 
getFaceNormalFromPointNearEdge(Part::TopoShape& edge, double middle, TopoDS_Face& face);

Base::Vector3d  getFaceNormalFromPoint(Base::Vector3d& point, TopoDS_Face& face);

std::pair<TopoDS_Face, TopoDS_Face> 
getAdjacentFacesFromEdge(Part::TopoShape& edge, Part::TopoShape& baseShape);

struct  DraggerPlacementProps
{
    Base::Vector3d position;
    Base::Vector3d dir;
};
DraggerPlacementProps 
getDraggerPlacementFromEdgeAndFace(Part::TopoShape& edge, TopoDS_Face& face);

DraggerPlacementProps 
getDraggerPlacementFromEdgeAndFace(Part::TopoShape& edge, Part::TopoShape& face);

std::vector<Part::TopoShape>  getAdjacentEdgesFromFace(Part::TopoShape& face);

Base::Vector3d  getMidPointFromFace(Part::TopoShape& face);

Base::Vector3d  getMidPointFromProfile(Part::TopoShape& profile);

struct  DraggerNormalProps
{
    Base::Vector3d normal;
    Base::Vector3d faceNormal;
};
struct  DraggerPlacementPropsWithNormals
{
    DraggerPlacementProps placementProps;
    std::optional<DraggerNormalProps> normalProps;
};
std::optional<DraggerPlacementPropsWithNormals> 
getDraggerPlacementFromPlaneAndFace(Part::TopoShape& face, gp_Pln& plane);
