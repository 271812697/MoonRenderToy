#pragma once
#include "editor/UI/TaskPanel/ParamTaskDialog.h"
namespace MOON
{
	class Feature;
	class DatumLineFeature;
	class DatumLineTask : public ParamTaskDialog
	{
		Q_OBJECT
	public:
		explicit DatumLineTask(QWidget* parent = nullptr, Feature* feature = nullptr);
		virtual ~DatumLineTask() override;

		virtual QVariant getParamValue(const QString& propertyName) override;
		virtual void setParamValue(const QString& propertyName, const QVariant& value) override;
		virtual void clickOk() override;
		virtual void clickApply() override;
		virtual void clickCancel() override;

	private:
		void updatePreview();
		void clearPreview();
		class Internal;
		Internal* mInternal = nullptr;
	};
}
