#include "editor/Toolbar/ContraintToolbar.h"
#include "editor/Command/command.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/SceneView.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
#include "core/log.h"
#include <QCoreApplication>
namespace MOON {
	Sketcher::PointPos convertPointPos(SketcherObj::PointPos pos) {
	   if (pos == SketcherObj::PointPos::None) {
		  return Sketcher::PointPos::none;
	   }
	   if (pos == SketcherObj::PointPos::CenterP) {
		   return Sketcher::PointPos::mid;
	   }
	   if (pos == SketcherObj::PointPos::StartP) {
		   return Sketcher::PointPos::start;
	   }
	   if (pos == SketcherObj::PointPos::EndP) {
		   return Sketcher::PointPos::end;
	   }
	   return Sketcher::PointPos::none;
	}
	class  ConstraintCommand : public Command
	{
	public:
		ConstraintCommand(QObject* parent) :Command(parent) {
			auto action = new QAction(this);
			action->setCheckable(false);
			setAction(action);
			
		}
		
	protected:
		virtual void execute()override {
			bool value = action()->isChecked();
			auto& view = GetService(Editor::Panels::SceneView);
			SketcherObj* Obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
			std::vector<SketcherObj::SelectGeoId> listOfGeoIds = Obj->getSelectGeoPosIds();
			/*
					void addConstraint(
			Sketcher::ConstraintType constrType,
			int firstGeoId,
			Sketcher::PointPos firstPos,
			int secondGeoId = Sketcher::GeoEnum::GeoUndef,
			Sketcher::PointPos secondPos = Sketcher::PointPos::none,
			int thirdGeoId = Sketcher::GeoEnum::GeoUndef,
			Sketcher::PointPos thirdPos = Sketcher::PointPos::none
		);
			*/
			if (listOfGeoIds.size() > 1) {
				Obj->addConstraint(
					Sketcher::ConstraintType::Coincident, 
					listOfGeoIds[0].GeoId,
					convertPointPos(listOfGeoIds[0].pointPos),
					listOfGeoIds[1].GeoId,
					convertPointPos(listOfGeoIds[1].pointPos) );
				Obj->solve();
			}

			//for (auto  id : listOfGeoIds) {
			//	std::string pos = "None";
			//	if (id.pointPos == SketcherObj::PointPos::StartP) {
			//		pos = "Start";
			//	}
			//	if (id.pointPos == SketcherObj::PointPos::CenterP) {
			//		pos = "Center";
			//	}
			//	if (id.pointPos == SketcherObj::PointPos::EndP) {
			//		pos = "End";
			//	}
			//	CORE_INFO("Select {} {}",id.GeoId,pos);
			//}
			

		}
	};

	class ConstraintToolbar::Internal {
	public:

		Internal(ConstraintToolbar* toolbar) :self(toolbar)
		{
			setup();
			
		}
		void setup() {
			coincident = new ConstraintCommand(self);
			coincident->setIcon(":/widgets/icons/constraint/Constraint_Coincident.svg");
			self->addAction(coincident->action());
			retranslateUi();
		}
		void retranslateUi() {
			coincident->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Coincident", nullptr));
		}
	private:
		friend class ConstraintToolbar;
		ConstraintToolbar* self = nullptr;
		ConstraintCommand* coincident = nullptr;
	};

	ConstraintToolbar::ConstraintToolbar(const QString& title, QWidget* parent)
		:QToolBar(title, parent),mInternal(new Internal(this))
	{
		RegService(ConstraintToolbar,*this);
		
	}
	ConstraintToolbar::ConstraintToolbar(QWidget* parentObject) 
		:QToolBar(parentObject), mInternal(new Internal(this))
	{
		RegService(ConstraintToolbar, *this);
		
	}
	ConstraintToolbar::~ConstraintToolbar()
	{
		if (mInternal) {
			delete mInternal;
			mInternal = nullptr;
		}
	}
}