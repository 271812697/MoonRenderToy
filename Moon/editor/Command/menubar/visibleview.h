#pragma once
#include <QObject>
class QMenu;
namespace MOON {

class  VisibleViewCommand : public QObject
{
	Q_OBJECT

public:
	VisibleViewCommand(QObject* parent);
	void setUp(QMenu* menu);
};

}

