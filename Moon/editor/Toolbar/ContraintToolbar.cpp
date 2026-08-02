#include "editor/Toolbar/ContraintToolbar.h"
#include "editor/Command/command.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/SceneView.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
#include "core/log.h"
#include <QCoreApplication>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFormLayout>
namespace MOON {
	class ParamDialog : public QDialog
	{
		
	public:
		struct ParamDef
		{
			std::string name;
			float range[2];
			int decimal = 3;
			float singleStep = 1.0f;
			float value;
			ParamDef(
				const std::string&n,float a,float b,
				float v,int dec=3,float step=1.0):name(n),decimal(dec),singleStep(step),value(v)
			{
				range[0] = a;
				range[1] = b;
			}
		};
		explicit ParamDialog(const std::string&name,QWidget* parent = nullptr) {
			setWindowTitle(name.c_str());
			setModal(true);
		}
		void setUp() {
			// 总布局
			QVBoxLayout* layMain = new QVBoxLayout(this);
			layMain->setContentsMargins(20, 20, 20, 20);
			layMain->setSpacing(12);
			// 表单布局（核心，多行参数）
			QFormLayout* m_formLayout = new QFormLayout();
			m_formLayout->setLabelAlignment(Qt::AlignRight);
			m_formLayout->setSpacing(10);
			layMain->addLayout(m_formLayout);
			for (auto it : paramList) {
				QLabel* lableTip = new QLabel(it.name.c_str());
				QDoubleSpinBox* widget = new QDoubleSpinBox();
				paramWidgetList[it.name] = widget;

				// 数值框参数（匹配CAD软件）
				widget->setRange(it.range[0], it.range[1]); // 最小0.001，避免0半径
				widget->setDecimals(it.decimal);           // 3位小数，CAD精度
				widget->setValue(it.value);
				widget->setSingleStep(it.singleStep);
				m_formLayout->addRow(lableTip, widget);
			}
			QPushButton* btnOk = new QPushButton("ok");
			QPushButton* btnCancel = new QPushButton("cancle");



			QHBoxLayout* layBtn = new QHBoxLayout();
			layBtn->addStretch();
			layBtn->addWidget(btnOk);
			layBtn->addWidget(btnCancel);

			layMain->addLayout(layBtn);
			connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);
			connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
		}
		void addParamDef(const ParamDef& def) {
			paramList.push_back(def);
		}
		float getParamValue(const std::string& name) {
			return paramWidgetList[name]->value();
		}
	private:
		std::vector<ParamDef>paramList;
		std::unordered_map<std::string, QDoubleSpinBox*>paramWidgetList;
		
	};
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
				// horizontal line
				Obj->addConstraint(
						Sketcher::ConstraintType::Horizontal,
						listOfGeoIds[0].GeoId, Sketcher::PointPos::none
						);
				Obj->solve();
			}
			else if (listOfGeoIds.size() == 2) {
				// two points on the same horizontal line
				Obj->addConstraint(
					Sketcher::ConstraintType::Horizontal,
					listOfGeoIds[0].GeoId, convertPointPos(listOfGeoIds[0].pointPos),
					listOfGeoIds[1].GeoId, convertPointPos(listOfGeoIds[1].pointPos)
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
				// vertical line
				Obj->addConstraint(
					Sketcher::ConstraintType::Vertical,
					listOfGeoIds[0].GeoId, Sketcher::PointPos::none
				);
				Obj->solve();
			}
			else if (listOfGeoIds.size() == 2) {
				// two points on the same vertical line
				Obj->addConstraint(
					Sketcher::ConstraintType::Vertical,
					listOfGeoIds[0].GeoId, convertPointPos(listOfGeoIds[0].pointPos),
					listOfGeoIds[1].GeoId, convertPointPos(listOfGeoIds[1].pointPos)
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
	class TangentConstraint :public ConstraintCommand {
	public:
		TangentConstraint(QObject* parent) :ConstraintCommand(parent) {
		}
	protected:
		virtual void execute()override {
			SketcherObj* Obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
			std::vector<SketcherObj::SelectGeoId> listOfGeoIds = Obj->getSelectGeoPosIds();
			if (listOfGeoIds.size() == 2) {
				//we need to make a oritention
				Obj->addConstraint(
					Sketcher::ConstraintType::Tangent,
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
				//line and line
				Obj->addConstraint(
					Sketcher::ConstraintType::Perpendicular,
					listOfGeoIds[0].GeoId, Sketcher::PointPos::none,
					listOfGeoIds[1].GeoId, Sketcher::PointPos::none
				);

				Obj->solve();
			}
			else if (listOfGeoIds.size() == 3) {
				if (listOfGeoIds[2].pointPos == SketcherObj::PointPos::None) {
					//point and point and line
					Obj->addConstraint(
						Sketcher::ConstraintType::Perpendicular,
						listOfGeoIds[0].GeoId, convertPointPos(listOfGeoIds[0].pointPos),
						listOfGeoIds[1].GeoId, convertPointPos(listOfGeoIds[1].pointPos),
						listOfGeoIds[2].GeoId
					);
				}
				else
				{
					//this case need to figure out
					//line and line
					//Obj->addConstraint(
					//	Sketcher::ConstraintType::Perpendicular,
					//	listOfGeoIds[0].GeoId, convertPointPos(listOfGeoIds[0].pointPos),
					//	listOfGeoIds[1].GeoId, convertPointPos(listOfGeoIds[1].pointPos),
					//	listOfGeoIds[2].GeoId, convertPointPos(listOfGeoIds[2].pointPos)
					//);
				}

				Obj->solve();
			}
		}
	};
	class DistanceXConstraint :public ConstraintCommand {
	public:
		DistanceXConstraint(QObject* parent) :ConstraintCommand(parent) {
		}
	protected:
		virtual void execute()override {
			SketcherObj* Obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
			std::vector<SketcherObj::SelectGeoId> listOfGeoIds = Obj->getSelectGeoPosIds();

			int selectNum = listOfGeoIds.size();
			if (selectNum>0) {
				ParamDialog dialog("DX");

				dialog.addParamDef(ParamDialog::ParamDef("DistanceX", 0, 100, 5));
				dialog.setUp();
				int ret = dialog.exec();
				if (ret == QDialog::Accepted)
				{
					double distance = dialog.getParamValue("DistanceX");
					CORE_INFO("distanceX is {}", distance);
					if (selectNum == 1) {
						if (listOfGeoIds[0].pointPos == SketcherObj::PointPos::None) {
							//horizontal length
							auto newConstr = std::make_unique<Sketcher::Constraint>();
							newConstr->Type = Sketcher::ConstraintType::DistanceX;
							newConstr->First = listOfGeoIds[0].GeoId;
							newConstr->setValue(distance);
							Obj->addConstraint(std::move(newConstr));
							Obj->solve();
						}
						else
						{
							// point on fixed x-coordinate
							auto newConstr = std::make_unique<Sketcher::Constraint>();
							newConstr->Type = Sketcher::ConstraintType::DistanceX;
							newConstr->First = listOfGeoIds[0].GeoId;
							newConstr->FirstPos = convertPointPos(listOfGeoIds[0].pointPos);
							newConstr->setValue(distance);
							Obj->addConstraint(std::move(newConstr));
							Obj->solve();
						}
					}
					else if (selectNum == 2) {
						// point to point horizontal distance
						auto newConstr = std::make_unique<Sketcher::Constraint>();
						newConstr->Type = Sketcher::ConstraintType::DistanceX;
						newConstr->First = listOfGeoIds[0].GeoId;
						newConstr->FirstPos = convertPointPos(listOfGeoIds[0].pointPos);
						newConstr->Second = listOfGeoIds[1].GeoId;
						newConstr->SecondPos = convertPointPos(listOfGeoIds[1].pointPos);
						newConstr->setValue(distance);
						Obj->addConstraint(std::move(newConstr));
						Obj->solve();
					}
				}
			}
		}
	};
	class DistanceYConstraint :public ConstraintCommand {
	public:
		DistanceYConstraint(QObject* parent) :ConstraintCommand(parent) {
		}
	protected:
		virtual void execute()override {
			SketcherObj* Obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
			std::vector<SketcherObj::SelectGeoId> listOfGeoIds = Obj->getSelectGeoPosIds();

			int selectNum = listOfGeoIds.size();
			if (selectNum > 0) {
				ParamDialog dialog("DY");

				dialog.addParamDef(ParamDialog::ParamDef("DistanceY", 0, 100, 5));
				dialog.setUp();
				int ret = dialog.exec();
				if (ret == QDialog::Accepted)
				{
					double distance = dialog.getParamValue("DistanceY");
					CORE_INFO("distanceY is {}", distance);
					if (selectNum == 1) {
						if (listOfGeoIds[0].pointPos == SketcherObj::PointPos::None) {
							//horizontal length
							auto newConstr = std::make_unique<Sketcher::Constraint>();
							newConstr->Type = Sketcher::ConstraintType::DistanceY;
							newConstr->First = listOfGeoIds[0].GeoId;
							newConstr->setValue(distance);
							Obj->addConstraint(std::move(newConstr));
							Obj->solve();
						}
						else
						{
							// point on fixed x-coordinate
							auto newConstr = std::make_unique<Sketcher::Constraint>();
							newConstr->Type = Sketcher::ConstraintType::DistanceY;
							newConstr->First = listOfGeoIds[0].GeoId;
							newConstr->FirstPos = convertPointPos(listOfGeoIds[0].pointPos);
							newConstr->setValue(distance);
							Obj->addConstraint(std::move(newConstr));
							Obj->solve();
						}
					}
					else if (selectNum == 2) {
						// point to point horizontal distance
						auto newConstr = std::make_unique<Sketcher::Constraint>();
						newConstr->Type = Sketcher::ConstraintType::DistanceY;
						newConstr->First = listOfGeoIds[0].GeoId;
						newConstr->FirstPos = convertPointPos(listOfGeoIds[0].pointPos);
						newConstr->Second = listOfGeoIds[1].GeoId;
						newConstr->SecondPos = convertPointPos(listOfGeoIds[1].pointPos);
						newConstr->setValue(distance);
						Obj->addConstraint(std::move(newConstr));
						Obj->solve();
					}
				}
			}
		}
	};
	class EqualConstraint :public ConstraintCommand {
	public:
		EqualConstraint(QObject* parent) :ConstraintCommand(parent) {
		}
	protected:
		virtual void execute()override {
			SketcherObj* Obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
			std::vector<SketcherObj::SelectGeoId> listOfGeoIds = Obj->getSelectGeoPosIds();

			int selectNum = listOfGeoIds.size();
			if (selectNum == 2) {
				if (listOfGeoIds[0].pointPos == SketcherObj::PointPos::None&& listOfGeoIds[1].pointPos == SketcherObj::PointPos::None) {
					Obj->addConstraint(
						Sketcher::ConstraintType::Equal,
						listOfGeoIds[0].GeoId, Sketcher::PointPos::none,
						listOfGeoIds[1].GeoId, Sketcher::PointPos::none
					);
					Obj->solve();
				}
			}
		}
	};
	class SymmetricConstraint :public ConstraintCommand {
	public:
		SymmetricConstraint(QObject* parent) :ConstraintCommand(parent) {
		}
	protected:
		virtual void execute()override {
			SketcherObj* Obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
			std::vector<SketcherObj::SelectGeoId> listOfGeoIds = Obj->getSelectGeoPosIds();

			int selectNum = listOfGeoIds.size();
			if (selectNum == 3) {
				if (listOfGeoIds[2].pointPos == SketcherObj::PointPos::None) {
					Obj->addConstraint(
						Sketcher::ConstraintType::Symmetric,
						listOfGeoIds[0].GeoId, convertPointPos(listOfGeoIds[0].pointPos),
						listOfGeoIds[1].GeoId, convertPointPos(listOfGeoIds[1].pointPos),
						listOfGeoIds[2].GeoId
					);
					Obj->solve();
				}
				else
				{
					Obj->addConstraint(
						Sketcher::ConstraintType::Symmetric,
						listOfGeoIds[0].GeoId, convertPointPos(listOfGeoIds[0].pointPos),
						listOfGeoIds[1].GeoId, convertPointPos(listOfGeoIds[1].pointPos),
						listOfGeoIds[2].GeoId, convertPointPos(listOfGeoIds[2].pointPos)
					);
					Obj->solve();
				}
			}
		}
	};
	class LengthConstraint :public ConstraintCommand {
	public:
		LengthConstraint(QObject* parent) :ConstraintCommand(parent) {
		}
	protected:
		virtual void execute()override {
			SketcherObj* Obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
			std::vector<SketcherObj::SelectGeoId> listOfGeoIds = Obj->getSelectGeoPosIds();
			int selectNum = listOfGeoIds.size();
			if (selectNum > 0) {
				ParamDialog dialog("Distance");
				dialog.addParamDef(ParamDialog::ParamDef("Dist", 0, 1000, 5));
				dialog.setUp();
				int ret = dialog.exec();
				if (ret == QDialog::Accepted)
				{
					double distance = dialog.getParamValue("Dist");
					CORE_INFO("distance is {}", distance);
					if (selectNum == 1) {
						if (listOfGeoIds[0].pointPos == SketcherObj::PointPos::None) {
							//line or arc length
							auto newConstr = std::make_unique<Sketcher::Constraint>();
							newConstr->Type = Sketcher::ConstraintType::Distance;
							newConstr->First = listOfGeoIds[0].GeoId;
							newConstr->setValue(distance);
							Obj->addConstraint(std::move(newConstr));
							Obj->solve();
						}
					}
					else if (selectNum == 2) {
						// point to line horizontal distance. it needs to make sure Orientation
						if (listOfGeoIds[0].pointPos != SketcherObj::PointPos::None&&
							listOfGeoIds[1].pointPos == SketcherObj::PointPos::None
							) {
							auto newConstr = std::make_unique<Sketcher::Constraint>();
							newConstr->Type = Sketcher::ConstraintType::Distance;
							newConstr->First = listOfGeoIds[0].GeoId;
							newConstr->FirstPos = convertPointPos(listOfGeoIds[0].pointPos);
							newConstr->Second = listOfGeoIds[1].GeoId;
							newConstr->setValue(distance);
							Obj->addConstraint(std::move(newConstr));
							Obj->solve();
						}
						else if 
							(listOfGeoIds[0].pointPos == SketcherObj::PointPos::None &&
							listOfGeoIds[1].pointPos == SketcherObj::PointPos::None
							) {
							// circle to circle, circle to
				            // arc, etc.
							auto newConstr = std::make_unique<Sketcher::Constraint>();
							newConstr->Type = Sketcher::ConstraintType::Distance;
							newConstr->First = listOfGeoIds[0].GeoId;
							newConstr->Second = listOfGeoIds[1].GeoId;
							newConstr->setValue(distance);
							Obj->addConstraint(std::move(newConstr));
							Obj->solve();
						}
						else if
							(listOfGeoIds[0].pointPos != SketcherObj::PointPos::None &&
								listOfGeoIds[1].pointPos != SketcherObj::PointPos::None
								) {
							// point to point distance
							auto newConstr = std::make_unique<Sketcher::Constraint>();
							newConstr->Type = Sketcher::ConstraintType::Distance;
							newConstr->First = listOfGeoIds[0].GeoId;
							newConstr->FirstPos = convertPointPos(listOfGeoIds[0].pointPos);
							newConstr->Second = listOfGeoIds[1].GeoId;
							newConstr->SecondPos = convertPointPos(listOfGeoIds[1].pointPos);
							newConstr->setValue(distance);
							Obj->addConstraint(std::move(newConstr));
							Obj->solve();
						}
					}
				}
			}
		}
	};
	class RadiusConstraint :public ConstraintCommand {
	public:
		RadiusConstraint(QObject* parent) :ConstraintCommand(parent) {
		}
	protected:
		virtual void execute()override {
			SketcherObj* Obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
			std::vector<SketcherObj::SelectGeoId> listOfGeoIds = Obj->getSelectGeoPosIds();

			int selectNum = listOfGeoIds.size();
			if (selectNum > 0) {
				ParamDialog dialog("Radius");

				dialog.addParamDef(ParamDialog::ParamDef("Radius", 0, 1000, 5));
				dialog.setUp();
				int ret = dialog.exec();
				if (ret == QDialog::Accepted)
				{
					double distance = dialog.getParamValue("Radius");
					CORE_INFO("Radius is {}", distance);
					if (selectNum == 1) {
						if (listOfGeoIds[0].pointPos == SketcherObj::PointPos::None) {
							//horizontal length
							auto newConstr = std::make_unique<Sketcher::Constraint>();
							newConstr->Type = Sketcher::ConstraintType::Radius;
							newConstr->First = listOfGeoIds[0].GeoId;
							newConstr->setValue(distance);
							Obj->addConstraint(std::move(newConstr));
							Obj->solve();
						}
					}
				}
			}
		}
	};
	class DiameterConstraint :public ConstraintCommand {
	public:
		DiameterConstraint(QObject* parent) :ConstraintCommand(parent) {
		}
	protected:
		virtual void execute()override {
			SketcherObj* Obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
			std::vector<SketcherObj::SelectGeoId> listOfGeoIds = Obj->getSelectGeoPosIds();

			int selectNum = listOfGeoIds.size();
			if (selectNum > 0) {
				ParamDialog dialog("Diameter");

				dialog.addParamDef(ParamDialog::ParamDef("Diameter", 0, 1000, 5));
				dialog.setUp();
				int ret = dialog.exec();
				if (ret == QDialog::Accepted)
				{
					double distance = dialog.getParamValue("Diameter");
					CORE_INFO("Diameter is {}", distance);
					if (selectNum == 1) {
						if (listOfGeoIds[0].pointPos == SketcherObj::PointPos::None) {
							//horizontal length
							auto newConstr = std::make_unique<Sketcher::Constraint>();
							newConstr->Type = Sketcher::ConstraintType::Diameter;
							newConstr->First = listOfGeoIds[0].GeoId;
							newConstr->setValue(distance);
							Obj->addConstraint(std::move(newConstr));
							Obj->solve();
						}
					}
				}
			}
		}
	};
	class BlockConstraint :public ConstraintCommand {
	public:
		BlockConstraint(QObject* parent) :ConstraintCommand(parent) {
		}
	protected:
		virtual void execute()override {
			SketcherObj* Obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
			std::vector<SketcherObj::SelectGeoId> listOfGeoIds = Obj->getSelectGeoPosIds();
			for (int i = 0; i < listOfGeoIds.size(); i++) {
				if (listOfGeoIds[i].pointPos == SketcherObj::None) {
					Obj->addConstraint(
						Sketcher::ConstraintType::Block,
						listOfGeoIds[i].GeoId, Sketcher::PointPos::none
					);
					Obj->solve();
				}
			}
		}
	};
	class AngleConstraint :public ConstraintCommand {
	public:
		AngleConstraint(QObject* parent) :ConstraintCommand(parent) {
		}
	protected:
		virtual void execute()override {
			SketcherObj* Obj = SketcherObjManager::instance().GetCurrentActiveSketcherObj();
			std::vector<SketcherObj::SelectGeoId> listOfGeoIds = Obj->getSelectGeoPosIds();

			int selectNum = listOfGeoIds.size();
			if (selectNum > 0) {
				ParamDialog dialog("Angle");
				dialog.addParamDef(ParamDialog::ParamDef("Angle", 0, 360, 90));
				dialog.setUp();
				int ret = dialog.exec();
				if (ret == QDialog::Accepted)
				{
					double distance = dialog.getParamValue("Angle");
					
					CORE_INFO("angle is {}", distance);
					distance = distance * 3.14159265358979323846f / 180.0f;
					if (selectNum == 3) {
						if (
							listOfGeoIds[0].pointPos != SketcherObj::PointPos::None
							&& listOfGeoIds[1].pointPos != SketcherObj::PointPos::None
							&& listOfGeoIds[2].pointPos != SketcherObj::PointPos::None
							) {
						    
							//auto newConstr = std::make_unique<Sketcher::Constraint>();
							//newConstr->Type = Sketcher::ConstraintType::Angle;
							//newConstr->First = listOfGeoIds[0].GeoId;
							//newConstr->FirstPos = convertPointPos(listOfGeoIds[0].pointPos);
							//newConstr->Second = listOfGeoIds[1].GeoId;
							//newConstr->SecondPos = convertPointPos(listOfGeoIds[1].pointPos);
							//newConstr->Third = listOfGeoIds[2].GeoId;
							//newConstr->ThirdPos = convertPointPos(listOfGeoIds[2].pointPos);
							//newConstr->setValue(distance);
							//Obj->addConstraint(std::move(newConstr));
							//Obj->solve();
						}
					}
					else if (selectNum == 2) {
						// line to line
						if (
							listOfGeoIds[0].pointPos != SketcherObj::PointPos::None
							&& listOfGeoIds[1].pointPos != SketcherObj::PointPos::None
							) {

							auto newConstr = std::make_unique<Sketcher::Constraint>();
							newConstr->Type = Sketcher::ConstraintType::Angle;
							newConstr->First = listOfGeoIds[0].GeoId;
							newConstr->FirstPos = convertPointPos(listOfGeoIds[0].pointPos);
							newConstr->Second = listOfGeoIds[1].GeoId;
							newConstr->SecondPos = convertPointPos(listOfGeoIds[1].pointPos);
							newConstr->setValue(distance);
							Obj->addConstraint(std::move(newConstr));
							Obj->solve();
						}
						else if (listOfGeoIds[0].pointPos == SketcherObj::PointPos::None
							&& listOfGeoIds[1].pointPos == SketcherObj::PointPos::None) {
							auto newConstr = std::make_unique<Sketcher::Constraint>();
							newConstr->Type = Sketcher::ConstraintType::Angle;
							newConstr->First = listOfGeoIds[0].GeoId;
							newConstr->Second = listOfGeoIds[1].GeoId;
							newConstr->setValue(distance);
							Obj->addConstraint(std::move(newConstr));
							Obj->solve();
						}
					}
					else if (selectNum == 1) {
						if (
							listOfGeoIds[0].pointPos == SketcherObj::PointPos::None
							) {
							auto newConstr = std::make_unique<Sketcher::Constraint>();
							newConstr->Type = Sketcher::ConstraintType::Angle;
							newConstr->First = listOfGeoIds[0].GeoId;
							newConstr->setValue(distance);
							Obj->addConstraint(std::move(newConstr));
							Obj->solve();
						}
					}
				}
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
			distanceX = new DistanceXConstraint(self);
			distanceX->setIcon(":/widgets/icons/constraint/Constraint_DistanceX.svg");
			distanceY = new DistanceYConstraint(self);
			distanceY->setIcon(":/widgets/icons/constraint/Constraint_VerticalDistance.svg");
			tangent = new TangentConstraint(self);
			tangent->setIcon(":/widgets/icons/constraint/Constraint_Tangent.svg");
			equal = new EqualConstraint(self);
			equal->setIcon(":/widgets/icons/constraint/Constraint_EqualLength.svg");
			symmetric = new SymmetricConstraint(self);
			symmetric->setIcon(":/widgets/icons/constraint/Constraint_Symmetric.svg");
			length = new LengthConstraint(self);
			length->setIcon(":/widgets/icons/constraint/Constraint_Length.svg");
			radius = new RadiusConstraint(self);
			radius->setIcon(":/widgets/icons/constraint/Constraint_Radius.svg");
			diameter = new DiameterConstraint(self);
			diameter->setIcon(":/widgets/icons/constraint/Constraint_Diameter.svg");
			block = new BlockConstraint(self);
			block->setIcon(":/widgets/icons/constraint/Constraint_Block.svg");
			angle= new AngleConstraint(self);
			angle->setIcon(":/widgets/icons/constraint/Constraint_InternalAngle.svg");
			self->addAction(coincident->action());
			self->addAction(horizontal->action());
			self->addAction(vertical->action());
			self->addAction(parallel->action());
			self->addAction(perpendicular->action());
			self->addAction(tangent->action());
			self->addAction(distanceX->action());
			self->addAction(distanceY->action());
			self->addAction(equal->action());
			self->addAction(symmetric->action());
			self->addAction(length->action());
			self->addAction(radius->action());
			self->addAction(diameter->action());
			self->addAction(block->action());
			self->addAction(angle->action());
			retranslateUi();
		}
		void retranslateUi() {
			coincident->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Coincident", nullptr));
			horizontal->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Horizontal", nullptr));
			vertical->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Vertical", nullptr));
			parallel->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Parallel", nullptr));
			perpendicular->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Perpendicular", nullptr));
			distanceX->action()->setText(QCoreApplication::translate("ConstraintToolbar", "DistanceX", nullptr));
			distanceY->action()->setText(QCoreApplication::translate("ConstraintToolbar", "DistanceY", nullptr));
			tangent->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Tangent", nullptr));
			equal->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Equal", nullptr));
			symmetric->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Symmetric", nullptr));
			length->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Length", nullptr));
			radius->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Radius", nullptr));
			diameter->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Diameter", nullptr));
			block->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Block", nullptr));
			angle->action()->setText(QCoreApplication::translate("ConstraintToolbar", "Angle", nullptr));
		}
	private:
		friend class ConstraintToolbar;
		ConstraintToolbar* self = nullptr;
		ConstraintCommand* coincident = nullptr;
		ConstraintCommand* horizontal = nullptr;
		ConstraintCommand* vertical = nullptr;
		ConstraintCommand* parallel = nullptr;
		ConstraintCommand* perpendicular = nullptr;
		ConstraintCommand* distanceX = nullptr;
		ConstraintCommand* distanceY = nullptr;
		ConstraintCommand* tangent = nullptr;
		ConstraintCommand* equal = nullptr;
		ConstraintCommand* symmetric = nullptr;
		ConstraintCommand* length = nullptr;
		ConstraintCommand* radius = nullptr;
		ConstraintCommand* diameter = nullptr;
		ConstraintCommand* block = nullptr;
		ConstraintCommand* angle = nullptr;
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