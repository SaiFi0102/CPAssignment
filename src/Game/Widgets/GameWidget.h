#ifndef DB_GAME_WIDGET_H
#define DB_GAME_WIDGET_H

#include <Wt/WGlobal>
#include <Wt/WContainerWidget>

namespace DB
{
	class GameWidget : public Wt::WContainerWidget
	{
	public:
		GameWidget(Wt::WContainerWidget *parent = nullptr);
		//virtual ~GameWidget();

	protected:
		void handleKeyStateChanged(Wt::Key key, bool newState);
		void handleTimer();

		Wt::WText *_test = nullptr;
		Wt::WTimer *_timer = nullptr;
	};
}

#endif
