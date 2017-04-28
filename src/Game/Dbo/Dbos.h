#ifndef DB_DBOS_H
#define DB_DBOS_H

#include <Wt/WGlobal>
#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/WString>

namespace DB
{
	class Highscore;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Highscore>> HighscoreCollection;

	class Highscore
	{
	protected:
		Wt::WString _playerName;
		int _score = 0;

	public:
		Wt::WString playerName() const { return _playerName; }
		int score() const { return _score; }

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::field(a, _playerName, "playerName", 64);
			Wt::Dbo::field(a, _score, "score");
		}
	};
}

#endif