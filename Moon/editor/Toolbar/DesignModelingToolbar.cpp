#include "editor/Toolbar/DesignModelingToolbar.h"
#include "editor/Command/command.h"
#include "editor/UI/TaskPanel/TaskViewWidget.h"
#include "editor/UI/TaskPanel/PadTaskDialog.h"
#include "editor/UI/TaskPanel/ThicknessTaskDialog.h"
#include "TopoShape.h"
#include "Core/Global/ServiceLocator.h"
#include "core/ViewTool.h"
#include "renderer/SceneView.h"
#include "renderer/GizmoRenderPass.h"
#include <QCoreApplication>
namespace MOON {

	class PadCommand : public Command
	{
	public:
		PadCommand(QObject* parent,const std::string& handler) :Command(parent),handlerName(handler) {
			auto action = new QAction(this);
			action->setCheckable(true);
			setAction(action);
		}
	protected:
		virtual void execute()override {
			auto& task = GetService(TaskViewWidget);
			if (!task.hasTask()) {
				task.setTaskDialog(new PadTaskDialog());
			}
		}
	private:
		std::string handlerName = "";
	};
	class ThicknessCommand : public Command
	{
	public:
		ThicknessCommand(QObject* parent) :Command(parent){
			auto action = new QAction(this);
			action->setCheckable(true);
			setAction(action);
		}
	protected:
		virtual void execute()override {
			auto& task = GetService(TaskViewWidget);
			if (!task.hasTask()) {
				task.setTaskDialog(new ThicknessTaskDialog());
			}
		}
	private:
		std::string handlerName = "";
	};
	class DesignModelingToolbar::DesignModelingToolbarInternal {
	public:

		DesignModelingToolbarInternal(DesignModelingToolbar* toolbar) :self(toolbar)
		{
			setup();
		}
		void setup() {
			padCommand = new PadCommand(self, "DesignModelingToolbar_Pad");
			padCommand->setIcon(":/widgets/icons/partdesign/PartDesign_Pad.svg");
			thicknessCommand = new ThicknessCommand(self);
			thicknessCommand->setIcon(":/widgets/icons/partdesign/PartDesign_Thickness.svg");
			self->addAction(padCommand->action());
			self->addAction(thicknessCommand->action());
			retranslateUi();
		}
		void retranslateUi() {
			padCommand->action()->setText(QCoreApplication::translate("DesignModelingToolbar", "Pad", nullptr));
			thicknessCommand->action()->setText(QCoreApplication::translate("DesignModelingToolbar", "Thickness", nullptr));
		}
	private:
		friend class DesignModelingToolbarToolbar;
		DesignModelingToolbar* self = nullptr;
		PadCommand* padCommand = nullptr;
		ThicknessCommand* thicknessCommand = nullptr;
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