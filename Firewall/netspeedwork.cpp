#include "netspeedwork.h"

NetSpeedWork::NetSpeedWork(QObject* parent)
	: QObject(parent)
{
	isTerminated = false;
}

void NetSpeedWork::work()
{
	double lastIn = 0, lastOut = 0;

	while (!isTerminated)
	{
		PMIB_IF_TABLE2 m_pTable;
		PMIB_IF_ROW2 m_pRow;
		ULONGLONG pkgIn = 0, pkgOut = 0;

		if (NO_ERROR == GetIfTable2(&m_pTable))//获取本地端口信息
		{
			for (int i = 0; i < (int)m_pTable->NumEntries; i++) //遍历所有端口,获取经过的数据包数量
			{
				m_pRow = &m_pTable->Table[i];
				pkgIn += m_pRow->InOctets;
				pkgOut += m_pRow->OutOctets;
			}

			double downSp = 0, upSp = 0;

			upSp = (double)pkgOut - lastOut;
			downSp = (double)pkgIn - lastIn;

			upSp = upSp < 0 ? 0 : upSp / 1024.0 / 8; //1Byte = 8bits
			downSp = downSp < 0 ? 0 : downSp / 1024.0 / 8;

			lastIn = pkgIn;
			lastOut = pkgOut;

			if (isTerminated)
			{
				break;
			}
			emit workDown(upSp, downSp);
		}
		FreeMibTable(m_pTable);
		QThread::msleep(1000);//每秒获取一次，得到的包数量就是网速
	}

}

void NetSpeedWork::terminal()
{
	isTerminated = true;
}

