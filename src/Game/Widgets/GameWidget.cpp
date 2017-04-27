#include "Widgets/GameWidget.h"
#include "Application/WApplication.h"

#include <Wt/WText>
#include <Wt/WTimer>
#include <Wt/WImage>
#include <Wt/WString>
#include <Wt/WMemoryResource>

namespace DB
{
	bool isOppositeDirection(Direction d1, Direction d2)
	{
		return (d1 == Up && d2 == Down) || (d1 == Down && d2 == Up) || (d1 == Left && d2 == Right) || (d1 == Right && d2 == Left);
	}

	bool isValidCoordinate(int x, int y)
	{
		return x >= 0 && x < GRID_SIZE_X && y >= 0 && y < GRID_SIZE_Y;
	}

	GameSprite::GameSprite(GameWidget *parent, int x, int y, GameSprite *head)
		: Wt::WImage(parent), _head(head), _x(x), _y(y)
	{
		if(!isValidCoordinate(_x, _y))
			throw std::logic_error("Invalid coordinates");

		//Update head's tail to this
		if(_head)
		{
			if(_head->tail())
				throw std::logic_error("Sprite already has a tail");

			_head->_tail = this;
		}

		setPositionScheme(Wt::Absolute);
		resize(CELL_WIDTH, CELL_HEIGHT);

	}

	GameSprite::~GameSprite()
	{
		//This will delete all tail sprites
		delete _tail;
	}

	void GameSprite::move(Direction d)
	{
		if (d == Up)
			_y = _y - 1;
		else if (d == Down)
			_y = _y + 1;
		else if (d == Left)
			_x = _x - 1;
		else if (d == Right)
			_x = _x + 1;
	}

	SnakeSprite::SnakeSprite(GameWidget *parent, Direction direction, int x, int y)
		: GameSprite(parent, x, y)
	{
		setIsCollidable(true);
		setDirection(direction);
	}

	SnakeSprite::SnakeSprite(GameWidget *parent, SnakeSprite *head)
		: GameSprite(parent, 0, 0, head)
	{
		if(!head || head->direction() == NoChange)
			throw std::logic_error("SnakeSprite constructor with head argument requires a valid head to have a valid direction");

		setIsCollidable(true);
		setDirection(head->direction());
		
		_x = head->x();
		_y = head->y();
		switch(head->direction())
		{
		case Up: _y += 1; break;
		case Down: _y -= 1; break;
		case Left: _x += 1; break;
		case Right: _x -= 1; break;
		}

		if(!isValidCoordinate(_x, _y))
			throw std::logic_error("SnakeSprite constructor with head argument requires enough space for the sprite to be placed behind the head");
	}

	void SnakeSprite::insertTail()
	{
		GameSprite *previousTail = tail();
		_tail = nullptr; //to avoid exception
		SnakeSprite *newSprite = new SnakeSprite(static_cast<GameWidget*>(parent()), this);
		newSprite->_tail = previousTail;
	}

	void SnakeSprite::updateImageSrc()
	{
		if(direction() == NoChange)
			throw std::runtime_error("SnakeSprite's direction must not be NoChange");

		//This is the head
		if(isHead())
		{
			switch(direction())
			{
			case Up: setImageLink("sprites/head-up.png"); break;
			case Down: setImageLink("sprites/head-down.png"); break;
			case Left: setImageLink("sprites/head-left.png"); break;
			case Right: setImageLink("sprites/head-right.png"); break;
			}
			
		}
		else if(isTail()) //This is the tail
		{
			switch(direction())
			{
			case Up: setImageLink("sprites/tail-up.png"); break;
			case Down: setImageLink("sprites/tail-down.png"); break;
			case Left: setImageLink("sprites/tail-left.png"); break;
			case Right: setImageLink("sprites/tail-right.png"); break;
			}
		}
		else
		{
			//This is the body
			if(direction() == tail()->direction())
			{
				switch(direction())
				{
				case Up:
				case Down:
					setImageLink("sprites/body-v.png");
					break;
				case Left:
				case Right:
					setImageLink("sprites/body-h.png");
					break;
				}
			}
			else //This is a turning point
			{
				if((direction() == Up && tail()->direction() == Left) || (direction() == Right && tail()->direction() == Down))
					setImageLink("sprites/corner-down-left.png");
				else if((direction() == Up && tail()->direction() == Right) || (direction() == Left && tail()->direction() == Down))
					setImageLink("sprites/corner-down-right.png");
				else if((direction() == Right && tail()->direction() == Up) || (direction() == Down && tail()->direction() == Left))
					setImageLink("sprites/corner-up-left.png");
				else
					setImageLink("sprites/corner-up-right.png");
			}
		}
	}

	GameWidget::GameWidget(Wt::WContainerWidget *parent)
		: Wt::WContainerWidget(parent)
	{
		setStyleClass("game");
		resize(CELL_WIDTH*GRID_SIZE_X, CELL_HEIGHT*GRID_SIZE_Y);

		WApplication *app = WApplication::instance();
		app->keyStateUpdated().connect(this, &GameWidget::handleKeyState);

		//Init grid
		for(int i = 0; i < GRID_SIZE_X; ++i)
		{
			for(int j = 0; j < GRID_SIZE_Y; ++j)
			{
				_grid[i][j] = nullptr;
			}
		}

		//Snake
		_head = new SnakeSprite(this, Right, 8, 2);

		SnakeSprite *body1 = new SnakeSprite(this, _head);
		SnakeSprite *body2 = new SnakeSprite(this, body1);
		SnakeSprite *body3 = new SnakeSprite(this, body2);
		SnakeSprite *body4 = new SnakeSprite(this, body3);
		SnakeSprite *body5 = new SnakeSprite(this, body4);
		SnakeSprite *tail = new SnakeSprite(this, body5);

		Food = new GameSprite(this, 14,14);
		Food->setImageLink("sprites/food.png");
		//Init timer
		_timer = new Wt::WTimer(this);
		_timer->setInterval(100);
		_timer->timeout().connect(this, &GameWidget::update);
		_timer->start();
		update();
	}

	void GameWidget::handleKeyState(Wt::Key key, bool state)
	{
		if(!state)
			return;

		if(key == Wt::Key_Up)
			_nextDirection = Up;
		else if(key == Wt::Key_Down)
			_nextDirection = Down;
		else if(key == Wt::Key_Left)
			_nextDirection = Left;
		else if(key == Wt::Key_Right)
			_nextDirection = Right;
	}

	void GameWidget::update()
	{
		WApplication *app = WApplication::instance();

		//Update snake's direction
		Direction prevHeadDirection = headSprite()->direction();
		if (_nextDirection != NoChange)
		{
			if (!isOppositeDirection(headSprite()->direction(), _nextDirection))
				headSprite()->setDirection(_nextDirection);
			_nextDirection = NoChange;
		}

		//Check for collision
		bool collision = false;

		if (headSprite()->direction() == Up)
		{
			if (headSprite()->y() <= 0)
				collision = true;
			else if (_grid[headSprite()->x()][headSprite()->y() - 1])
				collision = true;
		}
		else if (headSprite()->direction() == Down)
		{
			if (headSprite()->y() >= GRID_SIZE_Y - 1)
				collision = true;
			else if (_grid[headSprite()->x()][headSprite()->y() + 1])
				collision = true;
		}
		else if (headSprite()->direction() == Left)
		{
			if (headSprite()->x() <= 0)
				collision = true;
			else if (_grid[headSprite()->x() - 1][headSprite()->y()])
				collision = true;
		}
		else if (headSprite()->direction() == Right)
		{
			if (headSprite()->x() >= GRID_SIZE_X - 1)
				collision = true;
			else if (_grid[headSprite()->x() + 1][headSprite()->y()])
				collision = true;
		}

		if (collision)
		{
			gameOver();
			return;
		}

		if (_justAte)
		{
			_head->move(_head->direction());
			_head->insertTail();
			_justAte = false;
		}
		else
		{
			GameSprite *currentSprite = _head;
			Direction d = headSprite()->direction();
			do
			{
				if (currentSprite->direction() != d)
				{
					Direction spriteD = currentSprite->direction();
					currentSprite->setDirection(d);
					d = spriteD;
				}
				currentSprite->move(d);
			} while (currentSprite = currentSprite->tail());
		}

		//Food collection
		if (_head->x() == Food->x() && _head->y() == Food->y())
		{
			_justAte = true;
			Food->_x = 20;
			Food->_y = 20;
		}
		
		//Re init grid
		for(int x = 0; x < GRID_SIZE_X; ++x)
		{
			for(int y = 0; y < GRID_SIZE_Y; ++y)
			{
				_grid[x][y] = nullptr;
			}
		}
		
		//Set position and update grid
		int wCount = count();
		for (int i = 0; i < wCount; ++i)
		{
			GameSprite *sprite = dynamic_cast<GameSprite*>(widget(i));
			if (!sprite)
				continue;

			if(sprite->isCollidable())
				_grid[sprite->x()][sprite->y()] = sprite;

			sprite->setOffsets(sprite->x() * CELL_WIDTH, Wt::Left);
			sprite->setOffsets(sprite->y() * CELL_HEIGHT, Wt::Top);
			sprite->updateImageSrc();
		}

	}
	void GameWidget::gameOver()
	{
		_timer->stop();
	}
}