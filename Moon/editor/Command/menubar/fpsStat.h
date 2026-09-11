#pragma once
#include "editor/Command/command.h"

namespace MOON {

class  FpsStatCommand : public Command
{
	Q_OBJECT

public:
	FpsStatCommand(QObject* parent);
	virtual void execute()override;

};

}
