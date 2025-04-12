#pragma once
#include "pqReaction.h"
#include <QList>
#include <QStringList>

class  pqLoadDataReaction : public pqReaction
{
	Q_OBJECT
		typedef pqReaction Superclass;

public:

	pqLoadDataReaction(QAction* parent);

public Q_SLOTS: // NOLINT(readability-redundant-access-specifiers)

	void updateEnableState() override;

protected:

	void onTriggered() override;



private:
	Q_DISABLE_COPY(pqLoadDataReaction)
};

