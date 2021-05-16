#include "memorywork.h"

MemoryWork::MemoryWork(QObject *parent) : QObject(parent)
{
	isTerminated = false;
}

void MemoryWork::work()
{
	while(!isTerminated)
	{
		MEMORYSTATUSEX ms;
		ms.dwLength = sizeof(ms);
		int ret = GlobalMemoryStatusEx(&ms);
		if(isTerminated)
		{
			break;
		}
		if(ret)
		{
			emit workDown((int)ms.dwMemoryLoad);
		}
		QThread::msleep(1000);
	}

}


void MemoryWork::terminal()
{
	isTerminated = true;
}
