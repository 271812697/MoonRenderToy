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
	};
	class CoincidentConstraint :public ConstraintCommand {
	public:
		CoincidentConstraint(QObject* parent):ConstraintCommand(parent) {
		}
	protected:
		virtual void execute()override {
			SketcherObj* Obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
			std::vector<SketcherObj::SelectGeoId> listOfGeoIds = Obj->getSelectGeoPosIds();

			if (listOfGeoIds.size() > 1) {
				if (listOfGeoIds.size() == 2) {
					Sketcher::ConstraintType constrType = Sketcher::ConstraintType::None;
					if (listOfGeoIds[0].pointPos != SketcherObj::PointPos::None && listOfGeoIds[1].pointPos != SketcherObj::PointPos::None) {
						constrType = Sketcher::ConstraintType::Coincident;
						Obj->addConstraint(
							constrType,
							listOfGeoIds[0].GeoId,
							convertPointPos(listOfGeoIds[0].pointPos),
							listOfGeoIds[1].GeoId,
							convertPointPos(listOfGeoIds[1].pointPos));
					}
					else if (listOfGeoIds[0].pointPos == SketcherObj::PointPos::None && listOfGeoIds[1].pointPos != SketcherObj::PointPos::None) {
						constrType = Sketcher::ConstraintType::PointOnObject;
						Obj->addConstraint(
							constrType,
							listOfGeoIds[1].GeoId,
							convertPointPos(listOfGeoIds[1].pointPos),
							listOfGeoIds[0].GeoId,
							convertPointPos(listOfGeoIds[0].pointPos));
					}
					else if (listOfGeoIds[0].pointPos != SketcherObj::PointPos::None && listOfGeoIds[1].pointPos == SketcherObj::PointPos::None) {
						constrType = Sketcher::ConstraintType::PointOnObject;
						Obj->addConstraint(
							constrType,
							listOfGeoIds[0].GeoId,
							convertPointPos(listOfGeoIds[0].pointPos),
							listOfGeoIds[1].GeoId,
							convertPointPos(listOfGeoIds[1].pointPos));

					}
					else {
						CORE_ERROR("both are edges");
					}
				}
				else {
					Obj->addConstraint(
						Sketcher::ConstraintType::Coincident,
						listOfGeoIds[0].GeoId,
						convertPointPos(listOfGeoIds[0].pointPos),
						listOfGeoIds[1].GeoId,
						convertPointPos(listOfGeoIds[1].pointPos),
						listOfGeoIds[2].GeoId,
						convertPointPos(listOfGeoIds[2].pointPos)
					);
				}

				Obj->solve();
			}
		}
	};
	class HorizontalConstraint :public ConstraintCommand {
	public:
		HorizontalConstraint(QObject* parent) :ConstraintCommand(parent) {
		}
	protected:
		virtual void execute()override {
			SketcherObj* Obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
			std::vector<SketcherObj::SelectGeoId> listOfGeoIds = Obj->getSelectGeoPosIds();

			if (listOfGeoIds.size() == 1) {
				Obj->addConstraint(
						Sketcher::ConstraintType::Horizontal,
						listOfGeoIds[0].GeoId, Sketcher::PointPos::none
						);
			
				Obj->solve();
			}
		}
	};
	class VerticalConstraint :public ConstraintCommand {
	public:
		VerticalConstraint(QObject* parent) :ConstraintCommand(parent) {
		}
	protected:
		virtual void execute()override {
			SketcherObj* Obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
			std::vector<SketcherObj::SelectGeoId> listOfGeoIds = Obj->getSelectGeoPosIds();
			if (listOfGeoIds.size() == 1) {
				Obj->addConstraint(
					Sketcher::ConstraintType::Vertical,
					listOfGeoIds[0].GeoId, Sketcher::PointPos::none
				);

				Obj->solve();
			}
		}
	};
	class ParallelConstraint :public ConstraintCommand {
	public:
		ParallelConstraint(QObject* parent) :ConstraintCommand(parent) {
		}
	protected:
		virtual void execute()override {
			SketcherObj* Obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
			std::vector<SketcherObj::SelectGeoId> listOfGeoIds = Obj->getSelectGeoPosIds();
			if (listOfGeoIds.size() == 2) {
				Obj->addConstraint(
					Sketcher::ConstraintType::Parallel,
					listOfGeoIds[0].GeoId, Sketcher::PointPos::none,
					listOfGeoIds[1].GeoId, Sketcher::PointPos::none
				);

				Obj->solve();
			}
		}
	};
	class PerpendicularConstraint :public ConstraintCommand {
	public:
		PerpendicularConstraint(QObject* parent) :ConstraintCommand(parent) {
		}
	protected:
		virtual void execute()override {
			SketcherObj* Obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
			std::vector<SketcherObj::SelectGeoId> listOfGeoIds = Obj->getSelectGeoPosIds();
			if (listOfGeoIds.size() == 2) {
				Obj->addConstraint(
					Sketcher::ConstraintType::Perpendicular,
					listOfGeoIds[0].GeoId, Sketcher::PointPos::none,
					listOfGeoIds[1].GeoId, Sketcher::PointPos::none
				);

				Obj->solve();
			}
		}
	};
	class ConstraintToolbar::Internal {
	public:

		Internal(ConstraintToolbar* toolbar) :self(toolbar)
		{
			setup();
			
		}
		void setup() {
			coincident = new CoincidentConstraint(self);
			coincident->setIcon(":/widgets/icons/constraint/Constraint_Coincident.svg");
			horizontal = new HorizontalConstraint(self);
			horizontal->setIcon(":/widgets/icons/constraint/Constraint_Horizontal.svg");
			vertical = new VerticalConstraint(self);
			vertical->setIcon(":/widgets/icons/constraint/Constraint_Vertical.svg");
			parallel = new ParallelConstraint(self);
			parallel->setIcon(":/widgets/icons/constraint/Constraint_Parallel.svg");
			perpendicular = new PerpendicularConstraint(self);
			perpendicular->setIcon(":/widgets/icons/constraint/Constraint_Perpendicular.svg");
			self->addAction(coincident->action());
			self->addAction(horizontal->action());
			self->addAction(vertical->action());
			self->addAction(parallel->action());
			self->addAction(perpendicular->action());
			retranslateUi();
		}
		void retranslateUi() {
			coincident->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Coincident", nullptr));
			horizontal->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Horizontal", nullptr));
			vertical->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Vertical", nullptr));
			parallel->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Parallel", nullptr));
			perpendicular->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Perpendicular", nullptr));
		}
	private:
		friend class ConstraintToolbar;
		ConstraintToolbar* self = nullptr;
		ConstraintCommand* coincident = nullptr;
		ConstraintCommand* horizontal = nullptr;
		ConstraintCommand* vertical = nullptr;
		ConstraintCommand* parallel = nullptr;
		ConstraintCommand* perpendicular = nullptr;
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