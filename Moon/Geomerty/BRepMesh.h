#pragma once

#include "Geomerty/GeoData.h"
namespace MOON {

class  BRepMesh
{
public:
    
    using Segment = std::vector<std::size_t>;

    void getFacesFromDomains(const std::vector<Domain>& domains,
                             std::vector<Vector3d>& points,
                             std::vector<Facet>& faces);
    std::vector<Segment> createSegments() const;

private:
    std::vector<std::size_t> domainSizes;
};

}
