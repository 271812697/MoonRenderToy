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