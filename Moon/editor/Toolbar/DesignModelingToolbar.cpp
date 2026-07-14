#include "editor/Toolbar/DesignModelingToolbar.h"
#include "editor/Command/command.h"
#include "editor/UI/TaskPanel/TaskViewWidget.h"
#include "editor/UI/TaskPanel/ExtrudeTaskDialog.h"
#include "editor/UI/TaskPanel/ThicknessTaskDialog.h"
#include "editor/UI/TaskPanel/FilletTask.h"
#include "editor/UI/TaskPanel/RevolutionTask.h"
#include "TopoShape.h"
#include "Core/Global/ServiceLocator.h"
#include "core/ViewTool.h"
#include "renderer/SceneView.h"
#include "renderer/GizmoRenderPass.h"
#include <QCoreApplication>
namespace MOON {

	BaseTaskDialog* createTaskDialog(const std::string name) {
		if (name == "Pad") {
			ExtrudeTaskDialog* extrude= new ExtrudeTaskDialog();
			///Feature* f=ViewTool::getSelectedFeature();;
			//extrude->setFeature(f);
			//extrude->setUp();
			return extrude;
		}
		if (name == "Thickness") {
			return new ThicknessTaskDialog();
		}
		if (name == "Fillet") {
			return new FilletTask();
		}
		if (name == "Pocket") {
			return new ExtrudeTaskDialog(nullptr, ExtrudeType::Subtractive);
		}
		if (name == "Revolve") {
			return new RevolutionTask(RevolutionType::ReAdditive,nullptr);
		}
		if (name == "Groove")
		{
			return new RevolutionTask(RevolutionType::ReSubtractive,nullptr);
		}
		return nullptr;
	}
	class DesignModelCommand : public Command
	{
	public:
		DesignModelCommand(QObject* parent,const std::string& name) :Command(parent),taskName(name) {
			auto action = new QAction(this);
			action->setCheckable(true);
			setAction(action);
		}
	protected:
		virtual void execute()override {
			auto& task = GetService(TaskViewWidget);
			if (!task.hasTask()) {
				task.setTaskDialog(createTaskDialog(taskName));
			}
		}
	private:
		std::string taskName = "";
	};
	class DesignModelingToolbar::DesignModelingToolbarInternal {
	public:

		DesignModelingToolbarInternal(DesignModelingToolbar* toolbar) :self(toolbar)
		{
			setup();
		}
		void setup() {
			padCommand = new DesignModelCommand(self, "Pad");
			padCommand->setIcon(":/widgets/icons/partdesign/PartDesign_Pad.svg");
			revolveCommand = new DesignModelCommand(self,"Revolve");
			revolveCommand->setIcon(":/widgets/icons/partdesign/PartDesign_Revolution.svg");
			thicknessCommand = new DesignModelCommand(self,"Thickness");
			thicknessCommand->setIcon(":/widgets/icons/partdesign/PartDesign_Thickness.svg");
			filletCommand = new DesignModelCommand(self,"Fillet");
			filletCommand->setIcon(":/widgets/icons/partdesign/PartDesign_Fillet.svg");
			pocketCommand = new DesignModelCommand(self,"Pocket");
			pocketCommand->setIcon(":/widgets/icons/partdesign/PartDesign_Pocket.svg");
			grooveCommand = new DesignModelCommand(self,"Groove");
			grooveCommand->setIcon(":/widgets/icons/partdesign/PartDesign_Groove.svg");
			self->addAction(padCommand->action());			
			self->addAction(revolveCommand->action());
			self->addAction(thicknessCommand->action());
			self->addAction(filletCommand->action());
			self->addAction(pocketCommand->action());
			self->addAction(grooveCommand->action());

			retranslateUi();
		}
		void retranslateUi() {
			padCommand->action()->setText(QCoreApplication::translate("DesignModelingToolbar", "Pad", nullptr));
			thicknessCommand->action()->setText(QCoreApplication::translate("DesignModelingToolbar", "Thickness", nullptr));
			filletCommand->action()->setText(QCoreApplication::translate("DesignModelingToolbar", "Fillet", nullptr));
			pocketCommand->action()->setText(QCoreApplication::translate("DesignModelingToolbar", "Pocket", nullptr));
			revolveCommand->action()->setText(QCoreApplication::translate("DesignModelingToolbar", "Revolve", nullptr));
			grooveCommand->action()->setText(QCoreApplication::translate("DesignModelingToolbar", "Groove", nullptr));
		}
	private:
		friend class DesignModelingToolbarToolbar;
		DesignModelingToolbar* self = nullptr;
		DesignModelCommand* padCommand = nullptr;
		DesignModelCommand* thicknessCommand = nullptr;
		DesignModelCommand* filletCommand = nullptr;
		DesignModelCommand* pocketCommand = nullptr;
		DesignModelCommand* revolveCommand = nullptr;
		DesignModelCommand* grooveCommand = nullptr;
	};

	DesignModelingToolbar::DesignModelingToolbar(const QString& title, QWidget* parent)
		:QToolBar(title, parent)
	{
		constructor();
	}
	DesignModelingToolbar::DesignModelingToolbar(QWidget* parentObject) :QToolBar(parentObject)
	{
		constructor();
	}
	DesignModelingToolbar::~DesignModelingToolbar()
	{
		if (mInternal) {
			delete mInternal;
			mInternal = nullptr;
		}
	}
	void DesignModelingToolbar::constructor()
	{
		mInternal = new DesignModelingToolbarInternal(this);
	}
}