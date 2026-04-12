/***************************************************************************
 *   Copyright (c) 2019 Abdullah Tahiri <abdullah.tahiri.yo@gmail.com>     *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef PART_GEOMETRYEXTENSION_H
#define PART_GEOMETRYEXTENSION_H

#include <memory>
#include <string>
#include "base/BaseClass.h"



namespace Part {

class Geometry;

class GeometryExtension: public Base::BaseClass
{
    TYPESYSTEM_HEADER_WITH_OVERRIDE();
public:
    ~GeometryExtension() override = default;

    virtual std::unique_ptr<GeometryExtension> copy() const = 0;

    inline void setName(const std::string& str) {name = str;}
    inline const std::string &getName () const {return name;}

    // Default method to notify an extension that it has been attached
    // to a given geometry
    virtual void notifyAttachment(Part::Geometry *) {}

protected:
    GeometryExtension();
    GeometryExtension(const GeometryExtension &obj) = default;
    GeometryExtension& operator= (const GeometryExtension &obj) = default;

    virtual void copyAttributes(Part::GeometryExtension * cpy) const;

private:
    std::string name;
};



class GeometryPersistenceExtension : public Part::GeometryExtension
{
    TYPESYSTEM_HEADER_WITH_OVERRIDE();
public:
    ~GeometryPersistenceExtension() override = default;


    bool isSame(const GeometryPersistenceExtension &other) const;

};

}

#endif // PART_GEOMETRYEXTENSION_H
