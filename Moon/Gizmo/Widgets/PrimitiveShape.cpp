#include "Gizmo/Widgets/PrimitiveShape.h"
#include "Gizmo/Gizmo.h"
#include "Qtimgui/imgui/imgui.h"
#include "Gizmo/Interactive/Event.h"
#include "Gizmo/Interactive/ExecuteCommand.h"
#include "Gizmo/Interactive/WidgetCallbackMapper.h"
#include "Gizmo/Interactive/WidgetEvent.h"
#include "Gizmo/Interactive/WidgetEventTranslator.h"
#include "Gizmo/Interactive/RenderWindowInteractor.h"
#include "Gizmo/MathUtil/MathUtil.h"
#include "Gizmo/Interactive/CallbackCommand.h"
#include "renderer/SceneView.h"
#include <Core/ECS/Components/CModelRenderer.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include "core/component/CTopoShape.h"
#include "core/component/TopoShapeActor.h"
#include "TopoShape.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <Precision.hxx>

namespace MOON {
	
	PrimitiveShape::PrimitiveShape(const std::string& name) :GizmoWidget(name)
	{
		this->KeyEventCallbackCommand = CallbackCommand::New();
		this->KeyEventCallbackCommand->SetClientData(this);
		this->KeyEventCallbackCommand->SetCallback(PrimitiveShape::ProcessKeyEvents);
		setActive(false);
	}
	PrimitiveShape::~PrimitiveShape()
	{
	}
	void PrimitiveShape::createTopoShape()
	{
	}

	void PrimitiveShape::onKeyPress(const std::string& key)
	{
		if (key == "RETURN")
		{
			createTopoShape();
		}
	}
}