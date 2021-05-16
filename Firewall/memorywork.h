#ifndef MEMORYWORK_H
#define MEMORYWORK_H

#include <QObject>
#include <windows.h>
#include <QThread>

class MemoryWork : public QObject
{
	Q_OBJECT
public:
	explicit MemoryWork(QObject *parent = nullptr);
	~MemoryWork() {}

	void terminal();

private:
	bool isTerminated;

public slots:
	void work();

signals:
	void workDown(int usage);
};

#endif // MEMORYWORK_H
