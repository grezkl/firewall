#include "infodlg.h"
#include "ui_infodlg.h"

InfoDlg::InfoDlg(int style, QWidget* parent) :
	QWidget(parent),
	ui(new Ui::InfoDlg), m_style(style)
{
	ui->setupUi(this);

	//����ͼƬ
	QPixmap pixmap;
	if (m_style == 0)
	{
		pixmap.load(":/images/speedmonitor.png");
		ui->label_9->setPixmap(pixmap);
		pixmap.load(":/images/CPU.png");
		ui->label_7->setPixmap(pixmap);
		pixmap.load(":/images/memory.png");
		ui->label_8->setPixmap(pixmap);
	}
	else
	{
		pixmap.load(":/images/speedmonitor_b.png");
		ui->label_9->setPixmap(pixmap);
		pixmap.load(":/images/CPU_b.png");
		ui->label_7->setPixmap(pixmap);
		pixmap.load(":/images/memory_b.png");
		ui->label_8->setPixmap(pixmap);
	}

	//����qss
	QString styleQss;
	if (m_style == 0)
		styleQss = ":/qss/DarkSkin_infoDlg.qss";
	else if (m_style == 1)
		styleQss = ":/qss/BrightSkin_infoDlg.qss";

	QFile qssFile(styleQss);
	if (qssFile.open(QFile::ReadOnly))
		this->setStyleSheet(qssFile.readAll());
	qssFile.close();

	firewall_status = firewallStatus();//��ȡ����ǽ״̬
	WritePrivateProfileStringA("Firewall", "Status", firewall_status ? "1" : "0", "C:\\Windows\\FW_rule.ini");

	if (firewall_status)
	{
		QPixmap pixmap_btn;
		if (m_style == 0)
			pixmap_btn.load(":/images/button2.png");
		else
			pixmap_btn.load(":/images/button2_b.png");
		QIcon icon_btn(pixmap_btn);
		ui->startBtn->setIcon(icon_btn);

		QPixmap pixmap_logo(":/images/firewall_on.png");
		ui->label_6->setPixmap(pixmap_logo);
	}
	else
	{
		QPixmap pixmap_btn;
		if (m_style == 0)
			pixmap_btn.load(":/images/buttonX2.png");
		else
			pixmap_btn.load(":/images/buttonX2_b.png");
		QIcon icon_btn(pixmap_btn);
		ui->startBtn->setIcon(icon_btn);

		QPixmap pixmap_logo(":/images/firewall_off.png");
		ui->label_6->setPixmap(pixmap_logo);
	}

	QString strIp = getLocalIP();
	ui->label_IP->setText(strIp);

	netspeed_thread = new QThread(this);
	netSpeedWork = new NetSpeedWork();//����ָ��������
	netSpeedWork->moveToThread(netspeed_thread);//�������������ƽ����µ��߳�

	memory_thread = new QThread(this);
	memoryWork = new MemoryWork();
	memoryWork->moveToThread(memory_thread);

	cpu_thread = new QThread(this);
	cpuWork = new CpuWork();
	cpuWork->moveToThread(cpu_thread);

	//�߳�����ʱ��������Ҳ���ٲ��ͷ��ڴ�ռ�
	connect(netspeed_thread, &QThread::finished, netSpeedWork, &QObject::deleteLater);
	connect(memory_thread, &QThread::finished, memoryWork, &QObject::deleteLater);
	connect(cpu_thread, &QThread::finished, cpuWork, &QObject::deleteLater);

	//���������ź�
	connect(netSpeedWork, &NetSpeedWork::workDown, this, &InfoDlg::updateNetspeed);
	//�����ڴ�ռ���ź�
	connect(memoryWork, &MemoryWork::workDown, this, &InfoDlg::updateMemoryUsage);
	//����cpuռ���ź�
	connect(cpuWork, &CpuWork::workDown, this, &InfoDlg::updateCpuUsage);

	//��ʼ�����ź�
	connect(this, &InfoDlg::startThread, netSpeedWork, &NetSpeedWork::work);
	connect(this, &InfoDlg::startThread, memoryWork, &MemoryWork::work);
	connect(this, &InfoDlg::startThread, cpuWork, &CpuWork::work);

	if (!netspeed_thread->isRunning())
	{
		netspeed_thread->start();
	}
	if (!memory_thread->isRunning())
	{
		memory_thread->start();
	}
	if (!cpu_thread->isRunning())
	{
		cpu_thread->start();
	}

	emit startThread();

}

InfoDlg::~InfoDlg()
{
	netSpeedWork->terminal();
	netspeed_thread->quit();
	netspeed_thread->wait();

	memoryWork->terminal();
	memory_thread->quit();
	memory_thread->wait();

	cpuWork->terminal();
	cpu_thread->quit();
	cpu_thread->wait();

	//����QObject::deleteLater֮���ڴ��Ѿ��ͷţ�ָ��Ҫ�ÿ�
	netSpeedWork = nullptr;
	memoryWork = nullptr;
	cpuWork = nullptr;

	delete ui;
}

void InfoDlg::on_startBtn_clicked()
{
	if (!firewall_status)
	{
		if (install_firewall())//��װ����ǽ
		{
			QPixmap pixmap_btn;
			if (m_style == 0)
				pixmap_btn.load(":/images/button2.png");
			else
				pixmap_btn.load(":/images/button2_b.png");
			QIcon icon_btn(pixmap_btn);
			ui->startBtn->setIcon(icon_btn);

			QPixmap pixmap_logo(":/images/firewall_on.png");
			ui->label_6->setPixmap(pixmap_logo);

			firewall_status = true;

			//������־
			SYSTEMTIME sys;
			GetLocalTime(&sys);
			char time[50] = { 0 };
			sprintf_s(time, 50, "%4d/%02d/%02d %02d:%02d:%02d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);

			WritePrivateProfileStringA("Firewall", "Status", "1", "C:\\Windows\\FW_rule.ini");
			emit updateSelfLog(QString("[" + QString(time) + "]" + "\n" + QString::fromLocal8Bit("����ǽ������") + "\n"));
			emit updateFWStatus(true);
		}
	}
	else
	{
		if (uninstall_firewall())//ж�ط���ǽ
		{
			QPixmap pixmap_btn;
			if (m_style == 0)
				pixmap_btn.load(":/images/buttonX2.png");
			else
				pixmap_btn.load(":/images/buttonX2_b.png");
			QIcon icon_btn(pixmap_btn);
			ui->startBtn->setIcon(icon_btn);

			QPixmap pixmap_logo(":/images/firewall_off.png");
			ui->label_6->setPixmap(pixmap_logo);

			firewall_status = false;

			//������־
			SYSTEMTIME sys;
			GetLocalTime(&sys);
			char time[50] = { 0 };
			sprintf_s(time, 50, "%4d/%02d/%02d %02d:%02d:%02d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);

			WritePrivateProfileStringA("Firewall", "Status", "0", "C:\\Windows\\FW_rule.ini");
			emit updateSelfLog(QString("[" + QString(time) + "]" + "\n" + QString::fromLocal8Bit("����ǽ�رգ�") + "\n"));
			emit updateFWStatus(false);
		}
	}
}

void InfoDlg::updateNetspeed(double up, double down)
{
	QString upSpeed, downSpeed;

	//up *= 1.1;
	//down *= 1.1;

	if (up >= 1024.0)
	{
		upSpeed = QString::number(up / 1024.0, 'f', 2) + "M/s";
	}
	else
	{
		upSpeed = QString::number(up, 'f', 2) + "K/s";
	}

	if (down >= 1024.0)
	{
		downSpeed = QString::number(down / 1024.0, 'f', 2) + "M/s";
	}
	else
	{
		downSpeed = QString::number(down, 'f', 2) + "K/s";
	}

	ui->label_upSpeed->setText(upSpeed);
	ui->label_downSpeed->setText(downSpeed);
}

void InfoDlg::updateMemoryUsage(int usage)
{
	if (usage < 0 || usage > 100)
	{
		return;
	}
	QString qstrUsage = QString::number(usage) + "%";
	ui->label_memory->setText(qstrUsage);
}

void InfoDlg::updateCpuUsage(int usage)
{
	if (usage < 0 || usage > 100)
	{
		return;
	}
	if(usage<70)
		usage = (usage * 1.46) > 100 ? 100 : (usage * 1.46); //У׼
	QString qstrUsage = QString::number(usage) + "%";
	ui->label_cpu->setText(qstrUsage);
}

void InfoDlg::changeTheme(int style)
{
	//����ͼƬ
	QPixmap pixmap;
	if (style == 0)
	{
		pixmap.load(":/images/speedmonitor.png");
		ui->label_9->setPixmap(pixmap);
		pixmap.load(":/images/CPU.png");
		ui->label_7->setPixmap(pixmap);
		pixmap.load(":/images/memory.png");
		ui->label_8->setPixmap(pixmap);
		if (firewallStatus())
		{
			pixmap.load(":/images/button2.png");
			ui->startBtn->setIcon(QIcon(pixmap));
		}
		else
		{
			pixmap.load(":/images/buttonX2.png");
			ui->startBtn->setIcon(QIcon(pixmap));
		}
	}
	else
	{
		pixmap.load(":/images/speedmonitor_b.png");
		ui->label_9->setPixmap(pixmap);
		pixmap.load(":/images/CPU_b.png");
		ui->label_7->setPixmap(pixmap);
		pixmap.load(":/images/memory_b.png");
		ui->label_8->setPixmap(pixmap);
		if (firewallStatus())
		{
			pixmap.load(":/images/button2_b.png");
			ui->startBtn->setIcon(QIcon(pixmap));
		}
		else
		{
			pixmap.load(":/images/buttonX2_b.png");
			ui->startBtn->setIcon(QIcon(pixmap));
		}
	}

	//����css
	QString styleQss;
	if (style == 0)
		styleQss = ":/qss/DarkSkin_infoDlg.qss";
	else if (style == 1)
		styleQss = ":/qss/BrightSkin_infoDlg.qss";

	QFile qssFile(styleQss);
	if (qssFile.open(QFile::ReadOnly))
		this->setStyleSheet(qssFile.readAll());
	qssFile.close();

	m_style = style;
}

bool InfoDlg::install_firewall()
{
	//����LSP
	SHELLEXECUTEINFOA info = { 0 };
	info.cbSize = sizeof(SHELLEXECUTEINFOA);
	info.fMask = SEE_MASK_NOCLOSEPROCESS;
	info.hwnd = NULL;
	info.lpVerb = "runas";
	info.lpFile = "cmd.exe";
	info.lpParameters = "/c netsh winsock reset";
	info.lpDirectory = NULL;
	info.nShow = SW_HIDE;
	info.hInstApp = NULL;
	ShellExecuteExA(&info);
	WaitForSingleObject(info.hProcess, INFINITE); // ���������ȴ��������п����Լ�����·����ű�����

	//��ȡ·����
	QString m_pathName = QCoreApplication::applicationDirPath() + "/miniFW.dll";
	m_pathName = QDir::toNativeSeparators(m_pathName);

	QFileInfo file(m_pathName);
	if (!file.exists())
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�Ҳ���miniFW.dll��"), QMessageBox::Yes, QMessageBox::Cancel);
		return false;
	}
	if (firewall_status)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����ǽ�Ѿ�������"), QMessageBox::Yes, QMessageBox::Cancel);
		return false;
	}

	//����Firewall�Ӽ�
	HKEY hKey;
	DWORD dwDisposition;
	if ((RegCreateKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Firewall",
		0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &hKey, &dwDisposition)) != ERROR_SUCCESS)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�޷�д��ע���"), QMessageBox::Yes, QMessageBox::Cancel);
		return false;
	}
	RegCloseKey(hKey);

	//��Catalog_EntriesĿ¼
	if ((RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\Protocol_Catalog9\\Catalog_Entries",
		0, KEY_READ, &hKey)) != ERROR_SUCCESS)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�޷���ע���"), QMessageBox::Yes, QMessageBox::Cancel);
		return false;
	}

	//����Catalog_EntriesĿ¼�µ�ÿһ���Ӽ�
	char tzSubKey[MAX_PATH];
	DWORD dwIndex = 0;
	while (RegEnumKeyA(hKey, dwIndex, tzSubKey, MAX_PATH) == ERROR_SUCCESS)
	{
		QString path("SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\Protocol_Catalog9\\Catalog_Entries\\");
		path += tzSubKey;
		QByteArray temp = path.toLocal8Bit();
		char* m_path = temp.data();
		//qDebug()<<m_path;

		//���Ӽ���000000000001��
		HKEY hKey2;
		RegOpenKeyExA(HKEY_LOCAL_MACHINE, m_path, 0, KEY_READ | KEY_WRITE, &hKey2);

		DWORD lpType = REG_BINARY;
		DWORD dataSize = 0;

		//��ȡPackedCatalogItem��ֵ����һ�λ�ȡ��������С���ڶ��λ�ȡ��ֵ
		RegQueryValueExA(hKey2, "PackedCatalogItem", nullptr, &lpType, nullptr, &dataSize);
		BYTE* pBuf = new BYTE[dataSize];
		RegQueryValueExA(hKey2, "PackedCatalogItem", nullptr, &lpType, (LPBYTE)pBuf, &dataSize);

		//����ṹ�屣���ֵ
		struct PACKED_CATALOG_ITEM
		{
			char spi_path[MAX_PATH]; //dll��·������
			WSAPROTOCOL_INFO protocol_info; //Э��Ľṹ��
		};

		PACKED_CATALOG_ITEM dataStruct;
		memcpy(&dataStruct, pBuf, sizeof(PACKED_CATALOG_ITEM));

		//QString str=QString::fromWCharArray(dataStruct.protocol_info.szProtocol);
		//qDebug()<<str<<dataStruct.protocol_info.dwCatalogEntryId;

		//�ж�ChainLenΪ1,��iAddressFamilyΪAF_INET,˵��ΪIPv4���������ṩ��
		if (dataStruct.protocol_info.ProtocolChain.ChainLen == 1 && dataStruct.protocol_info.iAddressFamily == AF_INET)
		{
			HKEY hKey3;
			RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Firewall", 0, KEY_WRITE, &hKey3);
			char id[MAX_PATH];
			wsprintfA(id, "%d", dataStruct.protocol_info.dwCatalogEntryId);

			//WritePrivateProfileStringA(id,"path",dataStruct.spi_path,".\\SystemProvider.ini");

			//�ѻ��������ṩ��·������
			if (RegSetValueExA(hKey3, id, 0, REG_SZ, (BYTE*)dataStruct.spi_path, sizeof(dataStruct.spi_path)) != ERROR_SUCCESS)
			{
				QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�޷�����·����"), QMessageBox::Yes, QMessageBox::Cancel);
				RegCloseKey(hKey3);
				RegCloseKey(hKey2);
				RegCloseKey(hKey);
				return false;
			}

			//��ϵͳ�����ṩ��·���滻���Զ�������ṩ��·��
			QByteArray temp = m_pathName.toLocal8Bit();
			char* m_path = temp.data();
			memset(dataStruct.spi_path, '\0', sizeof(dataStruct.spi_path));
			strcpy(dataStruct.spi_path, m_path);

			memcpy(pBuf, &dataStruct, sizeof(PACKED_CATALOG_ITEM));

			if (RegSetValueExA(hKey2, "PackedCatalogItem", 0, REG_BINARY, pBuf, sizeof(PACKED_CATALOG_ITEM)) != ERROR_SUCCESS)
			{
				QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�޷��滻·����"), QMessageBox::Yes, QMessageBox::Cancel);
				RegCloseKey(hKey3);
				RegCloseKey(hKey2);
				RegCloseKey(hKey);
				return false;
			}

			RegCloseKey(hKey3);
		}

		RegCloseKey(hKey2);
		dwIndex++;
	}

	RegCloseKey(hKey);

	//д���Զ���dll·��
	RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Firewall", 0, KEY_WRITE, &hKey);
	QByteArray temp = m_pathName.toLocal8Bit();
	char* m_path = temp.data();
	RegSetValueExA(hKey, "PathName", 0, REG_SZ, (BYTE*)m_path, (ULONG)strlen(m_path));
	RegCloseKey(hKey);

	return true;
}

bool InfoDlg::uninstall_firewall()
{
	//ж�ط���ǽ
	HKEY hKey, hKey2;

	if (!firewall_status)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("����ǽ"), QString::fromLocal8Bit("����ǽδ������"), QMessageBox::Yes, QMessageBox::Cancel);
		return false;
	}

	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Firewall", 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("����ǽ"), QString::fromLocal8Bit("��ע���ʧ�ܣ�"), QMessageBox::Yes, QMessageBox::Cancel);
		return false;
	}

	char* nameBuf = new char[MAX_PATH];
	BYTE* dataBuf = new BYTE[MAX_PATH];
	DWORD szData = MAX_PATH, szName = MAX_PATH, dwIndex = 0;
	while (RegEnumValueA(hKey, dwIndex, nameBuf, &szName, nullptr, nullptr, dataBuf, &szData) != ERROR_NO_MORE_ITEMS)
	{
		//��ԭϵͳ�����ṩ��·��
		HKEY hKey3;
		if ((RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\Protocol_Catalog9\\Catalog_Entries",
			0, KEY_READ, &hKey3)) != ERROR_SUCCESS)
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�޷���ע���"), QMessageBox::Yes, QMessageBox::Cancel);
			RegCloseKey(hKey);
			return false;
		}
		/*****************************************************/
		//����Catalog_EntriesĿ¼�µ�ÿһ���Ӽ�
		char tzSubKey[MAX_PATH];
		DWORD dwIndex2 = 0;
		while (RegEnumKeyA(hKey3, dwIndex2, tzSubKey, MAX_PATH) == ERROR_SUCCESS)
		{
			QString path("SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\Protocol_Catalog9\\Catalog_Entries\\");
			path += tzSubKey;
			QByteArray temp = path.toLocal8Bit();
			char* m_path = temp.data();
			//qDebug()<<m_path;

			//���Ӽ���000000000001��
			HKEY hKey4;
			RegOpenKeyExA(HKEY_LOCAL_MACHINE, m_path, 0, KEY_READ | KEY_WRITE, &hKey4);

			DWORD lpType = REG_BINARY;
			DWORD dataSize;

			//��ȡPackedCatalogItem��ֵ����һ�λ�ȡ��������С���ڶ��λ�ȡ��ֵ
			RegQueryValueExA(hKey4, "PackedCatalogItem", nullptr, &lpType, nullptr, &dataSize);
			BYTE* pBuf = new BYTE[dataSize];
			RegQueryValueExA(hKey4, "PackedCatalogItem", nullptr, &lpType, (LPBYTE)pBuf, &dataSize);

			//����ṹ�屣���ֵ
			struct PACKED_CATALOG_ITEM
			{
				char spi_path[MAX_PATH]; //dll��·������
				WSAPROTOCOL_INFO protocol_info; //Э��Ľṹ��
			};

			PACKED_CATALOG_ITEM dataStruct;
			memcpy(&dataStruct, pBuf, sizeof(PACKED_CATALOG_ITEM));

			//QString str=QString::fromWCharArray(dataStruct.protocol_info.szProtocol);
			//qDebug()<<str<<dataStruct.protocol_info.dwCatalogEntryId;

			//���id�����滻������id,���滻·��
			if (dataStruct.protocol_info.dwCatalogEntryId == (DWORD)atoi(nameBuf))
			{
				memcpy(dataStruct.spi_path, dataBuf, MAX_PATH);
				memcpy(pBuf, &dataStruct, sizeof(PACKED_CATALOG_ITEM));
				if (RegSetValueExA(hKey4, "PackedCatalogItem", 0, REG_BINARY, pBuf, sizeof(PACKED_CATALOG_ITEM)) != ERROR_SUCCESS)
				{
					QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�޷��滻·����"), QMessageBox::Yes, QMessageBox::Cancel);
					RegCloseKey(hKey4);
					RegCloseKey(hKey3);
					RegCloseKey(hKey);
					return false;
				}
			}

			dwIndex2++;
			RegCloseKey(hKey4);
		}
		/*************************************/
		RegCloseKey(hKey3);

		szName = MAX_PATH;
		dwIndex++;
	}

	RegCloseKey(hKey);

	//RegQueryValueExA(hKey,"PackedCatalogItem",NULL,&lpType,NULL,&dataSize);

	//ɾ��ini�ļ�
	/*QFile ini(".\\SystemProvider.ini");
	if(ini.exists())
	{
		ini.remove();
	}*/

	RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2", 0, KEY_ALL_ACCESS, &hKey2);
	if (RegDeleteKeyA(hKey2, "Firewall") != ERROR_SUCCESS)
	{
		QMessageBox::information(this, QString::fromLocal8Bit("����ǽ"), QString::fromLocal8Bit("δ��װ����ǽ��"), QMessageBox::Yes, QMessageBox::Cancel);
		RegCloseKey(hKey2);
		return false;
	}

	RegCloseKey(hKey2);
	return true;
}

bool InfoDlg::firewallStatus()
{
	HKEY hKey;
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Firewall", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return true;
	}
	return false;
}

QString InfoDlg::getLocalIP()
{
	QString strIp;
	QList<QHostAddress> ipAddressList = QNetworkInterface::allAddresses();
	// ��ȡ��һ����������IPv4��ַ
	int size = ipAddressList.size();
	//������·IP��Χ����Ҫ�ų�
	quint32 minIP = QHostAddress("169.254.1.0").toIPv4Address();
	quint32 maxIP = QHostAddress("169.254.254.255").toIPv4Address();
	for (int i = 0; i < size; i++)
	{
		if (ipAddressList.at(i) != QHostAddress::LocalHost && ipAddressList.at(i).toIPv4Address())
		{
			if (ipAddressList.at(i).toIPv4Address() >= minIP && ipAddressList.at(i).toIPv4Address() <= maxIP)
				continue;

			strIp = ipAddressList.at(i).toString();
			break;
		}
	}
	// ���û���ҵ������Ա���IP��ַΪIP
	if (strIp.isEmpty())
	{
		strIp = QHostAddress(QHostAddress::LocalHost).toString();
	}
	return strIp;
}
