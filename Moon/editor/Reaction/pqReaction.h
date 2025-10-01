#pragma once
#include <QAction>
#include <QObject>


class pqReaction : public QObject
{
	Q_OBJECT
		typedef QObject Superclass;

public:
	/**
	 * Constructor. Parent cannot be nullptr.
	 */
	pqReaction(QAction* parent, Qt::ConnectionType type = Qt::AutoConnection);
	~pqReaction() override;

	/**
	 * Provides access to the parent action.
	 */
	QAction* parentAction() const { return qobject_cast<QAction*>(this->parent()); }

protected Q_SLOTS:
	/**
	 * Called when the action is triggered.
	 */
	virtual void onTriggered() {}

	virtual void updateEnableState() {}
	virtual void updateMasterEnableState(bool);

protected: // NOLINT(readability-redundant-access-specifiers)
	bool IsMaster;

private:
	Q_DISABLE_COPY(pqReaction)
};

