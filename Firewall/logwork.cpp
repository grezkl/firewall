#include "logwork.h"

LogWork::LogWork(QObject *parent)
	: QObject(parent)
{
	isTerminated = false;

	//生成一个最低权限的令牌，否则其他非管理员权限进程无法获取邮槽句柄
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = &sd;

	//创建邮槽，监听其他进程的拦截信息
	m_hMailslot = CreateMailslotA("\\\\.\\mailslot\\FirewallMailSlot", 0, MAILSLOT_WAIT_FOREVER, &sa);
}

LogWork::~LogWork()
{
	CloseHandle(m_hMailslot);
}

void LogWork::work()
{
	while (!isTerminated)
	{
		DWORD nextMsgSize = 0;
		DWORD msgCount = 0;

		GetMailslotInfo(m_hMailslot, NULL, &nextMsgSize, &msgCount, NULL);
		for (int i = 0; i < msgCount; i++) // 这个循环有点占用系统资源，但不用这个循环本线程会被挂起。。
		{
			if (isTerminated)
				break;
			DWORD dwRead;
			char buf[400] = { 0 };
			if (ReadFile(m_hMailslot, buf, 400, &dwRead, NULL))
			{
				emit receiveLog(QString::fromLocal8Bit(buf));
			}
		}
		/*
		DWORD dwRead;
		char buf[400] = { 0 };
		if (ReadFile(m_hMailslot, buf, 400, &dwRead, NULL))
		{
			emit receiveLog(QString::fromLocal8Bit(buf));
		}*/
	}
}

void LogWork::terminal()
{
	isTerminated = true;
}
