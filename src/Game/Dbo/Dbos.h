#ifndef SM_DBOS_H
#define SM_DBOS_H

#include <Wt/WGlobal>
#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/WString>
#include <Wt/WDate>

#define TRANSACTION(app) Wt::Dbo::Transaction transaction(app->dboSession())

namespace SM
{
	void mapDboTree(Wt::Dbo::Session &dboSession);

	//User related
	class User;
	typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;
	typedef Wt::Auth::Dbo::AuthIdentity<AuthInfo> AuthIdentity;
	typedef Wt::Auth::Dbo::AuthToken<AuthInfo> AuthToken;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<AuthInfo>> AuthInfoCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<User>> UserCollection;

	typedef Wt::Auth::Dbo::UserDatabase<SM::AuthInfo> UserDatabase;

	//Game related
	class Game;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Game>> GameCollection;

	class User
	{
	public:
		Wt::Dbo::weak_ptr<AuthInfo> authInfoWPtr;
		GameCollection gameCollection;
		GameCollection gamesWon;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::hasOne(a, authInfoWPtr, "user");
			Wt::Dbo::hasMany(a, gameCollection, Wt::Dbo::ManyToMany, "user_game");
			Wt::Dbo::hasMany(a, gamesWon, Wt::Dbo::ManyToOne, "winner_user");
		}
	};

	class Game
	{
	public:
		enum State
		{
			InProgress, GameOver, Disconnected
		};

		Wt::WString name() const { return _name; }
		State state() const { return _state; }
		int rows() const { return _rows; }
		int columns() const { return _columns; }
		Wt::Dbo::ptr<User> winnerUserPtr() const { return _winnerUserPtr; }
		UserCollection &userCollection() { return _userCollection; }

		void setName(const Wt::WString &name) { _name = name; }
		void setState(State state) { _state = state; }
		void setRows(int rows) { _rows = rows; }
		void setColumns(int columns) { _columns = columns; }
		void setWinnerUserPtr(Wt::Dbo::ptr<User> user) { _winnerUserPtr = user; }

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::field(a, _name, "name");
			Wt::Dbo::field(a, _rows, "rows");
			Wt::Dbo::field(a, _columns, "columns");
			Wt::Dbo::belongsTo(a, _winnerUserPtr, "winner_user", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);
			Wt::Dbo::hasMany(a, _userCollection, Wt::Dbo::ManyToMany, "user_game");
		}

	protected:
		Wt::WString _name;
		State _state;
		int _rows;
		int _columns;

		Wt::Dbo::ptr<User> _winnerUserPtr;
		UserCollection _userCollection;
	};
}

#endif