#include "Widgets/GameWidget.h"
#include <Wt/WText>

namespace DB
{
	GameWidget::GameWidget(Wt::WContainerWidget *parent)
		: Wt::WContainerWidget(parent)
	{
		setStyleClass("gamewidget");
		new Wt::WText("Test", this);
	}
}