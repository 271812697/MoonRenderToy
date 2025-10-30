#pragma once
#include "editor/Command/command.h"

namespace MOON {

class  OpenFileCommand : public Command
{
	Q_OBJECT

public:
	OpenFileCommand(QObject* parent);
	virtual void execute()override;
signals:
	void sceneChange(const QString&);

};

}

