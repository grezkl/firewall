#include "firewall.h"
#include <QApplication>
#include <QWindow>
#include <QLocalSocket>
#include <QLocalServer>
#include <QMessagebox>

int main(int argc, char* argv[])
{
	//实现单例，能连接到Server则意味程序已经启动了
	QLocalSocket socket;
	socket.connectToServer("Firewall");
	if (socket.waitForConnected(1000))
	{
		MessageBoxA(NULL, "            防火墙已启动！\n        只允许启动一个进程！","Firewall", MB_OKCANCEL | MB_ICONWARNING);
		return -1;
	}

	QLocalServer server;
	if (!server.listen("Firewall"))
	{
		//如果监听失败，查看是否是端口被崩溃的残留进程占用
		if (server.serverError() == QAbstractSocket::AddressInUseError)
		{
			QLocalServer::removeServer("Firewall");
			server.listen("Firewall");
		}
	}

	Q_INIT_RESOURCE(firewall);

	QApplication a(argc, argv);
	Firewall w;
	w.setWindowIcon(QIcon(":/images/firewall_icon.ico"));
	w.show();

	return a.exec();
}
