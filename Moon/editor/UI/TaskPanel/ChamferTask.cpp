#include "editor/UI/TaskPanel/ChamferTask.h"
#include "TaskBox.h"
#include "Widgets/SliderFloatProperty.h"
#include "Widgets/BoolProperty.h"
#include "Widgets/EnumProperty.h"
#include "TopoShape.h"
#include "core/ViewTool.h"
#include "feature/ChamferFeature.h"
#include "core/log.h"
#include "Interactive/Widgets/AxisTranslationWidget.h"
#include "base/BoundBox.h"
#include "App/GizmoHelper.h"

#include <QStringList>

namespace MOON {

	class ChamferTask::Internal {
	public:
		Internal(ChamferTask* s) : self(s) {
			auto f = self->getFeature();
			if (f) {
				feature = dynamic_cast<ChamferFeature*>(f);
			}
			else {
				Feature* baseFeature = nullptr;
				std::vector<std::string> subValues;
				ViewTool::getSelectedBasedFeature(baseFeature, subValues);
				if (baseFeature) {
					isCreatedFeature = true;
					feature = new ChamferFeature("Chamfer");
					feature->setBaseFeature(baseFeature);
					feature->setSubValues(subValues);
					self->setFeature(feature);

					Part::TopoShape baseShape = feature->getBaseTopoShape();
					std::vector<Part::TopoShape> shapes = feature->getBaseTopoEdgeShapes();
					feature->len = baseShape.getBoundBoxOptimal().CalcDiagonalLength() * 0.01;
					feature->size = baseShape.getBoundBoxOptimal().CalcDiagonalLength() * 0.03;

					// Attach the arrows to the first edge (one per adjacent face).
					Part::TopoShape edge = shapes[0];
					auto [face1, face2] = getAdjacentFacesFromEdge(edge, baseShape);
					DraggerPlacementProps props1 = getDraggerPlacementFromEdgeAndFace(edge, face1);
					DraggerPlacementProps props2 = getDraggerPlacementFromEdgeAndFace(edge, face2);
					feature->origin1[0] = props1.position.x;
					feature->origin1[1] = props1.position.y;
					feature->origin1[2] = props1.position.z;
					feature->dir1[0] = props1.dir.x;
					feature->dir1[1] = props1.dir.y;
					feature->dir1[2] = props1.dir.z;
					feature->origin2[0] = props2.position.x;
					feature->origin2[1] = props2.position.y;
					feature->origin2[2] = props2.position.z;
					feature->dir2[0] = props2.dir.x;
					feature->dir2[1] = props2.dir.y;
					feature->dir2[2] = props2.dir.z;
				}
			}
			if (feature) {
				axisBehaviour1 = new AxisTranslationWidget("chamfer");
				axisBehaviour2 = new AxisTranslationWidget("chamfer");
				axisBehaviour1->setUpScale(feature->len);
				axisBehaviour2->setUpScale(feature->len);
				axisBehaviour1->setLength(feature->size);
				axisBehaviour1->setUpOrigin(feature->origin1[0], feature->origin1[1], feature->origin1[2]);
				axisBehaviour1->setUpDir(feature->dir1[0], feature->dir1[1], feature->dir1[2]);
				axisBehaviour2->setLength(feature->size);
				axisBehaviour2->setUpOrigin(feature->origin2[0], feature->origin2[1], feature->origin2[2]);
				axisBehaviour2->setUpDir(feature->dir2[0], feature->dir2[1], feature->dir2[2]);
				axisBehaviour1->AddObserver(AxisTranslationEvent::LengthChange, self, &ChamferTask::onWidgetLengthInvoke1);
				axisBehaviour2->AddObserver(AxisTranslationEvent::LengthChange, self, &ChamferTask::onWidgetLengthInvoke2);
			}
		}
		~Internal() {
			if (axisBehaviour1) {
				delete axisBehaviour1;
				delete axisBehaviour2;
			}
		}
	private:
		SliderFloatProperty* sizeProp;
		friend ChamferTask;
		ChamferTask* self = nullptr;
		ChamferFeature* feature = nullptr;
		AxisTranslationWidget* axisBehaviour1 = nullptr;
		AxisTranslationWidget* axisBehaviour2 = nullptr;
		bool isCreatedFeature = false;
	};

	ChamferTask::ChamferTask(QWidget* parent, Feature* feature)
		: ParamTaskDialog(parent), ShapeHelper(feature), mInternal(new Internal(this)) {
		setGenerateShapeName("ChamferShape");
		mPreviewOption.isTransparent = false;
		mPreviewOption.isBlend = true;
		mPreviewOption.useDomainColor = false;
		PropertyComponent* p = addGroupParam("Chamfer");

		EnumProperty* type = new EnumProperty("Type", p);
		addParam(type);
		mInternal->sizeProp = new SliderFloatProperty("Size", p);
		mInternal->sizeProp->setMinMax(0.1f, 10.f);
		addParam(mInternal->sizeProp);
		SliderFloatProperty* size2 = new SliderFloatProperty("Size2", p);
		size2->setMinMax(0.1f, 10.f);
		addParam(size2);
		SliderFloatProperty* angle = new SliderFloatProperty("Angle", p);
		angle->setMinMax(1.f, 179.f);
		addParam(angle);
		addParam(new BoolProperty("Flip Direction", p));
		addParam(new BoolProperty("Use ALL Edges", p));
		buildUi();
	}

	ChamferTask::~ChamferTask() {
		delete mInternal;
	}

	QVariant ChamferTask::getParamValue(const QString& propertyName) {
		if (propertyName == "Chamfer:Type") {
			QList<QString> list = { "Equal distance", "Two distances", "Distance and Angle" };
			return QVariant::fromValue(list);
		}
		else if (propertyName == "Chamfer:Size") {
			return QVariant::fromValue(mInternal->feature->size);
		}
		else if (propertyName == "Chamfer:Size2") {
			return QVariant::fromValue(mInternal->feature->size2);
		}
		else if (propertyName == "Chamfer:Angle") {
			return QVariant::fromValue(mInternal->feature->angle);
		}
		else if (propertyName == "Chamfer:Flip Direction") {
			return QVariant::fromValue(mInternal->feature->flipDirection);
		}
		else if (propertyName == "Chamfer:Use ALL Edges") {
			return QVariant::fromValue(mInternal->feature->useAllEdges);
		}
		return QVariant();
	}

	void ChamferTask::setParamValue(const QString& propertyName, const QVariant& value) {
		bool updatePreView = false;
		if (propertyName == "Chamfer:Type") {
			mInternal->feature->chamferType = value.value<int>();
			updatePreView = true;
		}
		else if (propertyName == "Chamfer:Size") {
			mInternal->feature->size = value.toFloat();
			if (mInternal->axisBehaviour1) {
				mInternal->axisBehaviour1->setLength(mInternal->feature->size);
				mInternal->axisBehaviour2->setLength(mInternal->feature->size);
			}
			updatePreView = true;
		}
		else if (propertyName == "Chamfer:Size2") {
			mInternal->feature->size2 = value.toFloat();
			updatePreView = true;
		}
		else if (propertyName == "Chamfer:Angle") {
			mInternal->feature->angle = value.toFloat();
			updatePreView = true;
		}
		else if (propertyName == "Chamfer:Flip Direction") {
			mInternal->feature->flipDirection = value.value<bool>();
			updatePreView = true;
		}
		else if (propertyName == "Chamfer:Use ALL Edges") {
			mInternal->feature->useAllEdges = value.value<bool>();
			updatePreView = true;
		}
		if (updatePreView && mInternal->axisBehaviour1) {
			previewShape();
		}
	}

	void ChamferTask::clickOk() {
		generateFinalShape();
	}

	void ChamferTask::clickApply() {
	}

	void ChamferTask::clickCancel() {
		clearPreviewShape();
		if (mInternal->isCreatedFeature) {
			mInternal->feature->RemoveFromScene();
			delete mInternal->feature;
		}
	}

	void ChamferTask::onWidgetLengthInvoke1() {
		mInternal->feature->size = mInternal->axisBehaviour1->getLength();
		mInternal->sizeProp->updateWidgetValue(mInternal->feature->size);
	}

	void ChamferTask::onWidgetLengthInvoke2() {
		mInternal->feature->size = mInternal->axisBehaviour2->getLength();
		mInternal->sizeProp->updateWidgetValue(mInternal->feature->size);
	}

}
