#pragma once
#include "DrawSketchHandler.h"
#include "Gizmo/Gizmo.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
#include "Qtimgui/imgui/imgui.h"
#include "renderer/SceneView.h"
namespace MOON
{
    static std::vector<Base::Vector2d> toVector2D(const Part::Geometry* geometry)
    {
        int curvedEdgeCountSegments = 50;
        std::vector<Base::Vector2d> vector2d;
        auto emplaceasvector2d = [&vector2d](const Base::Vector3d& point) {
            vector2d.emplace_back(point.x, point.y);
            };
        auto isperiodicconic = geometry->is<Part::GeomCircle>() || geometry->is<Part::GeomEllipse>();
        auto isbounded = geometry->isDerivedFrom<Part::GeomBoundedCurve>();

        if (geometry->is<Part::GeomLineSegment>()) 
        {  // add a line
            auto geo = static_cast<const Part::GeomLineSegment*>(geometry);
            emplaceasvector2d(geo->getStartPoint());
            emplaceasvector2d(geo->getEndPoint());
        }
        else if (isperiodicconic || isbounded) 
        {
            auto geo = static_cast<const Part::GeomConic*>(geometry);
            double segment = (geo->getLastParameter() - geo->getFirstParameter())
                / curvedEdgeCountSegments;
            for (int i = 0; i < curvedEdgeCountSegments; i++) {
                emplaceasvector2d(geo->value(geo->getFirstParameter() + i * segment));
            }
            // either close the curve for untrimmed conic or set the last point for bounded curves
            emplaceasvector2d(isperiodicconic ? geo->value(0) : geo->value(geo->getLastParameter()));
        }
        return vector2d;
    }
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
       if (drawSketchPos) {
            auto drawList=ImGui::GetForegroundDrawList();
            Maths::FVector2 screenPos=m_sceneView->worldToScreen(getWorldPosFromSketchPos(onSketchPos));
            screenPos.y -= 7;
		    screenPos.x += 5;
		    std::string posText = "(" + std::to_string(onSketchPos.x) + "," + std::to_string(onSketchPos.y) + ")";
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
            list.push_back(toVector2D(geo));
        }
        drawEdit(list);
    }
    void DrawSketchHandler::drawPositionAtCursor(Base::Vector2d pos)
    {
        drawSketchPos = true;
		onSketchPos = pos;  
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