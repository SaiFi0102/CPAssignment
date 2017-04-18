#include "Widgets/GameWidget.h"
#include "Application/WApplication.h"

#include <Wt/WText>
#include <Wt/WTimer>

namespace DB
{
	GameWidget::GameWidget(Wt::WContainerWidget *parent)
		: Wt::WContainerWidget(parent)
	{
		setStyleClass("game");

		WApplication *app = WApplication::instance();
		app->keyStateUpdated().connect(this, &GameWidget::handleKeyStateChanged);

		_test = new Wt::WText("Test", this);
		_test->setStyleClass("sprite");
		_test->setPositionScheme(Wt::Relative);

		_timer = new Wt::WTimer(this);
		_timer->setInterval(10);
		_timer->timeout().connect(this, &GameWidget::handleTimer);
	}

	void GameWidget::handleKeyStateChanged(Wt::Key key, bool newState)
	{
		WApplication *app = WApplication::instance();
		if(!app->keyState().anyEnabled())
		{
			_timer->stop();
		}
		else if(!_timer->isActive())
		{
			_timer->start();
			handleTimer();
		}
	}
	void GameWidget::handleTimer()
	{
		WApplication *app = WApplication::instance();

		if(app->keyState().arrowUp)
			_test->setOffsets(Wt::WLength(_test->offset(Wt::Top).value() - 2), Wt::Top);
		if(app->keyState().arrowDown)
			_test->setOffsets(Wt::WLength(_test->offset(Wt::Top).value() + 2), Wt::Top);
		if(app->keyState().arrowLeft)
			_test->setOffsets(Wt::WLength(_test->offset(Wt::Left).value() - 2), Wt::Left);
		if(app->keyState().arrowRight)
			_test->setOffsets(Wt::WLength(_test->offset(Wt::Left).value() + 2), Wt::Left);
	}
}