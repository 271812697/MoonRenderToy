#pragma once
#include "editor/Command/command.h"

namespace MOON {

class  ExportFileCommand : public Command
{
	Q_OBJECT

public:
	ExportFileCommand(QObject* parent);
	virtual void execute()override;

};

}

