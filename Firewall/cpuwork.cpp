#include "cpuwork.h"
#include <QDebug>

CpuWork::CpuWork(QObject* parent) : QObject(parent)
{
	isTerminated = false;
	GetSystemTimes(&lastIdleTime, &lastKernelTime, &lastUserTime);
}

void CpuWork::work()
{
	while (!isTerminated)
	{
		FILETIME idleTime, kernelTime, userTime;
		GetSystemTimes(&idleTime, &kernelTime, &userTime);

		double idle = 0, kernel = 0, user = 0;

		idle = (double)getTime(lastIdleTime, idleTime);
		kernel = (double)getTime(lastKernelTime, kernelTime);
		user = (double)getTime(lastUserTime, userTime);

		if (kernel + user == 0.0)
		{
			continue;
		}

		int usage = 0;
		usage = (int)(kernel + user - idle) / (kernel + user) * 100;

		lastIdleTime = idleTime;
		lastKernelTime = kernelTime;
		lastUserTime = userTime;

		if (isTerminated)
		{
			break;
		}
		emit workDown(usage);
		QThread::msleep(1000);
	}
}

ULONGLONG CpuWork::getTime(FILETIME time1, FILETIME time2)
{
	ULARGE_INTEGER temp1, temp2;
	temp1.HighPart = time1.dwHighDateTime;
	temp1.LowPart = time1.dwLowDateTime;
	temp2.HighPart = time2.dwHighDateTime;
	temp2.LowPart = time2.dwLowDateTime;

	return temp2.QuadPart - temp1.QuadPart;//需要取差值
}

void CpuWork::terminal()
{
	isTerminated = true;
}
