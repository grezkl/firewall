#ifndef INFODLG_H
#define INFODLG_H

#include <QWidget>
#include <QPixmap>
#include <QIcon>
#include <QThread>
#include <QDir>
#include <QMessageBox>
#include <QFile>
#include <QList>
#include <QHostAddress>
#include <QNetworkInterface>

#include <windows.h>
#include "iphlpapi.h"
#include <shellapi.h>

#include "netspeedwork.h"
#include "memorywork.h"
#include "cpuwork.h"

namespace Ui
{
	class InfoDlg;
}

class InfoDlg : public QWidget
{
	Q_OBJECT

public:
	explicit InfoDlg(int style, QWidget* parent = nullptr);
	~InfoDlg();

signals:
	void startThread();
	void updateSelfLog(QString log);
	void updateFWStatus(bool status);

private slots:
	void on_startBtn_clicked();
	void updateNetspeed(double up, double down);
	void updateMemoryUsage(int usage);
	void updateCpuUsage(int usage);

public slots:
	void changeTheme(int style);

private:
	Ui::InfoDlg* ui;
	bool firewall_status;
	NetSpeedWork* netSpeedWork;
	MemoryWork* memoryWork;
	CpuWork* cpuWork;
	QThread* netspeed_thread, * memory_thread, * cpu_thread;
	int m_style;

	bool install_firewall();
	bool uninstall_firewall();
	bool firewallStatus();
	QString getLocalIP();
};

#endif // INFODLG_H
