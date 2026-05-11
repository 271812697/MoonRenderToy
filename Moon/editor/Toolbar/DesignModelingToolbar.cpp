#include "editor/Toolbar/DesignModelingToolbar.h"
#include "editor/Command/command.h"
#include "editor/UI/TaskPanel/TaskViewWidget.h"
#include "editor/UI/TaskPanel/PadTaskDialog.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/SceneView.h"
#include "renderer/GizmoRenderPass.h"
#include <QCoreApplication>
namespace MOON {

	class DesignModelCommand : public Command
	{
	public:
		DesignModelCommand(QObject* parent,const std::string& handler) :Command(parent),handlerName(handler) {
			auto action = new QAction(this);
			action->setCheckable(true);
			setAction(action);
		}
	protected:
		virtual void execute()override {
			bool value = action()->isChecked();
			auto& task = GetService(TaskViewWidget);
			if (!task.hasTask()) {
				task.setTaskDialog(new PadTaskDialog());
			}
	
		}
	private:
		std::string handlerName = "";
			 
	};
	
	class DesignModelingToolbar::DesignModelingToolbarInternal {
	public:

		DesignModelingToolbarInternal(DesignModelingToolbar* toolbar) :self(toolbar)
		{
		}
		void setup() {
			padCommand = new DesignModelCommand(self, "DesignModelingToolbar_Pad");
			padCommand->setIcon(":/widgets/icons/partdesign/PartDesign_Pad.svg");
			self->addAction(padCommand->action());
			retranslateUi();
		}
		void retranslateUi() {
			padCommand->action()->setText(QCoreApplication::translate("DesignModelingToolbar", "Pad", nullptr));
		}
	private:
		friend class DesignModelingToolbarToolbar;
		DesignModelingToolbar* self = nullptr;
		DesignModelCommand* padCommand = nullptr;
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
		mInternal->setup();
	}
}