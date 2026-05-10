#include "Sketcher/SketcherObj.h"
namespace MOON {
    SketcherObj::SketcherObj()
    {
    }
    SketcherObj::~SketcherObj()
    {
    }
    void SketcherObj::setPlane(int p)
    {
        mPlane = p;
    }
    int SketcherObj::getPlane()
    {
        return mPlane;
    }
}