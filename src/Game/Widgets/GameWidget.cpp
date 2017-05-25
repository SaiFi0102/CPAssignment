#include "Widgets/GameWidget.h"
#include "Application/WApplication.h"

#include <Wt/WImage>
#include <Wt/WString>

namespace SM
{
	GameWidget::GameWidget(GameServer *server, Wt::WContainerWidget *parent)
		: Wt::WContainerWidget(parent), _server(server)
	{
		setStyleClass("game");
		_server->connect();
	}

	GameWidget::~GameWidget()
	{
		_server->disconnect();
	}

	void GameWidget::handleKeyState(Wt::Key key, bool state)
	{
		if(!state)
			return;

		Direction nextDirection = NoChange;
		if(key == Wt::Key_Up)
			nextDirection = Up;
		else if(key == Wt::Key_Down)
			nextDirection = Down;
		else if(key == Wt::Key_Left)
			nextDirection = Left;
		else if(key == Wt::Key_Right)
			nextDirection = Right;

		if(nextDirection != NoChange)
			_server->nextDirectionUpdated(nextDirection);
	}

	void GameWidget::_connected(const GameClientVector & clients, const GameCellMap & cellsMap)
	{
		WApplication *app = APP;
		GameWidget *w = app->gameWidget();
		if(!w)
			return;

		w->_isConnected = true;
		w->resize(CELL_WIDTH*w->_server->gamePtr()->columns(), CELL_HEIGHT*w->_server->gamePtr()->rows());
		app->keyStateUpdated().connect(w, &GameWidget::handleKeyState);

		_update(cellsMap);
	}

	void GameWidget::_update(const GameCellMap & cellsMap)
	{
		WApplication *app = APP;
		GameWidget *w = app->gameWidget();
		if(!w || !w->_isConnected)
			return;

		//Update from server's state
		for(const auto &c : cellsMap)
		{
			GameCell *cell = c.second;
			Wt::WImage *sprite;

			auto spriteItr = w->_spriteMap.find(c.first);
			if(spriteItr == w->_spriteMap.end())
			{
				spriteItr = w->_spriteMap.insert(std::make_pair(c.first, new Wt::WImage(w))).first;
				sprite = spriteItr->second;
				sprite->setPositionScheme(Wt::Absolute);
				sprite->resize(CELL_WIDTH, CELL_HEIGHT);
			}
			else
				sprite = spriteItr->second;
			

			sprite->setOffsets(cell->x() * CELL_WIDTH, Wt::Left);
			sprite->setOffsets(cell->y() * CELL_HEIGHT, Wt::Top);
			sprite->setImageLink(cell->imageLink());
		}

		//Check if any cell was removed
		for(SpriteMap::iterator itr = w->_spriteMap.begin(); itr != w->_spriteMap.end();)
		{
			//If client sprite doesn't have a server cell representation then it means it has been removed
			if(cellsMap.find(itr->first) == cellsMap.end())
			{
				delete itr->second;
				w->_spriteMap.erase(itr++);
			}
			else
				++itr;
		}

		app->triggerUpdate();
	}
}