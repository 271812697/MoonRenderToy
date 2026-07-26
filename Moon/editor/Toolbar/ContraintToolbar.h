#pragma once
#include <QToolBar>
namespace MOON {
	class ConstraintToolbar : public QToolBar
	{
		Q_OBJECT
	public:
		ConstraintToolbar(const QString& title, QWidget* parent = nullptr);
		ConstraintToolbar(QWidget* parentObject = nullptr);
		~ConstraintToolbar()override;
	private:
		class Internal;
		Internal* mInternal = nullptr;
	};
}