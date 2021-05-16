#include "logwork.h"

LogWork::LogWork(QObject *parent)
	: QObject(parent)
{
	isTerminated = false;

	//����һ�����Ȩ�޵����ƣ����������ǹ���ԱȨ�޽����޷���ȡ�ʲ۾��
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = &sd;

	//�����ʲۣ������������̵�������Ϣ
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
		for (int i = 0; i < msgCount; i++) // ���ѭ���е�ռ��ϵͳ��Դ�����������ѭ�����̻߳ᱻ���𡣡�
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
