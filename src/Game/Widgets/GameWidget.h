#ifndef DB_GAME_WIDGET_H
#define DB_GAME_WIDGET_H

#include <Wt/WGlobal>
#include <Wt/WContainerWidget>
#include <Wt/WImage>

#define GRID_SIZE_X 25
#define GRID_SIZE_Y 25
#define CELL_WIDTH 20
#define CELL_HEIGHT 20

namespace DB
{
	class GameWidget;

	enum Direction
	{
		NoChange,
		Up,
		Down,
		Left,
		Right
	};
	bool isOppositeDirection(Direction d1, Direction d2);
	bool isValidCoordinate(int x, int y);

	class GameSprite : public Wt::WImage
	{
	public:
		GameSprite(GameWidget *parent, int x, int y, GameSprite *head = nullptr);
		virtual ~GameSprite() override;

		virtual void updateImageSrc() { }

		GameSprite *head() const { return _head; }
		GameSprite *tail() const { return _tail; }

		bool isHead() const { return !head(); }
		bool isTail() const { return !tail(); }
		bool isTurningPoint() const;

		int rate() const { return _rate; }
		void setRate(int rate) { _rate = rate; }

		Direction direction() const { return _direction; }
		void setDirection(Direction direction) { _direction = direction; }

		int x() const { return _x; }
		int y() const { return _y; }

	protected:
		int _x, _y;

	private:
		GameSprite *_head = nullptr;
		GameSprite *_tail = nullptr;
		int _rate = 0;
		Direction _direction = NoChange;

		friend class GameWidget;
	};

	class SnakeSprite : public GameSprite
	{
	public:
		SnakeSprite(GameWidget *parent, Direction direction, int x, int y);
		SnakeSprite(GameWidget *parent, SnakeSprite *head);

	protected:
		virtual void updateImageSrc() override;
	};


	class GameWidget : public Wt::WContainerWidget
	{
	public:
		GameWidget(Wt::WContainerWidget *parent = nullptr);
		
		GameSprite *head() const { return _head; }

	protected:
		void handleKeyState(Wt::Key key, bool state);
		void update();

		void gameOver();

		Direction _nextDirection = NoChange;
		Wt::WTimer *_timer;

		SnakeSprite *_head = nullptr;
		GameSprite *_grid[GRID_SIZE_X][GRID_SIZE_Y];
	};
}

#endif
