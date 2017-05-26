#ifndef SM_GAMESERVER_H
#define SM_GAMESERVER_H

#include "Application/MatchServer.h"
#include "Dbo/Dbos.h"

#include <Wt/WLink>
#include <Wt/WString>
#include <Wt/Dbo/Session>
#include <unordered_map>
#include <boost/thread/mutex.hpp>

namespace SM
{
	class GameServer;
	struct GameClient;
	class GameCell;
	class SnakeNode;
	typedef std::vector<GameClient*> GameClientVector;
	typedef std::unordered_map<int, GameCell*> GameCellMap;

	enum Direction
	{
		NoChange,
		Up,
		Down,
		Left,
		Right
	};

	struct GameClient : public Client
	{
		GameClient(Client *o) : Client(*o) { }
		bool connected = false;

		Direction nextDirection = NoChange;
		SnakeNode *snakeHead = nullptr;
		bool justAte = false;
	};

	class GameCell
	{
	public:
		GameCell(GameServer *server, int x, int y);
		virtual ~GameCell();

		Direction direction() const { return _direction; }
		void setDirection(Direction direction) { _direction = direction; }

		bool isCollidable() const { return _collidable; }
		void setIsCollidable(bool collidable) { _collidable = collidable; }

		Wt::WLink imageLink() const { return _imgSrc; }
		void setImageLink(const Wt::WLink &src) { _imgSrc = src; }

		int x() const { return _x; }
		int y() const { return _y; }

		void setCoordinates(int x, int y);
		void move(Direction d);
		virtual void updateImageSrc() { }

	protected:
		GameServer *_server;
		int _x, _y;

	private:
		Wt::WLink _imgSrc;
		Direction _direction = NoChange;
		bool _collidable = false;
	};

	class SnakeNode : public GameCell
	{
	public:
		SnakeNode(GameServer *parent, Direction direction, int x, int y);
		SnakeNode(GameServer *parent, SnakeNode *head);
		virtual ~SnakeNode() override;

		SnakeNode *head() const { return _head; }
		SnakeNode *tail() const { return _tail; }

		bool isHead() const { return !head(); }
		bool isTail() const { return !tail(); }

		void insertTail();

		void moveSnake(bool increaseSize);
		virtual void updateImageSrc() override;

	protected:
		SnakeNode *_head = nullptr;
		SnakeNode *_tail = nullptr;
	};

	class GameServer
	{
	public:
		GameServer(const Wt::WString &matchName, const MMClientVector &clients, Wt::Dbo::Session &session);
		~GameServer();

		//Interface for client
		void connect();
		void disconnect();
		void nextDirectionUpdated(Direction d);
		Wt::Dbo::ptr<Game> gamePtr() const { return _gamePtr; }

	protected:
		void initLevel();
		void update();
		void _update();
		void restart();
		void snakeCollided(GameClient *client);
		void gameOver();

		void _connect(const std::string &sessionId);
		void _disconnect(const std::string &sessionId);
		void _nextDirectionUpdated(const std::string &sessionId, Direction d);

		//Helper functions
		GameClient *clientBySessionId(const std::string &sessionId) const;
		GameClient *clientByAuthInfoId(long long authInfoId) const;
		static bool isOppositeDirection(Direction d1, Direction d2) { return (d1 == Up && d2 == Down) || (d1 == Down && d2 == Up) || (d1 == Left && d2 == Right) || (d1 == Right && d2 == Left); }
		bool isValidCoordinate(int x, int y) { return x >= 0 && x < _gamePtr->columns() && y >= 0 && y < _gamePtr->rows(); }

		Wt::Dbo::ptr<Game> _gamePtr;
		GameClientVector _playerList;

		int _nextCellId = 0;
		GameCellMap _allCells;
		std::vector<std::vector<GameCell*>> _collisionGrid;
		std::vector<GameCell*> _foodVector;

		Wt::Dbo::Session &dboSession;
		mutable boost::mutex _mutex;

	private:
		friend class GameCell;
	};
}

#endif