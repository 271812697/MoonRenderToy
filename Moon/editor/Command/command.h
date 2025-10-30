#pragma once
#include <QAction>
#include <QObject>
namespace MOON
{
	class Command : public QObject
	{
		Q_OBJECT
	public:
		Command(QObject* parent);
		virtual ~Command()=default ;
		virtual void execute() = 0;
		QAction* action() const { return mAction; }
		void setIcon(const QString& path,bool on=true);
		virtual bool getEnabledStatus() const { return true; }
		void setAction(QAction* action);
	private:
		QAction* mAction = nullptr;
	};
}


