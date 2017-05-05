#ifndef DB_GAME_WIDGET_H
#define DB_GAME_WIDGET_H

#include "Application/GameServer.h"

#include <Wt/WGlobal>
#include <Wt/WContainerWidget>

#define CELL_WIDTH  20
#define CELL_HEIGHT 20

namespace DB
{
	typedef std::unordered_map<size_t, Wt::WImage*> SpriteMap;

	class GameWidget : public Wt::WContainerWidget
	{
	public:
		GameWidget(GameServer *server, Wt::WContainerWidget *parent = nullptr);

	protected:
		void handleKeyState(Wt::Key key, bool state);

		static void _connected(const GameClientVector &clients, const GameCellMap &cellsMap);
		static void _update(const GameCellMap &cellsMap);

		bool _isConnected = false;
		GameServer *_server = nullptr;
		SpriteMap _spriteMap;

	private:
		friend class GameServer;
	};
}
#endif
