#include "editor/Toolbar/primitiveToolbar.h"
#include "editor/Command/command.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/SceneView.h"
#include "renderer/GizmoRenderPass.h"
#include <QCoreApplication>
namespace MOON {

	class  CreatePrimitiveCommand : public Command
	{
	public:
		CreatePrimitiveCommand(QObject* parent,const std::string& handler) :Command(parent),handlerName(handler) {
			auto action = new QAction(this);
			action->setCheckable(true);
			setAction(action);
		}
	protected:
		virtual void execute()override {
			bool value = action()->isChecked();
			auto& view = GetService(Editor::Panels::SceneView);
			auto& gizmoPass = view.GetRenderer().GetPass<Editor::Rendering::GizmoRenderPass>("Gizmo");
			gizmoPass.enableGizmoWidget(handlerName, value);
			//if (value) {
			//	for (int i = 0;i < blackList.size();i++) {
			//		if (blackList[i] != handlerName) {
			//			gizmoPass.enableGizmoWidget(blackList[i], false);
			//			if (commandMap.find(blackList[i]) != commandMap.end()) {
			//				commandMap[blackList[i]]->action()->setChecked(false);
			//			}
			//		}
			//	}
			//}
		}
	private:
		std::string handlerName = "";
			 
	};
	
	class PrimitiveToolbar::PrimitiveToolbarInternal {
	public:

		PrimitiveToolbarInternal(PrimitiveToolbar* toolbar) :self(toolbar)
		{
		}
		void setup() {
			box = new CreatePrimitiveCommand(self, "PrimitiveBox");
			box->setIcon(":/widgets/icons/parametric/Part_Box_Parametric.svg");
			self->addAction(box->action());
			cone = new CreatePrimitiveCommand(self, "PrimitiveCone");
			cone->setIcon(":/widgets/icons/parametric/Part_Cone_Parametric.svg");
			self->addAction(cone->action());
			cylinder = new CreatePrimitiveCommand(self, "PrimitiveCylinder");
			cylinder->setIcon(":/widgets/icons/parametric/Part_Cylinder_Parametric.svg");
			self->addAction(cylinder->action());
			sphere = new CreatePrimitiveCommand(self, "PrimitiveSphere");
			sphere->setIcon(":/widgets/icons/parametric/Part_Sphere_Parametric.svg");
			self->addAction(sphere->action());
			retranslateUi();
		}
		void retranslateUi() {
			box->action()->setText(QCoreApplication::translate("PrimitiveToolbar", "Box", nullptr));
			sphere->action()->setText(QCoreApplication::translate("PrimitiveToolbar", "Sphere", nullptr));
			cylinder->action()->setText(QCoreApplication::translate("PrimitiveToolbar", "Cylinder", nullptr));
			cone->action()->setText(QCoreApplication::translate("PrimitiveToolbar", "Cone", nullptr));
		}
	private:
		friend class PrimitiveToolbar;
		PrimitiveToolbar* self = nullptr;
		CreatePrimitiveCommand* box;
		CreatePrimitiveCommand* cone;
		CreatePrimitiveCommand* cylinder;
		CreatePrimitiveCommand* sphere;

	};

	PrimitiveToolbar::PrimitiveToolbar(const QString& title, QWidget* parent)
		:QToolBar(title, parent)
	{
		constructor();
	}
	PrimitiveToolbar::PrimitiveToolbar(QWidget* parentObject) :QToolBar(parentObject)
	{
		constructor();
	}
	PrimitiveToolbar::~PrimitiveToolbar()
	{
		if (mInternal) {
			delete mInternal;
			mInternal = nullptr;
		}
	}
	void PrimitiveToolbar::constructor()
	{
		mInternal = new PrimitiveToolbarInternal(this);
		mInternal->setup();
	}
}