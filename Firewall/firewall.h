#ifndef FIREWALL_H
#define FIREWALL_H

#include <QWidget>
#include <QMouseEvent>
#include <QTabBar>
#include <QFile>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QSettings>

#include "infodlg.h"
#include "logdlg.h"
#include "ruledlg.h"

namespace Ui
{
	class Firewall;
}

class Firewall : public QWidget
{
	Q_OBJECT

public:
	explicit Firewall(QWidget* parent = nullptr);
	~Firewall();

protected:
	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);
	virtual void keyPressEvent(QKeyEvent*);

signals:
	void changeStyle(int style);

private slots:
	void on_exitBtn_clicked();
	void on_infoBtn_clicked();
	void on_ruleBtn_clicked();
	void on_logBtn_clicked();
	void on_minBtn_clicked();
	void on_menuBtn_clicked();
	void changeTheme(QAction* action);

private:
	Ui::Firewall* ui;
	bool bPressed;//��ť�Ƿ���
	QPoint ptPressed;//��ť���µ�λ��
	QSystemTrayIcon* m_trayIcon;
	QMenu* m_trayMenu;
	QMenu* m_skinMenu;
	int m_style;
};

#endif // FIREWALL_H
