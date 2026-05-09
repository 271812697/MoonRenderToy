#pragma once
#include "editor/Command/command.h"

namespace MOON {

class  SketchCommand : public Command
{
	Q_OBJECT

public:
	SketchCommand(QObject* parent);
	virtual void execute()override;

};

}

