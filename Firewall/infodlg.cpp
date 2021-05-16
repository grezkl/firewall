#include "infodlg.h"
#include "ui_infodlg.h"

InfoDlg::InfoDlg(int style, QWidget* parent) :
	QWidget(parent),
	ui(new Ui::InfoDlg), m_style(style)
{
	ui->setupUi(this);

	//加载图片
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

	//加载qss
	QString styleQss;
	if (m_style == 0)
		styleQss = ":/qss/DarkSkin_infoDlg.qss";
	else if (m_style == 1)
		styleQss = ":/qss/BrightSkin_infoDlg.qss";

	QFile qssFile(styleQss);
	if (qssFile.open(QFile::ReadOnly))
		this->setStyleSheet(qssFile.readAll());
	qssFile.close();

	firewall_status = firewallStatus();//获取防火墙状态
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
	netSpeedWork = new NetSpeedWork();//不能指定父对象
	netSpeedWork->moveToThread(netspeed_thread);//把整个工作类移交给新的线程

	memory_thread = new QThread(this);
	memoryWork = new MemoryWork();
	memoryWork->moveToThread(memory_thread);

	cpu_thread = new QThread(this);
	cpuWork = new CpuWork();
	cpuWork->moveToThread(cpu_thread);

	//线程销毁时，工作类也销毁并释放内存空间
	connect(netspeed_thread, &QThread::finished, netSpeedWork, &QObject::deleteLater);
	connect(memory_thread, &QThread::finished, memoryWork, &QObject::deleteLater);
	connect(cpu_thread, &QThread::finished, cpuWork, &QObject::deleteLater);

	//更新网速信号
	connect(netSpeedWork, &NetSpeedWork::workDown, this, &InfoDlg::updateNetspeed);
	//更新内存占用信号
	connect(memoryWork, &MemoryWork::workDown, this, &InfoDlg::updateMemoryUsage);
	//更新cpu占用信号
	connect(cpuWork, &CpuWork::workDown, this, &InfoDlg::updateCpuUsage);

	//开始工作信号
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

	//调用QObject::deleteLater之后内存已经释放，指针要置空
	netSpeedWork = nullptr;
	memoryWork = nullptr;
	cpuWork = nullptr;

	delete ui;
}

void InfoDlg::on_startBtn_clicked()
{
	if (!firewall_status)
	{
		if (install_firewall())//安装防火墙
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

			//更新日志
			SYSTEMTIME sys;
			GetLocalTime(&sys);
			char time[50] = { 0 };
			sprintf_s(time, 50, "%4d/%02d/%02d %02d:%02d:%02d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);

			WritePrivateProfileStringA("Firewall", "Status", "1", "C:\\Windows\\FW_rule.ini");
			emit updateSelfLog(QString("[" + QString(time) + "]" + "\n" + QString::fromLocal8Bit("防火墙开启！") + "\n"));
			emit updateFWStatus(true);
		}
	}
	else
	{
		if (uninstall_firewall())//卸载防火墙
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

			//更新日志
			SYSTEMTIME sys;
			GetLocalTime(&sys);
			char time[50] = { 0 };
			sprintf_s(time, 50, "%4d/%02d/%02d %02d:%02d:%02d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);

			WritePrivateProfileStringA("Firewall", "Status", "0", "C:\\Windows\\FW_rule.ini");
			emit updateSelfLog(QString("[" + QString(time) + "]" + "\n" + QString::fromLocal8Bit("防火墙关闭！") + "\n"));
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
		usage = (usage * 1.46) > 100 ? 100 : (usage * 1.46); //校准
	QString qstrUsage = QString::number(usage) + "%";
	ui->label_cpu->setText(qstrUsage);
}

void InfoDlg::changeTheme(int style)
{
	//加载图片
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

	//加载css
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
	//重置LSP
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
	WaitForSingleObject(info.hProcess, INFINITE); // 必须阻塞等待，否则有可能自己改完路径后才被重置

	//获取路径名
	QString m_pathName = QCoreApplication::applicationDirPath() + "/miniFW.dll";
	m_pathName = QDir::toNativeSeparators(m_pathName);

	QFileInfo file(m_pathName);
	if (!file.exists())
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("找不到miniFW.dll！"), QMessageBox::Yes, QMessageBox::Cancel);
		return false;
	}
	if (firewall_status)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("防火墙已经启动！"), QMessageBox::Yes, QMessageBox::Cancel);
		return false;
	}

	//建立Firewall子键
	HKEY hKey;
	DWORD dwDisposition;
	if ((RegCreateKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Firewall",
		0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &hKey, &dwDisposition)) != ERROR_SUCCESS)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("无法写入注册表！"), QMessageBox::Yes, QMessageBox::Cancel);
		return false;
	}
	RegCloseKey(hKey);

	//打开Catalog_Entries目录
	if ((RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\Protocol_Catalog9\\Catalog_Entries",
		0, KEY_READ, &hKey)) != ERROR_SUCCESS)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("无法打开注册表！"), QMessageBox::Yes, QMessageBox::Cancel);
		return false;
	}

	//遍历Catalog_Entries目录下的每一个子键
	char tzSubKey[MAX_PATH];
	DWORD dwIndex = 0;
	while (RegEnumKeyA(hKey, dwIndex, tzSubKey, MAX_PATH) == ERROR_SUCCESS)
	{
		QString path("SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\Protocol_Catalog9\\Catalog_Entries\\");
		path += tzSubKey;
		QByteArray temp = path.toLocal8Bit();
		char* m_path = temp.data();
		//qDebug()<<m_path;

		//打开子键如000000000001等
		HKEY hKey2;
		RegOpenKeyExA(HKEY_LOCAL_MACHINE, m_path, 0, KEY_READ | KEY_WRITE, &hKey2);

		DWORD lpType = REG_BINARY;
		DWORD dataSize = 0;

		//获取PackedCatalogItem键值，第一次获取缓冲区大小，第二次获取键值
		RegQueryValueExA(hKey2, "PackedCatalogItem", nullptr, &lpType, nullptr, &dataSize);
		BYTE* pBuf = new BYTE[dataSize];
		RegQueryValueExA(hKey2, "PackedCatalogItem", nullptr, &lpType, (LPBYTE)pBuf, &dataSize);

		//构造结构体保存键值
		struct PACKED_CATALOG_ITEM
		{
			char spi_path[MAX_PATH]; //dll的路径名称
			WSAPROTOCOL_INFO protocol_info; //协议的结构体
		};

		PACKED_CATALOG_ITEM dataStruct;
		memcpy(&dataStruct, pBuf, sizeof(PACKED_CATALOG_ITEM));

		//QString str=QString::fromWCharArray(dataStruct.protocol_info.szProtocol);
		//qDebug()<<str<<dataStruct.protocol_info.dwCatalogEntryId;

		//判断ChainLen为1,且iAddressFamily为AF_INET,说明为IPv4基础服务提供者
		if (dataStruct.protocol_info.ProtocolChain.ChainLen == 1 && dataStruct.protocol_info.iAddressFamily == AF_INET)
		{
			HKEY hKey3;
			RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Firewall", 0, KEY_WRITE, &hKey3);
			char id[MAX_PATH];
			wsprintfA(id, "%d", dataStruct.protocol_info.dwCatalogEntryId);

			//WritePrivateProfileStringA(id,"path",dataStruct.spi_path,".\\SystemProvider.ini");

			//把基础服务提供者路径保存
			if (RegSetValueExA(hKey3, id, 0, REG_SZ, (BYTE*)dataStruct.spi_path, sizeof(dataStruct.spi_path)) != ERROR_SUCCESS)
			{
				QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("无法保存路径！"), QMessageBox::Yes, QMessageBox::Cancel);
				RegCloseKey(hKey3);
				RegCloseKey(hKey2);
				RegCloseKey(hKey);
				return false;
			}

			//把系统服务提供者路径替换成自定义服务提供者路径
			QByteArray temp = m_pathName.toLocal8Bit();
			char* m_path = temp.data();
			memset(dataStruct.spi_path, '\0', sizeof(dataStruct.spi_path));
			strcpy(dataStruct.spi_path, m_path);

			memcpy(pBuf, &dataStruct, sizeof(PACKED_CATALOG_ITEM));

			if (RegSetValueExA(hKey2, "PackedCatalogItem", 0, REG_BINARY, pBuf, sizeof(PACKED_CATALOG_ITEM)) != ERROR_SUCCESS)
			{
				QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("无法替换路径！"), QMessageBox::Yes, QMessageBox::Cancel);
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

	//写入自定义dll路径
	RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Firewall", 0, KEY_WRITE, &hKey);
	QByteArray temp = m_pathName.toLocal8Bit();
	char* m_path = temp.data();
	RegSetValueExA(hKey, "PathName", 0, REG_SZ, (BYTE*)m_path, (ULONG)strlen(m_path));
	RegCloseKey(hKey);

	return true;
}

bool InfoDlg::uninstall_firewall()
{
	//卸载防火墙
	HKEY hKey, hKey2;

	if (!firewall_status)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("防火墙"), QString::fromLocal8Bit("防火墙未启动！"), QMessageBox::Yes, QMessageBox::Cancel);
		return false;
	}

	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Firewall", 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("防火墙"), QString::fromLocal8Bit("打开注册表失败！"), QMessageBox::Yes, QMessageBox::Cancel);
		return false;
	}

	char* nameBuf = new char[MAX_PATH];
	BYTE* dataBuf = new BYTE[MAX_PATH];
	DWORD szData = MAX_PATH, szName = MAX_PATH, dwIndex = 0;
	while (RegEnumValueA(hKey, dwIndex, nameBuf, &szName, nullptr, nullptr, dataBuf, &szData) != ERROR_NO_MORE_ITEMS)
	{
		//还原系统服务提供者路径
		HKEY hKey3;
		if ((RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\Protocol_Catalog9\\Catalog_Entries",
			0, KEY_READ, &hKey3)) != ERROR_SUCCESS)
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("无法打开注册表！"), QMessageBox::Yes, QMessageBox::Cancel);
			RegCloseKey(hKey);
			return false;
		}
		/*****************************************************/
		//遍历Catalog_Entries目录下的每一个子键
		char tzSubKey[MAX_PATH];
		DWORD dwIndex2 = 0;
		while (RegEnumKeyA(hKey3, dwIndex2, tzSubKey, MAX_PATH) == ERROR_SUCCESS)
		{
			QString path("SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters\\Protocol_Catalog9\\Catalog_Entries\\");
			path += tzSubKey;
			QByteArray temp = path.toLocal8Bit();
			char* m_path = temp.data();
			//qDebug()<<m_path;

			//打开子键如000000000001等
			HKEY hKey4;
			RegOpenKeyExA(HKEY_LOCAL_MACHINE, m_path, 0, KEY_READ | KEY_WRITE, &hKey4);

			DWORD lpType = REG_BINARY;
			DWORD dataSize;

			//获取PackedCatalogItem键值，第一次获取缓冲区大小，第二次获取键值
			RegQueryValueExA(hKey4, "PackedCatalogItem", nullptr, &lpType, nullptr, &dataSize);
			BYTE* pBuf = new BYTE[dataSize];
			RegQueryValueExA(hKey4, "PackedCatalogItem", nullptr, &lpType, (LPBYTE)pBuf, &dataSize);

			//构造结构体保存键值
			struct PACKED_CATALOG_ITEM
			{
				char spi_path[MAX_PATH]; //dll的路径名称
				WSAPROTOCOL_INFO protocol_info; //协议的结构体
			};

			PACKED_CATALOG_ITEM dataStruct;
			memcpy(&dataStruct, pBuf, sizeof(PACKED_CATALOG_ITEM));

			//QString str=QString::fromWCharArray(dataStruct.protocol_info.szProtocol);
			//qDebug()<<str<<dataStruct.protocol_info.dwCatalogEntryId;

			//如果id等于替换出来的id,则替换路径
			if (dataStruct.protocol_info.dwCatalogEntryId == (DWORD)atoi(nameBuf))
			{
				memcpy(dataStruct.spi_path, dataBuf, MAX_PATH);
				memcpy(pBuf, &dataStruct, sizeof(PACKED_CATALOG_ITEM));
				if (RegSetValueExA(hKey4, "PackedCatalogItem", 0, REG_BINARY, pBuf, sizeof(PACKED_CATALOG_ITEM)) != ERROR_SUCCESS)
				{
					QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("无法替换路径！"), QMessageBox::Yes, QMessageBox::Cancel);
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

	//删除ini文件
	/*QFile ini(".\\SystemProvider.ini");
	if(ini.exists())
	{
		ini.remove();
	}*/

	RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2", 0, KEY_ALL_ACCESS, &hKey2);
	if (RegDeleteKeyA(hKey2, "Firewall") != ERROR_SUCCESS)
	{
		QMessageBox::information(this, QString::fromLocal8Bit("防火墙"), QString::fromLocal8Bit("未安装防火墙！"), QMessageBox::Yes, QMessageBox::Cancel);
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
	// 获取第一个本主机的IPv4地址
	int size = ipAddressList.size();
	//本地链路IP范围，需要排除
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
	// 如果没有找到，则以本地IP地址为IP
	if (strIp.isEmpty())
	{
		strIp = QHostAddress(QHostAddress::LocalHost).toString();
	}
	return strIp;
}
