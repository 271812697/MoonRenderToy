#include "fpsStat.h"

#include "Settings/DebugSetting.h"
#include "core/log.h"

namespace MOON {
	//-----------------------------------------------------------------------------
	FpsStatCommand::FpsStatCommand(QObject* parentObject)
		: Command(parentObject)
	{
		auto* fpsAction = new QAction(Command::tr("Show FPS"), this);
		fpsAction->setCheckable(true);
		fpsAction->setChecked(
			MOON::DebugSettings::instance().getOrDefault<bool>("showFPS", false));
		fpsAction->setObjectName(QString::fromUtf8("actionFpsStat"));
		fpsAction->setStatusTip("Toggle the FPS statistics overlay");
		setAction(fpsAction);
	}

	void FpsStatCommand::execute()
	{
		const bool show = action()->isChecked();
		MOON::DebugSettings::instance().setData("showFPS", show);
		CORE_INFO("FPS statistics {}", show ? "enabled" : "disabled");
	}
}
