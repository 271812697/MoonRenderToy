#pragma once
#include "pqReaction.h"
namespace MOON {
class  pqLoadDataReaction : public pqReaction
{
	Q_OBJECT
		typedef pqReaction Superclass;
public:
	pqLoadDataReaction(QAction* parent);
signals:
	void sceneChange(const QString&);
public Q_SLOTS: // NOLINT(readability-redundant-access-specifiers)
	void updateEnableState() override;
protected:
	void onTriggered() override;
private:
	Q_DISABLE_COPY(pqLoadDataReaction)
};

}

