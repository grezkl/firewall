#include "firewall.h"
#include "ui_firewall.h"

Firewall::Firewall(QWidget* parent) :
	QWidget(parent), ui(new Ui::Firewall), m_style(0)
{
	ui->setupUi(this);

	if (!QFile::exists(".\\FW_config.ini"))
	{
		WritePrivateProfileStringA("Firewall", "Theme", "0", ".\\FW_config.ini");
	}
	else
		m_style = GetPrivateProfileIntA("Firewall", "Theme", 0, ".\\FW_config.ini");

	if (!QFile::exists("C:\\Windows\\FW_rule.ini"))
	{
		WritePrivateProfileStringA("Firewall", "BlockAll", "0", "C:\\Windows\\FW_rule.ini");
		WritePrivateProfileStringA("Firewall", "Status", "0", "C:\\Windows\\FW_rule.ini");
	}

	//加载图片
	QPixmap pixmap;
	if (m_style == 0)
	{
		pixmap.load(":/images/logo.png");
		ui->label->setPixmap(pixmap);
		pixmap.load(":/images/overview.png");
		ui->infoBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/rule.png");
		ui->ruleBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/log.png");
		ui->logBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/skin.png");
		ui->menuBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/minimize.png");
		ui->minBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/close3.png");
		ui->exitBtn->setIcon(QIcon(pixmap));
	}
	else
	{
		pixmap.load(":/images/logo_b.png");
		ui->label->setPixmap(pixmap);
		pixmap.load(":/images/overview_b.png");
		ui->infoBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/rule_b.png");
		ui->ruleBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/log_b.png");
		ui->logBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/skin_b.png");
		ui->menuBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/minimize_b.png");
		ui->minBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/close3_b.png");
		ui->exitBtn->setIcon(QIcon(pixmap));
	}

	//加载qss
	QString styleQss;
	if (m_style == 0)
		styleQss = ":/qss/DarkSkin_Firewall.qss";
	else if(m_style == 1)
		styleQss = ":/qss/BrightSkin_Firewall.qss";

	QFile qssFile(styleQss);
	if (qssFile.open(QFile::ReadOnly))
		this->setStyleSheet(qssFile.readAll());
	qssFile.close();

	//隐藏标题栏
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	bPressed = false;

	//隐藏tab标签
	ui->tabWidget->tabBar()->hide();

	//添加dlg
	InfoDlg* infoDlg = new InfoDlg(m_style, this);
	LogDlg* logDlg = new LogDlg(m_style, this);
	RuleDlg* ruleDlg = new RuleDlg(m_style, this);

	//更新防火墙状态
	connect(infoDlg, &InfoDlg::updateFWStatus, logDlg, &LogDlg::updateFWStatus);
	connect(infoDlg, &InfoDlg::updateFWStatus, ruleDlg, &RuleDlg::updateFWStatus);

	//更新日志信号
	connect(infoDlg, &InfoDlg::updateSelfLog, logDlg, &LogDlg::updateSelfLog);
	connect(ruleDlg, &RuleDlg::updateSelfLog, logDlg, &LogDlg::updateSelfLog);

	//改变皮肤主题信号
	connect(this, &Firewall::changeStyle, infoDlg, &InfoDlg::changeTheme);
	connect(this, &Firewall::changeStyle, logDlg, &LogDlg::changeTheme);
	connect(this, &Firewall::changeStyle, ruleDlg, &RuleDlg::changeTheme);

	this->ui->tabWidget->insertTab(0, infoDlg, "info");
	this->ui->tabWidget->insertTab(1, ruleDlg, "rule");
	this->ui->tabWidget->insertTab(2, logDlg, "log");

	//设置默认info页
	this->ui->tabWidget->setCurrentIndex(0);


	//系统托盘图标
	m_trayIcon = new QSystemTrayIcon(this);
	QIcon icon(":/images/firewall_icon.ico");
	m_trayIcon->setIcon(icon);
	m_trayIcon->setToolTip(QString::fromLocal8Bit("个人防火墙"));
	//托盘图标点击事件
	connect(m_trayIcon, &QSystemTrayIcon::activated, this,
		[=](QSystemTrayIcon::ActivationReason reason)
		{
			if (reason == QSystemTrayIcon::DoubleClick)
			{
				this->show();
			}
		});

	//托盘图标菜单
	m_trayMenu = new QMenu(this);
	QAction* m_showAction = new QAction(QString::fromLocal8Bit("显示主界面"), m_trayMenu);
	QAction* m_exitAction = new QAction(QString::fromLocal8Bit("退出"), m_trayMenu);
	m_trayMenu->addAction(m_showAction);
	m_trayMenu->addSeparator();
	m_trayMenu->addAction(m_exitAction);
	m_trayIcon->setContextMenu(m_trayMenu);

	connect(m_showAction, &QAction::triggered, this, [=] {this->show(); });
	connect(m_exitAction, &QAction::triggered, this, [=] {qApp->quit(); });

	//换皮肤菜单
	m_skinMenu = new QMenu(this);
	QAction* pDarkSkin = new QAction(QString::fromLocal8Bit("炫酷黑"));
	QAction* pBrightSkin = new QAction(QString::fromLocal8Bit("优雅白"));
	pDarkSkin->setData(0);
	pBrightSkin->setData(1);
	m_skinMenu->addAction(pDarkSkin);
	m_skinMenu->addAction(pBrightSkin);
	m_skinMenu->setObjectName("m_skinMenu");

	connect(m_skinMenu, &QMenu::triggered, this, &Firewall::changeTheme);
}

Firewall::~Firewall()
{
	delete m_trayIcon;
	delete m_trayMenu;
	delete m_skinMenu;
	delete ui;
}

/***************************************/
/***************重写拖动窗口**************/
void Firewall::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		bPressed = true;
		ptPressed = e->globalPos() - pos();//鼠标位置到窗口左上角的距离
	}
}

void Firewall::keyPressEvent(QKeyEvent* e)
{
	if (e->key() == Qt::Key_F5)
	{
		QMessageBox::information(this, "Firewall", 
			QString::fromLocal8Bit("作者:\n华南农业大学  数学与信息学院\n2017级工业工程专业  韦昌港"));
	}
}

void Firewall::mouseMoveEvent(QMouseEvent* e)
{
	if (bPressed && (e->buttons() == Qt::LeftButton))
	{
		move(e->globalPos() - ptPressed);
		ptPressed = e->globalPos() - pos();
	}
}

void Firewall::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		bPressed = false;
	}
}
/***************************************/
/***************************************/

void Firewall::on_exitBtn_clicked()
{
	qApp->quit();
}

void Firewall::on_infoBtn_clicked()
{
	this->ui->tabWidget->setCurrentIndex(0);
}

void Firewall::on_ruleBtn_clicked()
{
	this->ui->tabWidget->setCurrentIndex(1);
}

void Firewall::on_logBtn_clicked()
{
	this->ui->tabWidget->setCurrentIndex(2);
}

void Firewall::on_minBtn_clicked()
{
	this->hide();
	m_trayIcon->show();
}

void Firewall::on_menuBtn_clicked()
{
	m_skinMenu->popup(ui->menuBtn->mapToGlobal(QPoint(0, ui->menuBtn->height())));
}

void Firewall::changeTheme(QAction* action)
{
	switch (action->data().toInt())
	{
	case 0:
	{
		QString styleQss;
		styleQss = ":/qss/DarkSkin_Firewall.qss";
		QFile qssFile(styleQss);
		if (qssFile.open(QFile::ReadOnly))
			this->setStyleSheet(qssFile.readAll());
		qssFile.close();
		
		WritePrivateProfileStringA("Firewall", "Theme", "0", ".\\FW_config.ini");
		emit changeStyle(0);

		//加载图片
		QPixmap pixmap;
		pixmap.load(":/images/logo.png");
		ui->label->setPixmap(pixmap);
		pixmap.load(":/images/overview.png");
		ui->infoBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/rule.png");
		ui->ruleBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/log.png");
		ui->logBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/skin.png");
		ui->menuBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/minimize.png");
		ui->minBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/close3.png");
		ui->exitBtn->setIcon(QIcon(pixmap));
	}
	break;
	case 1:
	{
		QString styleQss;
		styleQss = ":/qss/BrightSkin_Firewall.qss";
		QFile qssFile(styleQss);
		if (qssFile.open(QFile::ReadOnly))
			this->setStyleSheet(qssFile.readAll());
		qssFile.close();

		WritePrivateProfileStringA("Firewall", "Theme", "1", ".\\FW_config.ini");
		emit changeStyle(1);

		//加载图片
		QPixmap pixmap;
		pixmap.load(":/images/logo_b.png");
		ui->label->setPixmap(pixmap);
		pixmap.load(":/images/overview_b.png");
		ui->infoBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/rule_b.png");
		ui->ruleBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/log_b.png");
		ui->logBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/skin_b.png");
		ui->menuBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/minimize_b.png");
		ui->minBtn->setIcon(QIcon(pixmap));
		pixmap.load(":/images/close3_b.png");
		ui->exitBtn->setIcon(QIcon(pixmap));
	}
	break;
	default:
		break;
	}
}
