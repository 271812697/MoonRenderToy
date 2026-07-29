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
			
			auto& gizmoPass = view.GetRenderer().GetPass<Editor::Rendering::GizmoRenderPass>("ImRenderer");
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
	std::vector<std::string> CreateCurveCommand::blackList = {
		"DrawSketchHandlerPoint",
		"DrawSketchHandlerLine",
		"DrawSketchHandlerLineSet",
		"DrawSketchHandlerCircle",
		"DrawSketchHandlerArc",
		"DrawSketchHandlerBSpline",
		"DrawSketchHandlerRectangle" ,
		"DrawSketchHandlerRotate",
		"DrawSketchHandlerSymmetry",
		"DrawSketchHandlerTrimming",
		"DrawSketchHandlerFillet"
	};
	std::unordered_map<std::string, CreateCurveCommand*> CreateCurveCommand::commandMap;
	class SketchToolbar::SketchToolbarInternal {
	public:

		SketchToolbarInternal(SketchToolbar* toolbar) :self(toolbar)
		{
			
		}
		void setup() {
			point = new CreateCurveCommand(self, "DrawSketchHandlerPoint");
			line =new CreateCurveCommand(self, "DrawSketchHandlerLine");
			lineSet = new CreateCurveCommand(self,"DrawSketchHandlerLineSet");
			circle = new CreateCurveCommand(self, "DrawSketchHandlerCircle");
			arc = new CreateCurveCommand(self, "DrawSketchHandlerArc");
			bspline = new CreateCurveCommand(self, "DrawSketchHandlerBSpline");
			rectangle = new CreateCurveCommand(self,"DrawSketchHandlerRectangle");
			trimming = new CreateCurveCommand(self, "DrawSketchHandlerTrimming");
			rotate = new CreateCurveCommand(self, "DrawSketchHandlerRotate");
			symmetry=new CreateCurveCommand(self, "DrawSketchHandlerSymmetry");
			fillet = new CreateCurveCommand(self, "DrawSketchHandlerFillet");
			point->setIcon(":/widgets/icons/Sketcher_CreatePoint.svg");
		    line->setIcon(":/widgets/icons/Sketcher_CreateLine.svg");
			lineSet->setIcon(":/widgets/icons/Sketcher_CreatePolyline.svg");
			circle->setIcon(":/widgets/icons/Sketcher_CreateCircle.svg");
			arc->setIcon(":/widgets/icons/Sketcher_CreateArc.svg");
			bspline->setIcon(":/widgets/icons/Sketcher_CreateBSpline.svg");
			rectangle->setIcon(":/widgets/icons/Sketcher_CreateRectangle_Constr.svg");
			trimming->setIcon(":/widgets/icons/Sketcher_Trimming.svg");
			rotate->setIcon(":/widgets/icons/Sketcher_Rotate.svg");
			symmetry->setIcon(":/widgets/icons/Sketcher_Symmetry.svg");
			fillet->setIcon(":/widgets/icons/Sketcher_CreateFillet.svg");
			self->addAction(point->action());
			self->addAction(line->action());
			self->addAction(lineSet->action());
			self->addAction(arc->action());
			self->addAction(bspline->action());
			self->addAction(circle->action());
			self->addAction(rectangle->action());
			self->addAction(trimming->action());
			self->addAction(rotate->action());
			self->addAction(symmetry->action());
			self->addAction(fillet->action());
			retranslateUi();
		}
		void retranslateUi() {
			point->action()->setText(QCoreApplication::translate("SketchToolbar", "Point", nullptr));
			line->action()->setText(QCoreApplication::translate("SketchToolbar", "Line", nullptr));
			lineSet->action()->setText(QCoreApplication::translate("SketchToolbar", "LineSet", nullptr));
			circle->action()->setText(QCoreApplication::translate("SketchToolbar", "Circle", nullptr));
			arc->action()->setText(QCoreApplication::translate("SketchToolbar", "Arc", nullptr));
			bspline->action()->setText(QCoreApplication::translate("SketchToolbar", "Bspline", nullptr));
			rectangle->action()->setText(QCoreApplication::translate("SketchToolbar", "Rectangle", nullptr));
			trimming->action()->setText(QCoreApplication::translate("SketchToolbar", "Trimming", nullptr));
			rotate->action()->setText(QCoreApplication::translate("SketchToolbar", "Rotate", nullptr));
			symmetry->action()->setText(QCoreApplication::translate("SketchToolbar", "Symmetry", nullptr));
			fillet->action()->setText(QCoreApplication::translate("SketchToolbar", "Fillet", nullptr));
		}
	private:
		friend class SketchToolbar;
		SketchToolbar* self = nullptr;
		CreateCurveCommand* point;
		CreateCurveCommand* line;
		CreateCurveCommand* lineSet;
		CreateCurveCommand* circle;
		CreateCurveCommand* rotate;
		CreateCurveCommand* arc;
		CreateCurveCommand* bspline;
		CreateCurveCommand* rectangle;
		CreateCurveCommand* trimming;
		CreateCurveCommand* symmetry;
		CreateCurveCommand* fillet;
		
	};

	SketchToolbar::SketchToolbar(const QString& title, QWidget* parent)
		:QToolBar(title, parent)
	{
		RegService(SketchToolbar,*this);
		constructor();
	}
	SketchToolbar::SketchToolbar(QWidget* parentObject) :QToolBar(parentObject)
	{
		RegService(SketchToolbar, *this);
		constructor();
	}
	SketchToolbar::~SketchToolbar()
	{
		if (mInternal) {
			delete mInternal;
			mInternal = nullptr;
		}
	}
	void SketchToolbar::disableAllHandlers()
	{
		auto& view = GetService(Editor::Panels::SceneView);

		auto& gizmoPass = view.GetRenderer().GetPass<Editor::Rendering::GizmoRenderPass>("ImRenderer");
	
		for (int i = 0; i < CreateCurveCommand::blackList.size(); i++) {
			
			gizmoPass.enableGizmoWidget(CreateCurveCommand::blackList[i], false);
			if (CreateCurveCommand::commandMap.find(CreateCurveCommand::blackList[i]) != CreateCurveCommand::commandMap.end()) {
				CreateCurveCommand::commandMap[CreateCurveCommand::blackList[i]]->action()->setChecked(false);
			}
		}
		
	}
	void SketchToolbar::constructor()
	{
		mInternal = new SketchToolbarInternal(this);
		mInternal->setup();
	}
}