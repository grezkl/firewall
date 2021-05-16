#ifndef RULEDLG_H
#define RULEDLG_H

#include <QWidget>
#include <QSettings>
#include <QComboBox>
#include <QTableWidgetItem>
#include <QLineEdit>
#include <QFile>
#include <QMenu>
#include <QFileDialog>

#include <fstream>
#include <unordered_map>
#include <string>
#include <windows.h>
#include <vector>

#include "behaviorbox.h"

#pragma comment(lib, "version")

namespace Ui
{
	class RuleDlg;
}

class RuleDlg : public QWidget
{
	Q_OBJECT

public:
	explicit RuleDlg(int style, QWidget* parent = nullptr);
	~RuleDlg();

signals:
	void updateSelfLog(QString log);

private slots:
	void EditRule(int row, int column);
	void BehaviorEditFinished(int row, int column, int index);

	void on_blockAllBtn_clicked();
	void on_addRuleBtn_clicked();
	void on_addRuleBtn_2_clicked();
	void showMenu(QPoint point);
	void saveRule();
	void deleteRule();
	void importRule();
	void exportSelectedRule();
	void exportAllRule();

public slots:
	void changeTheme(int style);
	void updateFWStatus(bool status);

private:
	Ui::RuleDlg* ui;
	bool bBlockAll;
	std::unordered_map<std::string, std::pair<std::string, bool>> rulesMap; //用哈希表存规则
	int m_style;
	bool firewall_status;
};

#endif // RULEDLG_H
