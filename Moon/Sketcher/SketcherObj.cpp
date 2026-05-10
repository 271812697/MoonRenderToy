#include "Sketcher/SketcherObj.h"
#include "renderer/SceneView.h"
#include "Core/Global/ServiceLocator.h"
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
        auto& view = GetService(Editor::Panels::SceneView);
        view.GetCameraController().EnableRotate(false);
        if (mPlane == 0) {
            view.LookAt({0,0,0},{1,0,0},1);
        }
        if (mPlane == 1) {
            view.LookAt({ 0,0,0 }, { 0,1,0 }, 1);
        }
        if (mPlane == 2) {
            view.LookAt({ 0,0,0 }, { 0,0,1 }, 1);
        }
    }
    int SketcherObj::getPlane()
    {
        return mPlane;
    }
    void SketcherObj::makeDone()
    {
        auto& view = GetService(Editor::Panels::SceneView);
        view.GetCameraController().EnableRotate(true);
    }
}