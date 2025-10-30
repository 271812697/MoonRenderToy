#pragma once
#include "editor/Command/command.h"

namespace MOON {

class  CameraModeComand : public Command
{
	Q_OBJECT

public:
	CameraModeComand(QObject* parent);
	virtual void execute()override;
private:
	QAction* mActionOrtho = nullptr;
	QAction* mActionPersp = nullptr;


};

}

