#ifndef BEHAVIORBOX_H
#define BEHAVIORBOX_H

#include <QComboBox>
#include <QAbstractItemView>
#include <QModelIndex>

class BehaviorBox :public QComboBox
{
	Q_OBJECT

public:
	explicit BehaviorBox(int row, int column, QWidget* parent = nullptr);
	~BehaviorBox();

protected:
	virtual void hidePopup();

private:
	int row, column;

signals:
	void activated(int row, int column, int index);

};

#endif // BEHAVIORBOX_H
