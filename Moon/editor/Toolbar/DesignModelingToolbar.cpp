#include "editor/Toolbar/DesignModelingToolbar.h"
#include "editor/Command/command.h"
#include "editor/UI/TaskPanel/TaskViewWidget.h"
#include "editor/UI/TaskPanel/PadTaskDialog.h"
#include "editor/UI/TaskPanel/ThicknessTaskDialog.h"
#include "editor/UI/TaskPanel/FilletTask.h"
#include "TopoShape.h"
#include "Core/Global/ServiceLocator.h"
#include "core/ViewTool.h"
#include "renderer/SceneView.h"
#include "renderer/GizmoRenderPass.h"
#include <QCoreApplication>
namespace MOON {

	BaseTaskDialog* createTaskDialog(const std::string name) {
		if (name == "Pad") {
			return new PadTaskDialog();
		}
		if (name == "Thickness") {
			return new ThicknessTaskDialog();
		}
		if (name == "Fillet") {
			return new FilletTask();
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
			thicknessCommand = new DesignModelCommand(self,"Thickness");
			thicknessCommand->setIcon(":/widgets/icons/partdesign/PartDesign_Thickness.svg");
			filletCommand = new DesignModelCommand(self,"Fillet");
			filletCommand->setIcon(":/widgets/icons/partdesign/PartDesign_Fillet.svg");
			self->addAction(padCommand->action());
			self->addAction(thicknessCommand->action());
			self->addAction(filletCommand->action());
			retranslateUi();
		}
		void retranslateUi() {
			padCommand->action()->setText(QCoreApplication::translate("DesignModelingToolbar", "Pad", nullptr));
			thicknessCommand->action()->setText(QCoreApplication::translate("DesignModelingToolbar", "Thickness", nullptr));
			filletCommand->action()->setText(QCoreApplication::translate("DesignModelingToolbar", "Fillet", nullptr));
		}
	private:
		friend class DesignModelingToolbarToolbar;
		DesignModelingToolbar* self = nullptr;
		DesignModelCommand* padCommand = nullptr;
		DesignModelCommand* thicknessCommand = nullptr;
		DesignModelCommand* filletCommand = nullptr;
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