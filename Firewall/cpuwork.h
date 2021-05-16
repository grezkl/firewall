#ifndef CPUWORK_H
#define CPUWORK_H

#include <QObject>
#include <QThread>
#include <windows.h>

class CpuWork : public QObject
{
	Q_OBJECT
public:
	explicit CpuWork(QObject *parent = nullptr);
	~CpuWork() {}

	void terminal();

private:
	bool isTerminated;
	FILETIME lastIdleTime, lastKernelTime, lastUserTime;

	ULONGLONG getTime(FILETIME time1, FILETIME time2);

public slots:
	void work();

signals:
	void workDown(int usage);
};

#endif // CPUWORK_H
