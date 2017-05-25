#include "Application/GameServer.h"
#include "Application/WApplication.h"
#include "Application/WServer.h"
#include "Widgets/GameWidget.h"

#include <Wt/WLogger>
#include <Wt/WIOService>
#include <boost/thread/lock_guard.hpp>

//boost::lock_guard<boost::mutex> lock(_mutex); //lock

#define UPDATE_INTERVAL 100 //Milliseconds

namespace SM
{

	GameCell::GameCell(GameServer *server, int x, int y)
		: _server(server)
	{
		_server->_allCells[_server->_nextCellId++] = this;
		setCoordinates(x, y);
	}

	GameCell::~GameCell()
	{
		GameCellMap::iterator fItr;
		for(fItr = _server->_allCells.begin(); fItr != _server->_allCells.end(); ++fItr)
		{
			if(fItr->second == this)
				break;
		}

		if(fItr != _server->_allCells.end())
			_server->_allCells.erase(fItr);
	}

	void GameCell::move(Direction d)
	{
		if(d == Up)
			_y = _y - 1;
		else if(d == Down)
			_y = _y + 1;
		else if(d == Left)
			_x = _x - 1;
		else if(d == Right)
			_x = _x + 1;
	}

	void GameCell::setCoordinates(int x, int y)
	{
		if(!_server->isValidCoordinate(x, y))
			throw std::logic_error("Invalid coordinates");

		_x = x;
		_y = y;
	}

	SnakeNode::SnakeNode(GameServer *server, Direction direction, int x, int y)
		: GameCell(server, x, y)
	{
		setIsCollidable(true);
		setDirection(direction);
	}

	SnakeNode::SnakeNode(GameServer *server, SnakeNode *head)
		: GameCell(server, 0, 0), _head(head)
	{
		if(!_head || _head->direction() == NoChange)
			throw std::logic_error("SnakeSprite constructor with head argument requires a valid head to have a valid direction");

		//Update head's tail to this
		if(_head)
		{
			if(_head->tail())
				throw std::logic_error("Sprite already has a tail");

			_head->_tail = this;
		}

		setIsCollidable(true);
		setDirection(_head->direction());

		int x = _head->x();
		int y = _head->y();
		switch(_head->direction())
		{
		case Up: y += 1; break;
		case Down: y -= 1; break;
		case Left: x += 1; break;
		case Right: x -= 1; break;
		}

		setCoordinates(x, y);
	}

	SnakeNode::~SnakeNode()
	{
		delete _tail;
	}

	void SnakeNode::insertTail()
	{
		SnakeNode *previousTail = tail();
		_tail = nullptr; //to avoid exception
		SnakeNode *newNode = new SnakeNode(_server, this);
		newNode->_tail = previousTail;
	}

	void SnakeNode::moveSnake(bool increaseSize)
	{
		if(!isHead())
			return;

		if(increaseSize)
		{
			move(direction());
			insertTail();
		}
		else
		{
			SnakeNode *currentNode = this;
			Direction d = direction();
			do
			{
				if(currentNode->direction() != d)
				{
					Direction spriteD = currentNode->direction();
					currentNode->setDirection(d);
					d = spriteD;
				}
				currentNode->move(d);
			} while(currentNode = currentNode->tail());
		}
	}

	void SnakeNode::updateImageSrc()
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

	GameServer::GameServer(const Wt::WString &matchName, const MMClientVector &clients, Wt::Dbo::Session &session)
		: dboSession(session)
	{
		//Game's definitions
		_gamePtr = dboSession.add(Wt::Dbo::ptr<Game>(new Game()));
		_gamePtr.modify()->setName(matchName);
		_gamePtr.modify()->setRows(25);
		_gamePtr.modify()->setColumns(25);

		//Create list of players
		for(const auto &client : clients)
			_playerList.push_back(new GameClient(client));

		//Initialize Grid
		_collisionGrid.resize(_gamePtr->rows());
		for(int i = 0; i < _collisionGrid.size(); ++i)
			_collisionGrid[i].resize(_gamePtr->columns());
		for(int i = 0; i < _collisionGrid.size(); ++i)
		{
			for(int j = 0; j < _collisionGrid[i].size(); ++j)
				_collisionGrid[i][j] = nullptr;
		}
	}

	GameServer::~GameServer()
	{
		for(const auto &cell : _allCells)
			delete cell.second;
		for(const auto &client : _playerList)
			delete client;
	}

	void GameServer::initLevel()
	{
		//Snakes
		int i = 0;
		for(const auto &client : _playerList)
		{
			if(i >= 4)
				throw std::logic_error("4 players max");

			//Head
			delete client->snakeHead;

			if(!client->connected)
				continue;

			if(i == 0)
				client->snakeHead = new SnakeNode(this, Right, 4, 2);
			else if(i == 1)
				client->snakeHead = new SnakeNode(this, Left, _gamePtr->columns()-1-4, _gamePtr->rows()-1-2);
			else if(i == 2)
				client->snakeHead = new SnakeNode(this, Down, _gamePtr->columns()-1-2, 4);
			else
				client->snakeHead = new SnakeNode(this, Up, 2, _gamePtr->rows()-1-4);

			//Body and tail
			SnakeNode *body1 = new SnakeNode(this, client->snakeHead);
			SnakeNode *tail = new SnakeNode(this, body1);

			++i;
		}

		for(int i = 0; i < _foodVector.size(); ++i)
			delete _foodVector[i];

		_foodVector.resize(1);
		_foodVector[0] = new GameCell(this, _gamePtr->columns()/2, _gamePtr->rows()/2);
		_foodVector[0]->setImageLink("sprites/food.png");
	}

	void GameServer::update()
	{
		boost::lock_guard<boost::mutex> lock(_mutex);

		if(_gamePtr->state() != Game::InProgress)
			return;

		//Schedule next update
		SERVER->ioService().schedule(UPDATE_INTERVAL, boost::bind(&GameServer::update, this));

		//Update snake's direction
		for(const auto &client : _playerList)
		{
			if(!client->snakeHead)
				continue;

			Direction prevHeadDirection = client->snakeHead->direction();
			if(client->nextDirection != NoChange)
			{
				if(!isOppositeDirection(client->snakeHead->direction(), client->nextDirection))
					client->snakeHead->setDirection(client->nextDirection);
				client->nextDirection = NoChange;
			}
		}

		//Check for collision
		for(const auto &client : _playerList)
		{
			if(!client->snakeHead)
				continue;

			bool collided = false;

			if(client->snakeHead->direction() == Up)
			{
				if(client->snakeHead->y() <= 0)
					collided = true;
				else if(_collisionGrid[client->snakeHead->x()][client->snakeHead->y() - 1])
					collided = true;
			}
			else if(client->snakeHead->direction() == Down)
			{
				if(client->snakeHead->y() >= _gamePtr->rows() - 1)
					collided = true;
				else if(_collisionGrid[client->snakeHead->x()][client->snakeHead->y() + 1])
					collided = true;
			}
			else if(client->snakeHead->direction() == Left)
			{
				if(client->snakeHead->x() <= 0)
					collided = true;
				else if(_collisionGrid[client->snakeHead->x() - 1][client->snakeHead->y()])
					collided = true;
			}
			else if(client->snakeHead->direction() == Right)
			{
				if(client->snakeHead->x() >= _gamePtr->columns() - 1)
					collided = true;
				else if(_collisionGrid[client->snakeHead->x() + 1][client->snakeHead->y()])
					collided = true;
			}

			if(collided)
			{
				snakeCollided(client);
				if(_gamePtr->state() != Game::InProgress)
					return;
			}
		}

		//Apply movement
		for(const auto &client : _playerList)
		{
			if(!client->snakeHead)
				continue;

			client->snakeHead->moveSnake(client->justAte);
			client->justAte = false;
		}

		//Update cell image sources
		for(const auto &cell : _allCells)
			cell.second->updateImageSrc();

		//Update grid
		for(int x = 0; x < _gamePtr->columns(); ++x)
		{
			for(int y = 0; y < _gamePtr->rows(); ++y)
				_collisionGrid[x][y] = nullptr;
		}
		for(const auto &cell : _allCells)
		{
			SnakeNode *node = dynamic_cast<SnakeNode*>(cell.second);
			if(!node)
				continue;

			if(node->isCollidable())
				_collisionGrid[node->x()][node->y()] = node;
		}

		//Food collection
		for(const auto &client : _playerList)
		{
			if(!client->snakeHead)
				continue;

			for(const auto &food : _foodVector)
			{
				if(client->snakeHead->x() == food->x() && client->snakeHead->y() == food->y())
				{
					client->justAte = true;

					std::vector<std::pair<int, int>> availPos;
					for(int i = 0; i < _gamePtr->columns(); i++)
					{
						for(int j = 0; j < _gamePtr->rows(); j++)
						{
							if(_collisionGrid[i][j] == nullptr)
								availPos.push_back(std::make_pair(i, j));
						}
					}

					int randdd = rand() % availPos.size();
					food->setCoordinates(availPos[randdd].first, availPos[randdd].second);
				}
			}
		}

		//Post updates to clients
		for(const auto &client : _playerList)
		{
			SERVER->post(client->sessionId, boost::bind(&GameWidget::_update, _allCells));
		}
	}

	void GameServer::restart()
	{
		boost::lock_guard<boost::mutex> lock(_mutex);
		Wt::Dbo::Transaction t(dboSession);
		_gamePtr.modify()->setState(Game::InProgress);
		t.commit();

		for(const auto &client : _playerList)
			client->nextDirection = NoChange;

		initLevel();
		SERVER->ioService().schedule(UPDATE_INTERVAL, boost::bind(&GameServer::update, this));
	}

	void GameServer::snakeCollided(GameClient *client)
	{
		size_t count = 0;
		
		for(const auto &client : _playerList)
		{
			if(client->snakeHead)
				++count;
		}

		if(count > 2)
		{
			delete client->snakeHead;
			client->snakeHead = nullptr;
		}
		else
			gameOver();
	}

	void GameServer::gameOver()
	{
		Wt::Dbo::Transaction t(dboSession);
		_gamePtr.modify()->setState(Game::GameOver);
		t.commit();

		SERVER->ioService().schedule(3000, boost::bind(&GameServer::restart, this));
	}

	void GameServer::connect()
	{
		boost::lock_guard<boost::mutex> lock(_mutex);

		WApplication *app = APP;
		_connect(app->sessionId());
	}

	void GameServer::disconnect()
	{
		boost::lock_guard<boost::mutex> lock(_mutex);

		WApplication *app = APP;
		_disconnect(app->sessionId());
	}

	void GameServer::_connect(const std::string &sessionId)
	{
		GameClient *client = clientBySessionId(sessionId);
		if(!client || client->connected)
			return;

		client->connected = true;

		//Check if all are connected
		bool allConnected = true;
		for(const auto &client : _playerList)
		{
			if(!client->connected)
			{
				allConnected = false;
				break;
			}
		}

		//Init timer if every one is connected
		if(allConnected)
		{
			initLevel();
			SERVER->ioService().schedule(UPDATE_INTERVAL, boost::bind(&GameServer::update, this));
		}

		//Notify client that it is connected
		SERVER->post(sessionId, boost::bind(&GameWidget::_connected, _playerList, _allCells));
	}

	void GameServer::_disconnect(const std::string &sessionId)
	{
		GameClient *client = clientBySessionId(sessionId);
		if(!client || !client->connected)
			return;

		//Mark disconnected and remove snakehead
		client->connected = false;
		delete client->snakeHead;
		client->snakeHead = nullptr;

		//Check if all are disconnected
		bool allDisconnected = true;
		for(const auto &client : _playerList)
		{
			if(client->connected)
			{
				allDisconnected = false;
				break;
			}
		}

		if(allDisconnected)
		{
			Wt::Dbo::Transaction t(dboSession);
			_gamePtr.modify()->setState(Game::Disconnected);
			t.commit();
			delete this;
		}
	}

	void GameServer::nextDirectionUpdated(Direction d)
	{
		boost::lock_guard<boost::mutex> lock(_mutex);

		WApplication *app = APP;
		_nextDirectionUpdated(app->sessionId(), d);
	}

	void GameServer::_nextDirectionUpdated(const std::string &sessionId, Direction d)
	{
		GameClient *client = clientBySessionId(sessionId);
		if(!client || !client->connected)
			return;

		client->nextDirection = d;
	}

	GameClient *GameServer::clientBySessionId(const std::string &sessionId) const
	{
		for(const auto &client : _playerList)
		{
			if(client->sessionId == sessionId)
				return client;
		}
		return nullptr;
	}

	GameClient *GameServer::clientByAuthInfoId(long long authInfoId) const
	{
		for(const auto &client : _playerList)
		{
			if(client->authInfoId == authInfoId)
				return client;
		}
		return nullptr;
	}

}

