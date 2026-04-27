#include "editor/Toolbar/sketchToolbar.h"
#include "editor/Command/command.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/SceneView.h"
#include "renderer/GizmoRenderPass.h"
#include <QCoreApplication>
namespace MOON {

	class  CreateCurveCommand : public Command
	{
	public:
		CreateCurveCommand(QObject* parent,const std::string& handler) :Command(parent),handlerName(handler) {
			auto action = new QAction(this);
			action->setCheckable(true);
			setAction(action);
			commandMap[handler] = this;
		}
		static std::vector<std::string> blackList; 
		static std::unordered_map<std::string, CreateCurveCommand*> commandMap;
	protected:
		virtual void execute()override {
			bool value = action()->isChecked();
			auto& view = GetService(Editor::Panels::SceneView);
			auto& gizmoPass = view.GetRenderer().GetPass<Editor::Rendering::GizmoRenderPass>("Gizmo");
			gizmoPass.enableGizmoWidget(handlerName, value);
			if (value) {
				for (int i = 0;i < blackList.size();i++) {
					if (blackList[i] != handlerName) {
						gizmoPass.enableGizmoWidget(blackList[i], false);
						if (commandMap.find(blackList[i]) != commandMap.end()) {
							commandMap[blackList[i]]->action()->setChecked(false);
						}
					}
				}
			}
		}
	private:
		std::string handlerName = "";
			 
	};
	std::vector<std::string> CreateCurveCommand::blackList = { "DrawSketchHandlerCircle","DrawSketchHandlerArc","DrawSketchHandlerBSpline","DrawSketchHandlerRectangle" };
	std::unordered_map<std::string, CreateCurveCommand*> CreateCurveCommand::commandMap;
	class SketchToolbar::SketchToolbarInternal {
	public:

		SketchToolbarInternal(SketchToolbar* toolbar) :self(toolbar)
		{
		}
		void setup() {
			circle = new CreateCurveCommand(self, "DrawSketchHandlerCircle");
			arc = new CreateCurveCommand(self, "DrawSketchHandlerArc");
			bspline = new CreateCurveCommand(self, "DrawSketchHandlerBSpline");
			rectangle = new CreateCurveCommand(self,"DrawSketchHandlerRectangle");
			circle->setIcon(":/widgets/icons/Sketcher_CreateCircle.svg");
			arc->setIcon(":/widgets/icons/Sketcher_CreateArc.svg");
			bspline->setIcon(":/widgets/icons/Sketcher_CreateBSpline.svg");
			rectangle->setIcon(":/widgets/icons/Sketcher_CreateRectangle_Constr.svg");
			self->addAction(arc->action());
			self->addAction(bspline->action());
			self->addAction(circle->action());
			self->addAction(rectangle->action());
			retranslateUi();
		}
		void retranslateUi() {
			circle->action()->setText(QCoreApplication::translate("SketchToolbar", "Circle", nullptr));
			arc->action()->setText(QCoreApplication::translate("SketchToolbar", "Arc", nullptr));
			bspline->action()->setText(QCoreApplication::translate("SketchToolbar", "Bspline", nullptr));
			rectangle->action()->setText(QCoreApplication::translate("SketchToolbar", "Rectangle", nullptr));
		}
	private:
		friend class SketchToolbar;
		SketchToolbar* self = nullptr;
		CreateCurveCommand* circle;
		CreateCurveCommand* arc;
		CreateCurveCommand* bspline;
		CreateCurveCommand* rectangle;
	};

	SketchToolbar::SketchToolbar(const QString& title, QWidget* parent)
		:QToolBar(title, parent)
	{
		constructor();
	}
	SketchToolbar::SketchToolbar(QWidget* parentObject) :QToolBar(parentObject)
	{
		constructor();
	}
	SketchToolbar::~SketchToolbar()
	{
		if (mInternal) {
			delete mInternal;
			mInternal = nullptr;
		}
	}
	void SketchToolbar::constructor()
	{
		mInternal = new SketchToolbarInternal(this);
		mInternal->setup();
		
	}
}