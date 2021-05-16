#ifndef LOGWORK_H
#define LOGWORK_H

#include <QObject>

#include <Windows.h>

class LogWork : public QObject
{
	Q_OBJECT

public:
	explicit LogWork(QObject *parent = nullptr);
	~LogWork();

	void terminal();

private:
	bool isTerminated;
	HANDLE m_hMailslot;

public slots:
	void work();

signals:
	void receiveLog(QString log);
};

#endif // LOGWORK_H
