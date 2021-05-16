#include "logdlg.h"
#include "ui_logdlg.h"

LogDlg::LogDlg(int style, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::LogDlg), m_style(style), bStopWriteLogFile(false), firewall_status(false)
{
	ui->setupUi(this);

	m_file = new QFile(".\\Firewall_log.log");

	//加载图片
	QPixmap pixmap;
	if (m_style == 0)
	{
		pixmap.load(":/images/exportLog2.png");
		ui->exportLogBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/exportAllLog2.png");
		ui->exportAllLogBtn->setIcon(QIcon(pixmap));
	}
	else
	{
		pixmap.load(":/images/exportLog2_b.png");
		ui->exportLogBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/exportAllLog2_b.png");
		ui->exportAllLogBtn->setIcon(QIcon(pixmap));
	}

	//加载qss
	QString styleQss;
	if (m_style == 0)
		styleQss = ":/qss/DarkSkin_logDlg.qss";
	else if (m_style == 1)
		styleQss = ":/qss/BrightSkin_logDlg.qss";

	QFile qssFile(styleQss);
	if (qssFile.open(QFile::ReadOnly))
		this->setStyleSheet(qssFile.readAll());
	qssFile.close();

	log_thread = new QThread(this);
	logWork = new LogWork();
	logWork->moveToThread(log_thread);
	connect(log_thread, &QThread::finished, logWork, &QObject::deleteLater);
	connect(logWork, &LogWork::receiveLog, this, &LogDlg::updateLog);
	connect(this, &LogDlg::startThread, logWork, &LogWork::work);
	if (!log_thread->isRunning())
		log_thread->start();
	emit startThread();

	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char time[50] = { 0 };
	sprintf_s(time, 50, "%4d/%02d/%02d %02d:%02d:%02d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);

	HKEY hKey;
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Firewall", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		firewall_status = true;
	}

	QString text = QString("[" + QString(time) + "]" + "\n" + QString::fromLocal8Bit("Firewall程序启动！"));
	if (firewall_status)
		text += QString::fromLocal8Bit("防火墙状态为：[已启动]");
	else
		text += QString::fromLocal8Bit("防火墙状态为：[未启动]");
	text += "\n";
	
	if (!bStopWriteLogFile)
	{
		ui->plainTextEdit->appendPlainText(text);
		m_file->open(QIODevice::WriteOnly | QIODevice::Append);
		QTextStream stream(m_file);
		stream << text;
		m_file->flush();
		m_file->close();
	}
}

LogDlg::~LogDlg()
{
	logWork->terminal();
	log_thread->quit();
	log_thread->wait();
	logWork = nullptr;
	delete m_file;
	delete ui;
}

void LogDlg::changeTheme(int style)
{
	//加载图片
	QPixmap pixmap;
	if (style == 0)
	{
		pixmap.load(":/images/exportLog2.png");
		ui->exportLogBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/exportAllLog2.png");
		ui->exportAllLogBtn->setIcon(QIcon(pixmap));
	}
	else
	{
		pixmap.load(":/images/exportLog2_b.png");
		ui->exportLogBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/exportAllLog2_b.png");
		ui->exportAllLogBtn->setIcon(QIcon(pixmap));
	}

	//加载qss
	QString styleQss;
	if (style == 0)
		styleQss = ":/qss/DarkSkin_logDlg.qss";
	else if (style == 1)
		styleQss = ":/qss/BrightSkin_logDlg.qss";

	QFile qssFile(styleQss);
	if (qssFile.open(QFile::ReadOnly))
		this->setStyleSheet(qssFile.readAll());
	qssFile.close();

	m_style = style;
}

void LogDlg::updateLog(QString log)
{
	if (!firewall_status)
		return;

	if (log == lastLog)
		return;

	lastLog = log;

	QString name, path, time, behavior;
	QStringList list = log.split("\"");
	time = list[0];
	path = list[1];
	behavior = list[2];
	name = path.right(path.length() - path.lastIndexOf("\\") - 1);
	
	//获取文件详细信息，获取程序描述
	int size = 0;
	std::string p(path.toLocal8Bit());
	if (size = GetFileVersionInfoSizeA(p.c_str(), NULL))
	{
		char* versionInfo = new char[size];
		memset(versionInfo, 0, size);
		if (GetFileVersionInfoA(p.c_str(), NULL, size, versionInfo))
		{
			//微软官方提供结构体
			struct LANGANDCODEPAGE {
				WORD wLanguage;
				WORD wCodePage;
			} *lpTranslate;

			UINT cbTranslate = 0;

			if (VerQueryValueA(versionInfo, "\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &cbTranslate))
			{
				char* info = new char[50];
				memset(info, 0, 50);
				sprintf_s(info, 50, "\\StringFileInfo\\%04x%04x\\FileDescription", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
				void* buf;
				if (VerQueryValueA(versionInfo, info, &buf, &cbTranslate))
				{
					if (cbTranslate > 1)
						name = QString::fromLocal8Bit((char*)buf);
				}
				delete[] info;
			}
		}
		delete[] versionInfo;
	}

	QString text = QString("[" + time + "]" + "\n" + "[" + name + "]" + behavior + "\n");

	if (!bStopWriteLogFile)
	{
		ui->plainTextEdit->appendPlainText(text);
		m_file->open(QIODevice::WriteOnly | QIODevice::Append);
		QTextStream stream(m_file);
		stream << text;
		m_file->flush();
		m_file->close();
	}
}

void LogDlg::updateSelfLog(QString log)
{
	if (!bStopWriteLogFile)
	{
		ui->plainTextEdit->appendPlainText(log);
		m_file->open(QIODevice::WriteOnly | QIODevice::Append);
		QTextStream stream(m_file);
		stream << log;
		m_file->flush();
		m_file->close();
	}
}

void LogDlg::on_exportLogBtn_clicked()
{
	bStopWriteLogFile = true;

	QString path = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("请选择保存路径"), "C:\\Users\\62686\\Desktop\\Firewall_log.log", "LOG(*.log)");
	QFile file(path);
	if (file.exists())
		file.remove();
	file.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream stream(&file);
	stream << ui->plainTextEdit->toPlainText();
	file.flush();
	file.close();

	bStopWriteLogFile = false;
}

void LogDlg::on_exportAllLogBtn_clicked()
{
	bStopWriteLogFile = true;

	QString path = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("请选择保存路径"), "C:\\Users\\62686\\Desktop\\Firewall_log_all.log", "LOG(*.log)");
	m_file->open(QIODevice::ReadOnly);
	QString text = QString::fromLocal8Bit(m_file->readAll());
	m_file->flush();
	m_file->close();

	QFile file(path);
	if (file.exists())
		file.remove();
	file.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream stream(&file);
	stream << text;
	file.flush();
	file.close();

	bStopWriteLogFile = false;
}

void LogDlg::updateFWStatus(bool status)
{
	firewall_status = status;
}
