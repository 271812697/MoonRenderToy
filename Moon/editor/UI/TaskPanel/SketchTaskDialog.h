#pragma once
#include "editor/UI/TaskPanel/ParamTaskDialog.h"
#include "editor/UI/TaskPanel/ShapeHelper.h"
class QListWidget;
class QTimer;
namespace MOON {
	class SketchTaskDialog : public ParamTaskDialog, public ShapeHelper
	{
		Q_OBJECT
	public:
		explicit SketchTaskDialog(QWidget* parent = nullptr,Feature* feature =nullptr);
		~SketchTaskDialog();
		virtual QVariant getParamValue(const QString& propertyName)override;
		virtual void setParamValue(const QString& propertyName, const QVariant& value)override;
		virtual void clickOk() override;
		virtual void clickApply() override;
		virtual void clickCancel() override;
		void onSelectPlane();
		void refreshLists();
	private:
		QListWidget* mConstraintList = nullptr;
		QListWidget* mCurveList = nullptr;
		QTimer* mRefreshTimer = nullptr;
		QString mListCache;
		class Internal;
		Internal* mInternal = nullptr;
	};
}
