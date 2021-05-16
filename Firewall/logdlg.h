#ifndef LOGDLG_H
#define LOGDLG_H

#include <QWidget>
#include <QFile>
#include <QThread>
#include <QPlainTextEdit>
#include <QTextStream>
#include <QFileDialog>

#include "logwork.h"

namespace Ui
{
	class LogDlg;
}

class LogDlg : public QWidget
{
	Q_OBJECT

public:
	explicit LogDlg(int style, QWidget *parent = nullptr);
	~LogDlg();

public slots:
	void changeTheme(int style);
	void updateLog(QString log);
	void updateSelfLog(QString log);
	void updateFWStatus(bool status);

signals:
	void startThread();

private slots:
	void on_exportLogBtn_clicked();
	void on_exportAllLogBtn_clicked();

private:
	Ui::LogDlg *ui;
	int m_style;
	QThread* log_thread;
	LogWork* logWork;
	QFile* m_file;
	bool bStopWriteLogFile;
	bool firewall_status;
	QString lastLog;
};

#endif // LOGDLG_H
