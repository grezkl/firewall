#include "behaviorbox.h"

BehaviorBox::BehaviorBox(int row, int column, QWidget* parent)
	:QComboBox(parent), row(row), column(column)
{

}

BehaviorBox::~BehaviorBox()
{

}

void BehaviorBox::hidePopup()
{
	QComboBox::hidePopup();
	int index = view()->currentIndex().row();
	emit activated(row, column ,index);
}
