#include "editor/UI/TaskPanel/BaseTaskDialog.h"
namespace MOON {
	BaseTaskDialog::BaseTaskDialog(QWidget* parent) : QWidget(parent)
	{
		m_layout = new QVBoxLayout(this);
		m_layout->setContentsMargins(5, 5, 5, 5);
		m_layout->setSpacing(6);
	}
}