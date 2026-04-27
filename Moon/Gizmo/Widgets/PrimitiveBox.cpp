#include "Gizmo/Widgets/PrimitiveBox.h"
#include "Gizmo/Gizmo.h"
#include "Qtimgui/imgui/imgui.h"
#include "Gizmo/Interactive/Event.h"
#include "Gizmo/Interactive/ExecuteCommand.h"
#include "Gizmo/Interactive/WidgetCallbackMapper.h"
#include "Gizmo/Interactive/WidgetEvent.h"
#include "Gizmo/Interactive/WidgetEventTranslator.h"
#include "Gizmo/Interactive/RenderWindowInteractor.h"
#include "Gizmo/MathUtil/MathUtil.h"
#include "renderer/SceneView.h"
#include <Core/ECS/Components/CModelRenderer.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include "core/component/CTopoShape.h"
#include "core/component/TopoShapeActor.h"
#include "TopoShape.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <Precision.hxx>

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

		BRepPrimAPI_MakeBox mkBox(2*scale.x(), 2*scale.y(), 2*scale.z());
	
		TopoDS_Shape ResultShape = mkBox.Shape();
	
		auto topoActor = new Core::ECS::TopoActor(scene, "TopoShapeBox", "TopoShape", false);
		const auto& topoComp = topoActor->GetComponent<Core::ECS::Components::CTopoShape>();
		Part::TopoShape& topo = topoComp->GetTopoShape();

		topo.setShape(ResultShape,false);
		Eigen::Matrix4f mat1 = Coord3(-scale, Eigen::Matrix3f::Identity(), {1,1,1});
		Eigen::Matrix4f mat2 = Coord3({0,0,0}, rot, {1,1,1});
		Eigen::Matrix4f mat3 = Coord3(translation, Eigen::Matrix3f::Identity(), { 1,1,1 });
		Eigen::Matrix4f mat =  mat3 * mat2 *mat1;
		Base::Matrix4D mm(
			mat(0,0), mat(0, 1),mat(0, 2), mat(0, 3),
			mat(1, 0), mat(1, 1), mat(1, 2), mat(1, 3),
			mat(2, 0), mat(2, 1), mat(2, 2), mat(2, 3),
			mat(3, 0), mat(3, 1), mat(3, 2), mat(3, 3)
		);
		
		topo.setTransform(mm);
		topoComp->discretizationShape();
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