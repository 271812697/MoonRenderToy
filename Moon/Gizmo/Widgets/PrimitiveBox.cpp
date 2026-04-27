#include "Gizmo/Widgets/PrimitiveBox.h"
#include "Gizmo/Gizmo.h"
#include "Qtimgui/imgui/imgui.h"
#include "Gizmo/Interactive/Event.h"
#include "Gizmo/Interactive/ExecuteCommand.h"
#include "Gizmo/Interactive/WidgetCallbackMapper.h"
#include "Gizmo/Interactive/WidgetEvent.h"
#include "Gizmo/Interactive/WidgetEventTranslator.h"
#include "Gizmo/Interactive/RenderWindowInteractor.h"
#include "renderer/SceneView.h"
#include <Core/ECS/Components/CModelRenderer.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include "core/component/CTopoShape.h"

namespace MOON {
	
	PrimitiveBox::PrimitiveBox(const std::string& name) :GizmoWidget(name)
	{
		translation = { 0,0,0 };
		rot = Eigen::Matrix3f::Identity();
		scale = {1,1,1};
		// Define widget events
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonPressEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::Select, this, PrimitiveBox::MousePressed);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::RightButtonReleaseEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::EndSelect, this, PrimitiveBox::RightMouseReleased);
		this->CallbackMapper->SetCallbackMethod(ExecuteCommand::MouseMoveEvent, GizmoEvent::NoModifier, 0,
			0, 0, WidgetEvent::Move3D, this, PrimitiveBox::MouseMove);
		setActive(false);

	}
	PrimitiveBox::~PrimitiveBox()
	{
	}
	void PrimitiveBox::onUpdate()
	{
		renderer->boxEdit(renderer->makeId("Box"), translation, rot, scale);

	}
	void PrimitiveBox::onSetActive(bool flag)
	{
		
	}
	void PrimitiveBox::onMouseClicked()
	{
	
	}
	void PrimitiveBox::onMouseMove()
	{

	}

	void PrimitiveBox::onRightButtonRelease()
	{
		
		auto scene = m_sceneView->GetScene();
		auto& actor = scene->CreateActor("", "SketchGeomertyLine");
		auto& geoComp = actor.AddComponent<Core::ECS::Components::CTopoShape>();
		actor.AddComponent<Core::ECS::Components::CModelRenderer>();
		actor.AddComponent<Core::ECS::Components::CMaterialRenderer>();

	}

	void PrimitiveBox::SetEnabled(int v)
	{
		GizmoWidget::SetEnabled(v);
	}
	void PrimitiveBox::MousePressed(AbstractWidget* w)
	{
		PrimitiveBox* self = reinterpret_cast<PrimitiveBox*>(w);
		self->onMouseClicked();
	}

	void PrimitiveBox::RightMouseReleased(AbstractWidget*w)
	{
		PrimitiveBox* self = reinterpret_cast<PrimitiveBox*>(w);
		self->onRightButtonRelease();
	}
	
	void PrimitiveBox::MouseMove(AbstractWidget* w)
	{
		PrimitiveBox* self = reinterpret_cast<PrimitiveBox*>(w);
		self->onMouseMove();
	}
}