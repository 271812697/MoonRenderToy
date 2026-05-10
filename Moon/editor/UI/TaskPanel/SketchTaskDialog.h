#pragma once
#include "editor/UI/TaskPanel/BaseTaskDialog.h"
#include <QVBoxLayout>
namespace MOON {
	class SketcherObj;
	class SketchTaskDialog : public BaseTaskDialog
	{
		Q_OBJECT
	public:
		explicit SketchTaskDialog(QWidget* parent = nullptr);
		virtual void buildUi() override;
		QVBoxLayout* mainLayout() { return m_layout; }
	private:
		
		// 这里可以添加一些成员变量，比如输入框、按钮等
		SketcherObj* sketchObj = nullptr;
	};
}