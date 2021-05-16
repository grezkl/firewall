#include "firewall.h"
#include <QApplication>
#include <QWindow>
#include <QLocalSocket>
#include <QLocalServer>
#include <QMessagebox>

int main(int argc, char* argv[])
{
	//ʵ�ֵ����������ӵ�Server����ζ�����Ѿ�������
	QLocalSocket socket;
	socket.connectToServer("Firewall");
	if (socket.waitForConnected(1000))
	{
		MessageBoxA(NULL, "            ����ǽ��������\n        ֻ��������һ�����̣�","Firewall", MB_OKCANCEL | MB_ICONWARNING);
		return -1;
	}

	QLocalServer server;
	if (!server.listen("Firewall"))
	{
		//�������ʧ�ܣ��鿴�Ƿ��Ƕ˿ڱ������Ĳ�������ռ��
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
