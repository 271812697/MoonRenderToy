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
		// Define widget events
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonPressEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::Select, this, PrimitiveShape::LeftMousePressed);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonReleaseEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::Select3D, this, PrimitiveShape::LeftMouseReleased);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::RightButtonReleaseEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::Completed, this, PrimitiveShape::RightMouseReleased);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::RightButtonPressEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::EndSelect, this, PrimitiveShape::RightMousePressed);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::MouseMoveEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::Move3D, this, PrimitiveShape::MouseMove);

		this->KeyEventCallbackCommand = CallbackCommand::New();
		this->KeyEventCallbackCommand->SetClientData(this);
		this->KeyEventCallbackCommand->SetCallback(PrimitiveShape::ProcessKeyEvents);
		setActive(false);
	}
	PrimitiveShape::~PrimitiveShape()
	{
	}

	void PrimitiveShape::onLeftMousePressed()
	{
	}

	void PrimitiveShape::onLeftMouseReleased()
	{
	}

	void PrimitiveShape::onRightMousePressed()
	{
	}

	void PrimitiveShape::onRightMouseReleased()
	{
		
	}

	void PrimitiveShape::onMouseMove()
	{
	}

	void PrimitiveShape::createTopoShape()
	{
	}

	void PrimitiveShape::SetEnabled(int enabling)
	{
		int enabled = this->Enabled;
		// We do this step first because it sets the CurrentRenderer
		GizmoWidget::SetEnabled(enabling);

		// We defer enabling the handles until the selection process begins
		if (enabling && !enabled)
		{
			this->Interactor->AddObserver(
				ExecuteCommand::KeyPressEvent, this->KeyEventCallbackCommand, this->Priority);
			this->Interactor->AddObserver(
				ExecuteCommand::KeyReleaseEvent, this->KeyEventCallbackCommand, this->Priority);	
		}
		else if (!enabling && enabled)
		{
			this->Interactor->RemoveObserver(this->KeyEventCallbackCommand);
		}
	}


	void PrimitiveShape::LeftMousePressed(AbstractWidget*w)
	{
		PrimitiveShape* self = reinterpret_cast<PrimitiveShape*>(w);
		self->onLeftMousePressed();
	}

	void PrimitiveShape::LeftMouseReleased(AbstractWidget*w)
	{
		PrimitiveShape* self = reinterpret_cast<PrimitiveShape*>(w);
		self->onLeftMouseReleased();
	}

	void PrimitiveShape::RightMouseReleased(AbstractWidget*w)
	{
		PrimitiveShape* self = reinterpret_cast<PrimitiveShape*>(w);
		self->onRightMouseReleased();
	}

	void PrimitiveShape::RightMousePressed(AbstractWidget*w)
	{
		PrimitiveShape* self = reinterpret_cast<PrimitiveShape*>(w);
		self->onRightMousePressed();
	}
	
	void PrimitiveShape::MouseMove(AbstractWidget* w)
	{
		PrimitiveShape* self = reinterpret_cast<PrimitiveShape*>(w);
		self->onMouseMove();
	}
	void PrimitiveShape::ProcessKeyEvents(GizmoObject*, unsigned long event, void* clientdata, void*)
	{
		PrimitiveShape* self = static_cast<PrimitiveShape*>(clientdata);
		char* cKeySym = self->Interactor->GetKeySym();
		std::string keySym = cKeySym != nullptr ? cKeySym : "";
		std::transform(keySym.begin(), keySym.end(), keySym.begin(), ::toupper);
		if (event == ExecuteCommand::KeyPressEvent)
		{
			if (keySym == "RETURN")
			{
				self->createTopoShape();
			}
		}
	}
}