#ifndef DB_WAPPLICATION_H
#define DB_WAPPLICATION_H

#include "Dbo/Dbos.h"
#include <Wt/WGlobal>
#include <Wt/WApplication>
#include <Wt/Dbo/Session>
#include <Wt/WSignal>
#include <Wt/Auth/Login>
#include <Wt/Auth/Dbo/UserDatabase>

namespace DB
{
	class GameWidget;
	class HighscoreWidget;
	class MatchFinder;
	class AuthWidget;

	struct KeyState
	{
		bool arrowUp = false;
		bool arrowDown = false;
		bool arrowLeft = false;
		bool arrowRight = false;

		bool anyEnabled() const { return arrowUp || arrowDown || arrowLeft || arrowRight; }
	};

	class AuthLogin : public Wt::Auth::Login
	{
	public:
		AuthLogin();
		Wt::Dbo::ptr<AuthInfo> authInfoPtr() const { return _authInfoPtr; };
		Wt::Dbo::ptr<User> userPtr() const { return _authInfoPtr ? _authInfoPtr->user() : Wt::Dbo::ptr<User>(); }
		Wt::Signal<void> &afterLoginChanged() { return _afterLoginChanged; }

	protected:
		void handleLoginChanged();
		Wt::Signal<void> _afterLoginChanged;
		Wt::Dbo::ptr<AuthInfo> _authInfoPtr;
	};

	class WApplication : public Wt::WApplication
	{
	public:
		WApplication(const Wt::WEnvironment& env);
		//virtual ~WApplication() override;

		static WApplication *instance() { return dynamic_cast<WApplication*>(Wt::WApplication::instance()); }
		static WApplication *createApplication(const Wt::WEnvironment &env) { return new WApplication(env); }

		const KeyState &keyState() const { return _keyState; }
		Wt::Signal<Wt::Key, bool> &keyStateUpdated() { return _keyStateUpdated; }

		//Dbos and auth
		Wt::Dbo::Session &dboSession() { return _dboSession; }
		AuthLogin &authLogin() { return _login; }
		UserDatabase &userDatabase() { return _userDatabase; }

		//Widgets
		Wt::WStackedWidget *mainStack() const { return _navStack; }
		Wt::WStackedWidget *gameStack() const { return _gameStack; }
		MatchFinder *matchFinder() const { return _matchFinder; }
		GameWidget *gameWidget() const { return _gameWidget; }
		void setGameWidget(GameWidget *w) { _gameWidget = w; }

		//Error handling
		Wt::WDialog *errorDialog() const { return _errorDialog; }
		void showErrorDialog(const Wt::WString &message);
		void showStaleObjectError() { showErrorDialog(Wt::WString::tr("StaleObjectError")); }
		void showDbBackendError(const std::string &code);

	protected:
		void lazyLoadGame();

		void handleAuthChanged();
		void handleInternalPathChanged(std::string path);
		void handleKeyStateEvent(int key, bool state);

		Wt::WStackedWidget *_mainStack = nullptr;
		AuthWidget *_authWidget = nullptr;

		Wt::WContainerWidget *_mainContainer = nullptr;
		Wt::WNavigationBar *_navBar = nullptr;
		Wt::WMenu *_menu = nullptr;
		Wt::WStackedWidget *_navStack = nullptr;

		Wt::WStackedWidget *_gameStack = nullptr;
		MatchFinder *_matchFinder = nullptr;
		GameWidget *_gameWidget = nullptr;
		HighscoreWidget *_highscoreWidget = nullptr;

		Wt::WDialog *_errorDialog = nullptr;
		Wt::WText *_errorDialogText = nullptr;

		KeyState _keyState;
		Wt::JSignal<int , bool> _keyStateJSignal;
		Wt::Signal<Wt::Key, bool> _keyStateUpdated;

		Wt::Dbo::Session _dboSession;
		AuthLogin _login;
		UserDatabase _userDatabase;
	};
}

#define APP DB::WApplication::instance()

#endif