#ifndef NETSPEEDWORK_H
#define NETSPEEDWORK_H

#include <QObject>
#include <QThread>
#include <QDebug>

#include <Winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <Iphlpapi.h>


class NetSpeedWork : public QObject
{
	Q_OBJECT

public:
	explicit NetSpeedWork(QObject* parent = nullptr);
	~NetSpeedWork() {}

	void terminal();

private:
	bool isTerminated;

public slots:
	void work();

signals:
	void workDown(double up, double down);

};

#endif // NETSPEEDWORK_H
