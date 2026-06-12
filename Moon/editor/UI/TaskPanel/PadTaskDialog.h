#pragma once
#include "editor/UI/TaskPanel/BaseTaskDialog.h"
namespace Part
{
	class TopoShape;
};
namespace MOON {
	class SketcherObj;
	class PadTaskDialog : public BaseTaskDialog
	{
		Q_OBJECT
	public:
		explicit PadTaskDialog(QWidget* parent = nullptr);
		~PadTaskDialog();
		void previewShape();
		void setBasedTopoShape( Part::TopoShape* topoShape);
		virtual void buildUi() override;
		virtual void clickOk() override;
		virtual void clickApply() override;
		virtual void clickCancel() override;
	private:
		class Internal;
		Internal* mInternal = nullptr;
	};
}