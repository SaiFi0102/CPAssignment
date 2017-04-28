#include "Widgets/HighscoreWidget.h"
#include <Wt/WString>

namespace DB
{
	HighscoreWidget::HighscoreWidget(Wt::WContainerWidget *parent)
		: Wt::WTable(parent)
	{
	
		insertColumn(0);
		insertColumn(1);
		insertColumn(2);
		insertColumn(3);
			
			for(int j = 0; j < 11; ++j)
			{
				insertRow(j);
			}
	
		

	}
}
