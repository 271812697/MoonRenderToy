#include "pqReaction.h"
#include <cassert>

//-----------------------------------------------------------------------------
pqReaction::pqReaction(QAction* parentObject, Qt::ConnectionType type)
	: Superclass(parentObject)
{
	assert(parentObject != nullptr);

	QObject::connect(parentObject, SIGNAL(triggered(bool)), this, SLOT(onTriggered()), type);



	this->IsMaster = true;
}

//-----------------------------------------------------------------------------
pqReaction::~pqReaction() = default;

//-----------------------------------------------------------------------------
void pqReaction::updateMasterEnableState(bool isMaster)
{
	this->IsMaster = isMaster;
	updateEnableState();
}
