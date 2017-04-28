#ifndef DB_HIGHSCORE_WIDGET_H
#define DB_HIGHSCORE_WIDGET_H

#include <Wt/WGlobal>
#include <Wt/WTable>

namespace DB
{
	class HighscoreWidget : public Wt::WTable
	{
	public:
		HighscoreWidget(Wt::WContainerWidget *parent = nullptr);
	};
}

#endif