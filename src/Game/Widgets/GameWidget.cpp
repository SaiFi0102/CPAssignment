#include "Widgets/GameWidget.h"

namespace DB
{
	GameWidget::GameWidget(Wt::WContainerWidget *parent)
		: Wt::WContainerWidget(parent)
	{
		setStyleClass("gamewidget");
	}
}