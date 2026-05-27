#pragma once
#include "DrawSketchHandler.h"
#include "Gizmo/Gizmo.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
#include "Qtimgui/imgui/imgui.h"
#include "renderer/SceneView.h"
#include "Sketcher/SketcheTool2D.h"
namespace MOON
{

    void DrawSketchHandler::quit()
    {
    }
    void DrawSketchHandler::clearEdit()
    {
        lines.clear();
    }
    void DrawSketchHandler::makePlane(int v)
    {
        plane = v;
        if (plane == 2) {
            planeNormal = { 0,0,1 };
        }
        else if (plane == 0)
        {
            planeNormal = { 1,0,0 };
        }
        else
        {
            planeNormal = { 0,1,0 };
        }
    }
    void DrawSketchHandler::onUpdate()
    {
       auto sketchobj= SketcherObjManager::instance().GetCurrentActiveSketcherObj();
       if (sketchobj) {
           makePlane(sketchobj->getPlane());
       }
       if (isSnapedSketchPos) {
           renderer->drawPoint2D({ onSketchPos.x,onSketchPos.y }, Eigen::Vector4<uint8_t>(255,0,255,0), 16, static_cast<MOON::Plane2D>(plane));
       }
       if (drawSketchPos) {
            auto drawList=ImGui::GetForegroundDrawList();
            Maths::FVector2 screenPos=m_sceneView->worldToScreen(getWorldPosFromSketchPos(drawPos));
            screenPos.y -= 7;
		    screenPos.x += 5;
		    std::string posText = "(" + std::to_string(drawPos.x) + "," + std::to_string(drawPos.y) + ")";
            drawList->AddText(nullptr,0,ImVec2(screenPos.x,screenPos.y), IM_COL32(0, 0, 0, 255),posText.c_str());
       }
       renderer->pushSize(3);
       for (int i = 0; i < lines.size();i += 2) {
           renderer->drawLine2D({ lines[i].x
               ,lines[i].y }, { lines[i + 1].x
               ,lines[i + 1].y }, static_cast<MOON::Plane2D>(plane));
       }
       renderer->popSize();
    }
    void DrawSketchHandler::onMouseMove()
    {
        //need to make sure which plane
        auto ray = m_sceneView->GetMouseRay();
        Maths::FVector3 out;
        ray.hitPlane(planeNormal, 0, out);
        if (plane == 2) {
            onSketchPos = Base::Vector2d(int(out.x*100)/100.0, int(out.y*100)/100.0);
        }
        else if (plane == 0) {
            onSketchPos = Base::Vector2d(int(out.y * 100) / 100.0, int(out.z * 100) / 100.0);
        }
        else {
            onSketchPos = Base::Vector2d(int(out.x * 100) / 100.0, int(out.z * 100) / 100.0);
        }
        auto sketchobj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
		isSnapedSketchPos = false;
        if (sketchobj) {
            isSnapedSketchPos=sketchobj->snapPoint(onSketchPos);
        }
    }
    void DrawSketchHandler::drawEdit(const std::vector<Base::Vector2d>& EditCurve)
    {
        for (int i = 0; i < EditCurve.size() - 1; i++) {
            lines.push_back(EditCurve[i]);
            lines.push_back(EditCurve[i + 1]);
        }
    }
    void DrawSketchHandler::drawEdit(const std::list<std::vector<Base::Vector2d>>& list) 
    {
        for (auto it = list.begin(); it != list.end(); it++) {
            auto& segment = *it;
            for (int i = 0; i < segment.size() - 1;i++) {
                lines.push_back(segment[i]);
                lines.push_back(segment[i+1]);
            } 
        }
    }

    void DrawSketchHandler::drawEdit(const std::vector<Part::Geometry*>& geometries)  {
        std::list<std::vector<Base::Vector2d>> list;
        for (const auto& geo : geometries) {
            if (geo->isDerivedFrom<Part::GeomCurve>()) {
                list.push_back(CurveConvert::toVector2D(geo,50));
            }
        }
        drawEdit(list);
    }
    void DrawSketchHandler::drawPositionAtCursor(Base::Vector2d pos)
    {
        drawSketchPos = true;
		drawPos = pos;  
    }
    void DrawSketchHandler::clearPositionAtCursor()
    {
        drawSketchPos = false;
    }
    void DrawSketchHandler::drawFloatValue(float value)
    {
        auto drawList = ImGui::GetForegroundDrawList();
        Maths::FVector2 screenPos = m_sceneView->worldToScreen(getWorldPosFromSketchPos(onSketchPos));
        screenPos.y -= 7;
        screenPos.x += 5;
        std::string posText = std::to_string(value);
        drawList->AddText(nullptr, 0, ImVec2(screenPos.x, screenPos.y), IM_COL32(0, 0, 0, 255), posText.c_str());
    }
    int DrawSketchHandler::getPreselectCurve() const
    {
        return 0;
    }
    Maths::FVector3 DrawSketchHandler::getWorldPosFromSketchPos(Base::Vector2d sketchPos)
    {
		Maths::FVector3 v;
        if (plane == 2) {
            v.x = static_cast<float>(sketchPos.x);
            v.y = static_cast<float>(sketchPos.y);
            v.z = 0;
        }
        else if (plane == 0) {
            v.x = 0;
            v.y = static_cast<float>(sketchPos.x);
            v.z = static_cast<float>(sketchPos.y);
        }
        else {
            v.x = static_cast<float>(sketchPos.x);
            v.y = 0;
            v.z = static_cast<float>(sketchPos.y);
        }
        return v;
    }
}