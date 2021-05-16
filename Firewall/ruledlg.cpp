#include "ruledlg.h"
#include "ui_ruledlg.h"

#include <QMessageBox>

RuleDlg::RuleDlg(int style, QWidget* parent) : 
	QWidget(parent), ui(new Ui::RuleDlg), bBlockAll(false), m_style(style), firewall_status(false)
{
	ui->setupUi(this);

	bBlockAll = GetPrivateProfileIntA("Firewall", "BlockAll", 0, "C:\\Windows\\FW_rule.ini");
	HKEY hKey;
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Firewall", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		firewall_status = true;
	}

	//����ͼƬ
	QPixmap pixmap;
	if (m_style == 0)
	{
		pixmap.load(":/images/addRule.png");
		ui->addRuleBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/addRule_2.png");
		ui->addRuleBtn_2->setIcon(QIcon(pixmap));
	}
	else
	{
		pixmap.load(":/images/addRule_b.png");
		ui->addRuleBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/addRule_2_b.png");
		ui->addRuleBtn_2->setIcon(QIcon(pixmap));
	}

	//����qss
	QString styleQss;
	if (m_style == 0)
		styleQss = ":/qss/DarkSkin_ruleDlg.qss";
	else if (m_style == 1)
		styleQss = ":/qss/BrightSkin_ruleDlg.qss";

	QFile qssFile(styleQss);
	if (qssFile.open(QFile::ReadOnly))
		this->setStyleSheet(qssFile.readAll());
	qssFile.close();

	//�̶��п�
	ui->tableWidget->setColumnWidth(0, 100);
	ui->tableWidget->setColumnWidth(1, 300);
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);

	//����Ӧ�п�
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

	//�и���Ӧ����
	ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

	//�����Ҽ��˵�
	ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this, &RuleDlg::showMenu);
	
	//���ñ�ͷ���ɵ��
	ui->tableWidget->horizontalHeader()->setSectionsClickable(false);
	ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	//���ñ��Ϊֻ�����Լ�ʵ��˫���༭���Դ���˫���༭
	//�ڱ༭��ʱ��������ɫ�ں�ɫ������Ҳ�Ǻ�ɫ��̫������= =
	ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//���ñ�ͷ�ɼ�
	ui->tableWidget->horizontalHeader()->setVisible(true);

	//��ȡ����
	char buf[10240] = { 0 };
	int ret = GetPrivateProfileSectionNamesA(buf, 10240, "C:\\Windows\\FW_rule.ini");
	if (ret != 10238) // ���������������ͻ᷵��size-2
	{
		std::vector<std::string> vec;
		std::string section;
		//��ȡ����section
		for (int i = 0; i < ret; i++)
		{
			if (buf[i] == '\0')
			{
				if (section != "Firewall")
					vec.push_back(section);
				section.clear();
			}
			else
			{
				section += buf[i];
			}
		}
		//��ȡ���й��򲢴��map
		for (auto section : vec)
		{
			char name[260] = { 0 };
			GetPrivateProfileStringA(section.c_str(), "name", NULL, name, 260, "C:\\Windows\\FW_rule.ini");
			bool isBlock = false;
			isBlock = GetPrivateProfileIntA(section.c_str(), "isBlock", 0, "C:\\Windows\\FW_rule.ini");
			rulesMap[section] = std::pair<std::string, bool>(name, isBlock);
		}
	}

	if (bBlockAll)
	{
		QPixmap pixmap_btn;
		if (m_style == 0)
			pixmap_btn.load(":/images/block_all.png");
		else
			pixmap_btn.load(":/images/block_all_b.png");
		QIcon icon_btn(pixmap_btn);
		ui->blockAllBtn->setIcon(icon_btn);
	}
	else
	{
		QPixmap pixmap_btn;
		if (m_style == 0)
			pixmap_btn.load(":/images/pass_all.png");
		else
			pixmap_btn.load(":/images/pass_all_b.png");
		QIcon icon_btn(pixmap_btn);
		ui->blockAllBtn->setIcon(icon_btn);
	}


	if (!rulesMap.empty())
	{
		for (auto& a : rulesMap)
		{
			int row = ui->tableWidget->rowCount();
			ui->tableWidget->insertRow(row);
			ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::fromLocal8Bit(a.first.data())));
			ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::fromLocal8Bit(a.second.first.data())));
			ui->tableWidget->item(row, 0)->setToolTip(QString::fromLocal8Bit(a.second.first.data()));
			ui->tableWidget->item(row, 1)->setToolTip(QString::fromLocal8Bit(a.first.data())); // ���������ʾ�ı�
			ui->tableWidget->setItem(row, 2, new QTableWidgetItem(a.second.second == false ? 
										QString::fromLocal8Bit("����") : QString::fromLocal8Bit("����")));
			//Ԫ�ؾ���
			ui->tableWidget->item(row, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			ui->tableWidget->item(row, 2)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		}
	}

	//�Լ�ʵ��˫���༭����
	connect(ui->tableWidget, &QTableWidget::cellDoubleClicked, this, &RuleDlg::EditRule);

}

RuleDlg::~RuleDlg()
{
	saveRule();
	delete ui;
	rulesMap.clear();
}

void RuleDlg::on_blockAllBtn_clicked()
{
	if (!bBlockAll)
	{
		QPixmap pixmap_btn;
		if (m_style == 0)
			pixmap_btn.load(":/images/block_all.png");
		else
			pixmap_btn.load(":/images/block_all_b.png");
		QIcon icon_btn(pixmap_btn);
		ui->blockAllBtn->setIcon(icon_btn);

		bBlockAll = true;
		WritePrivateProfileStringA("Firewall", "BlockAll", "1", "C:\\Windows\\FW_rule.ini");

		//������־
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		char time[50] = { 0 };
		sprintf_s(time, 50, "%4d/%02d/%02d %02d:%02d:%02d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);

		emit updateSelfLog(QString("[" + QString(time) + "]" + "\n" + QString::fromLocal8Bit("����ǽ��Ϊ����Ϊ��[ȫ������]") + "\n"));
	}
	else
	{
		QPixmap pixmap_btn;
		if (m_style == 0)
			pixmap_btn.load(":/images/pass_all.png");
		else
			pixmap_btn.load(":/images/pass_all_b.png");
		QIcon icon_btn(pixmap_btn);
		ui->blockAllBtn->setIcon(icon_btn);

		bBlockAll = false;
		WritePrivateProfileStringA("Firewall", "BlockAll", "0", "C:\\Windows\\FW_rule.ini");

		//������־
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		char time[50] = { 0 };
		sprintf_s(time, 50, "%4d/%02d/%02d %02d:%02d:%02d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);

		emit updateSelfLog(QString("[" + QString(time) + "]" + "\n" + QString::fromLocal8Bit("����ǽ��Ϊ����Ϊ��[����������]") + "\n"));
	}
}

void RuleDlg::on_addRuleBtn_clicked()
{
	//����Ԥ�����
	std::ifstream in(".\\pathList.txt");
	if (in)
	{
		//���ö�ѡ����ʾ�ղ���ӵ���
		ui->tableWidget->setSelectionMode(QAbstractItemView::MultiSelection);
		while (in.peek() != EOF)
		{
			char name[260] = { 0 };
			char path[260] = { 0 };
			in.getline(name, 260);
			in.getline(path, 260);

			int rowCount = ui->tableWidget->rowCount();

			//ȥ��
			int i = 0;
			for (; i < rowCount; i++)
			{
				if (ui->tableWidget->item(i, 1)->text() == QString(path))
					break;
			}
			if (i < rowCount)
			{
				if (ui->tableWidget->item(i, 1)->text() == QString(path))
					continue;
			}

			ui->tableWidget->insertRow(rowCount);
			ui->tableWidget->setItem(rowCount, 0, new QTableWidgetItem(QString(name)));
			ui->tableWidget->setItem(rowCount, 1, new QTableWidgetItem(QString(path)));
			ui->tableWidget->item(rowCount, 0)->setToolTip(QString(name));
			ui->tableWidget->item(rowCount, 1)->setToolTip(QString(path)); // ���������ʾ�ı�
			ui->tableWidget->setItem(rowCount, 2, new QTableWidgetItem(QString::fromLocal8Bit("����")));

			ui->tableWidget->item(rowCount, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			ui->tableWidget->item(rowCount, 2)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

			ui->tableWidget->scrollToBottom();
			ui->tableWidget->selectRow(rowCount);
		}
		//���û�����ѡ��
		ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
		saveRule();
		QMessageBox::information(this, "Firewall", QString::fromLocal8Bit("�����Ԥ��Ӧ�ó���"));
	}
	else
	{
		QMessageBox::warning(this, "Firewall", QString::fromLocal8Bit("�Ҳ���Ԥ������ļ���"));
	}
}

void RuleDlg::on_addRuleBtn_2_clicked()
{
	QString file = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("��ѡ��Ҫ��ӵ�����Ŀ�ִ���ļ�"), "C:\\", "EXE(*.exe)");
	if (!file.isEmpty())
	{
		QFileInfo fileInfo(file);
		QString name(fileInfo.fileName());
		QString path(fileInfo.filePath());
		path.replace("/", "\\");

		//��ȡ�ļ���ϸ��Ϣ����ȡ��������
		int size = 0;
		std::string p(path.toLocal8Bit());
		if (size = GetFileVersionInfoSizeA(p.c_str(), NULL))
		{
			char* versionInfo = new char[size];
			memset(versionInfo, 0, size);
			if (GetFileVersionInfoA(p.c_str(), NULL, size, versionInfo))
			{
				//΢��ٷ��ṩ�ṹ��
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

		int rowCount = ui->tableWidget->rowCount();

		//ȥ��
		int i = 0;
		for (; i < rowCount; i++)
		{
			if (ui->tableWidget->item(i, 1)->text() == path)
				break;
		}
		if (i < rowCount)
		{
			if (ui->tableWidget->item(i, 1)->text() == path)
			{
				QMessageBox::information(this, "Firewall", QString::fromLocal8Bit("�ó����Ѿ����ڣ�"));
				return;
			}
		}

		ui->tableWidget->insertRow(rowCount);
		ui->tableWidget->setItem(rowCount, 0, new QTableWidgetItem(name));
		ui->tableWidget->setItem(rowCount, 1, new QTableWidgetItem(path));
		ui->tableWidget->item(rowCount, 0)->setToolTip(name);
		ui->tableWidget->item(rowCount, 1)->setToolTip(path); // ���������ʾ�ı�
		ui->tableWidget->setItem(rowCount, 2, new QTableWidgetItem(QString::fromLocal8Bit("����")));

		ui->tableWidget->item(rowCount, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		ui->tableWidget->item(rowCount, 2)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

		ui->tableWidget->scrollToBottom();
		ui->tableWidget->selectRow(rowCount);
		saveRule();
	}
}

void RuleDlg::EditRule(int row, int column)
{
	QString cellStr;
	//���������Ѿ������ݣ����ø����ݳ�ʼ�������
	if (ui->tableWidget->item(row, column))
		cellStr = ui->tableWidget->item(row, column)->text();


	if (column == 2)
	{
		BehaviorBox* comBox = new BehaviorBox(row, column, this);
		//�������ѡ���
		comBox->addItem(QString::fromLocal8Bit("����"));
		comBox->addItem(QString::fromLocal8Bit("����"));

		QFile qssFile(":/qss/DarkSkin_ruleDlg.qss");
		qssFile.open(QFile::ReadOnly);
		comBox->setStyleSheet(qssFile.readAll() + "QComboBox{background-color:#66CCFF;color:black;border-radius:0px}");
		qssFile.close();

		if (cellStr == QString::fromLocal8Bit("����"))
			comBox->setCurrentIndex(0);
		else if (cellStr == QString::fromLocal8Bit("����"))
			comBox->setCurrentIndex(1);

		ui->tableWidget->setCellWidget(row, column, comBox);

		connect(comBox, &BehaviorBox::activated, this, &RuleDlg::BehaviorEditFinished);
	}
}

void RuleDlg::BehaviorEditFinished(int row, int column, int index)
{
	QTableWidget* currentWidget = ui->tableWidget;

	if (!currentWidget)
		return;

	QString text;
	if (index == 0)
		text = QString::fromLocal8Bit("����");
	else if (index == 1)
		text = QString::fromLocal8Bit("����");

	currentWidget->removeCellWidget(row, column);

	if (currentWidget->item(row, column))
	{
		currentWidget->item(row, column)->setText(text);
	}
	else
	{
		QTableWidgetItem* item = new QTableWidgetItem(text);
		currentWidget->setItem(row, column, item);
	}

	currentWidget->item(row, column)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	saveRule();
}

void RuleDlg::showMenu(QPoint pos)
{
	static QMenu* menu = new QMenu(ui->tableWidget);
	static QAction* action1 = new QAction(QString::fromLocal8Bit("ɾ��"), menu);
	menu->addAction(action1);
	static QAction* action2 = new QAction(QString::fromLocal8Bit("�������"), menu);
	menu->addAction(action2);
	static QAction* action3 = new QAction(QString::fromLocal8Bit("����ѡ�й���"), menu);
	menu->addAction(action3);
	static QAction* action4 = new QAction(QString::fromLocal8Bit("����ȫ������"), menu);
	menu->addAction(action4);

	connect(action1, &QAction::triggered, this, &RuleDlg::deleteRule, Qt::UniqueConnection);
	connect(action2, &QAction::triggered, this, &RuleDlg::importRule, Qt::UniqueConnection);
	connect(action3, &QAction::triggered, this, &RuleDlg::exportSelectedRule, Qt::UniqueConnection);
	connect(action4, &QAction::triggered, this, &RuleDlg::exportAllRule, Qt::UniqueConnection);

	menu->popup(cursor().pos());
}

void RuleDlg::deleteRule()
{
	QList<QTableWidgetSelectionRange> ranges = ui->tableWidget->selectedRanges();
	int count = ranges.count();
	int removeCount = 0;
	for (int i = 0; i < count; i++)
	{
		int topRow = ranges.at(i).topRow();
		int bottomRow = ranges.at(i).bottomRow();
		for (int j = topRow; j <= bottomRow; j++)
		{
			std::string path(ui->tableWidget->item(j - removeCount, 1)->text().toLocal8Bit());
			rulesMap.erase(path);
			WritePrivateProfileStringA(path.c_str(), NULL, NULL, "C:\\Windows\\FW_rule.ini");
			ui->tableWidget->removeRow(j - removeCount);
			removeCount++;
		}
	}
}

void RuleDlg::importRule()
{
	QString filePath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("��ѡ����Ĺ���ini�ļ�"), "C:\\", "INI(*.ini)");
	if (filePath.isEmpty())
		return;
	char* buf = new char[20480];
	memset(buf, 0, 20480);
	int len = GetPrivateProfileSectionNamesA(buf, 20480, filePath.toLocal8Bit().constData());
	QStringList sections;
	QString str = QString::fromLocal8Bit(buf, len);
	sections = str.split(QChar('\0'));
	if (sections[0] != "Firewall")
	{
		QMessageBox::warning(NULL, "Firewall", QString::fromLocal8Bit("�����ļ����ڴ���"));
		return;
	}
	sections.removeAt(0);
	
	for (auto& str : sections)
	{
		std::string path(str.toLocal8Bit());
		char name[260] = { 0 };
		GetPrivateProfileStringA(path.c_str(), "name", NULL, name, 260, filePath.toLocal8Bit().constData());
		bool isBlock = false;
		isBlock = GetPrivateProfileIntA(path.c_str(), "isBlock", 0, filePath.toLocal8Bit().constData());
		rulesMap[path] = std::pair<std::string, bool>(name, isBlock);
	}

	//���tableWidget�������
	int rowCount = ui->tableWidget->rowCount();
	for (int i = 0; i < rowCount; i++)
	{
		ui->tableWidget->removeRow(0);
	}

	if (!rulesMap.empty())
	{
		for (auto& a : rulesMap)
		{
			int row = ui->tableWidget->rowCount();
			ui->tableWidget->insertRow(row);
			ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::fromLocal8Bit(a.first.data())));
			ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::fromLocal8Bit(a.second.first.data())));
			ui->tableWidget->item(row, 0)->setToolTip(QString::fromLocal8Bit(a.second.first.data()));
			ui->tableWidget->item(row, 1)->setToolTip(QString::fromLocal8Bit(a.first.data())); // ���������ʾ�ı�
			ui->tableWidget->setItem(row, 2, new QTableWidgetItem(a.second.second == false ?
				QString::fromLocal8Bit("����") : QString::fromLocal8Bit("����")));
			//Ԫ�ؾ���
			ui->tableWidget->item(row, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			ui->tableWidget->item(row, 2)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		}
	}
}

void RuleDlg::exportSelectedRule()
{
	if (rulesMap.empty())
	{
		QMessageBox::warning(NULL, "Firewall", QString::fromLocal8Bit("��ǰû�й���"));
		return;
	}

	QString savePath = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("��ѡ�񱣴�·��"), "C:\\Users\\62686\\Desktop\\FW_rule.ini", "INI(*.ini)");
	if (savePath.isEmpty())
		return;

	QFile file(savePath);
	if (file.exists())
		file.remove();

	WritePrivateProfileStringA("Firewall", "BlockAll", bBlockAll ? "1" : "0", savePath.toLocal8Bit().constData());
	WritePrivateProfileStringA("Firewall", "Status", firewall_status ? "1" : "0", savePath.toLocal8Bit().constData());

	QList<QTableWidgetSelectionRange> ranges = ui->tableWidget->selectedRanges();
	int count = ranges.count();
	for (int i = 0; i < count; i++)
	{
		int topRow = ranges.at(i).topRow();
		int bottomRow = ranges.at(i).bottomRow();
		for (int j = topRow; j <= bottomRow; j++)
		{
			std::string path(ui->tableWidget->item(j, 1)->text().toLocal8Bit());
			WritePrivateProfileStringA(path.c_str(), "name", rulesMap[path].first.c_str(), savePath.toLocal8Bit().constData());
			WritePrivateProfileStringA(path.c_str(), "isBlock", rulesMap[path].second ? "1" : "0", savePath.toLocal8Bit().constData());
		}
	}
}

void RuleDlg::exportAllRule()
{
	if (rulesMap.empty())
	{
		QMessageBox::warning(NULL, "Firewall", QString::fromLocal8Bit("��ǰû�й���"));
		return;
	}

	QString savePath = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("��ѡ�񱣴�·��"), "C:\\Users\\62686\\Desktop\\FW_rule.ini", "INI(*.ini)");
	if (savePath.isEmpty())
		return;

	QFile file(savePath);
	if (file.exists())
		file.remove();

	WritePrivateProfileStringA("Firewall", "BlockAll", bBlockAll ? "1" : "0", savePath.toLocal8Bit().constData());
	WritePrivateProfileStringA("Firewall", "Status", firewall_status ? "1" : "0", savePath.toLocal8Bit().constData());

	for (auto& rule : rulesMap)
	{
		WritePrivateProfileStringA(rule.first.c_str(), "name", rule.second.first.c_str(), savePath.toLocal8Bit().constData());
		WritePrivateProfileStringA(rule.first.c_str(), "isBlock", rule.second.second ? "1" : "0", savePath.toLocal8Bit().constData());
	}
}

void RuleDlg::saveRule()
{
	int rowCount = ui->tableWidget->rowCount();
	for (int row = 0; row < rowCount; row++)
	{
		std::string path(ui->tableWidget->item(row, 1)->text().toLocal8Bit());
		std::string section(ui->tableWidget->item(row, 0)->text().toLocal8Bit());
		bool isBlock = (ui->tableWidget->item(row, 2)->text() == QString::fromLocal8Bit("����")) ? false : true;
		//д����
		WritePrivateProfileStringA(path.c_str(), "name", section.c_str(), "C:\\Windows\\FW_rule.ini");
		WritePrivateProfileStringA(path.c_str(), "isBlock", isBlock ? "1" : "0", "C:\\Windows\\FW_rule.ini");
		//����map
		rulesMap[path] = std::pair<std::string, bool>(section, isBlock);
	}

	WritePrivateProfileStringA("Firewall", "BlockAll", bBlockAll ? "1" : "0", "C:\\Windows\\FW_rule.ini");
}

void RuleDlg::changeTheme(int style)
{
	//����ͼƬ
	QPixmap pixmap;
	if (style == 0)
	{
		pixmap.load(":/images/addRule.png");
		ui->addRuleBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/addRule_2.png");
		ui->addRuleBtn_2->setIcon(QIcon(pixmap));
		if (bBlockAll)
		{
			pixmap.load(":/images/block_all.png");
			ui->blockAllBtn->setIcon(QIcon(pixmap));
		}
		else
		{
			pixmap.load(":/images/pass_all.png");
			ui->blockAllBtn->setIcon(QIcon(pixmap));
		}
	}
	else
	{
		pixmap.load(":/images/addRule_b.png");
		ui->addRuleBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/addRule_2_b.png");
		ui->addRuleBtn_2->setIcon(QIcon(pixmap));
		if (bBlockAll)
		{
			pixmap.load(":/images/block_all_b.png");
			ui->blockAllBtn->setIcon(QIcon(pixmap));
		}
		else
		{
			pixmap.load(":/images/pass_all_b.png");
			ui->blockAllBtn->setIcon(QIcon(pixmap));
		}
	}

	//����qss
	QString styleQss;
	if (style == 0)
		styleQss = ":/qss/DarkSkin_ruleDlg.qss";
	else if (style == 1)
		styleQss = ":/qss/BrightSkin_ruleDlg.qss";

	QFile qssFile(styleQss);
	if (qssFile.open(QFile::ReadOnly))
		this->setStyleSheet(qssFile.readAll());
	qssFile.close();

	m_style = style;
}

void RuleDlg::updateFWStatus(bool status)
{
	firewall_status = status;
}

