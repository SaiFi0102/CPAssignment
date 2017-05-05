#include "Application/WApplication.h"
#include "Application/WServer.h"
#include "Widgets/GameWidget.h"
#include "Widgets/HighscoreWidget.h"
#include "Widgets/AuthWidget.h"
#include "Widgets/MatchFinder.h"
#include "Dbo/Dbos.h"

#include <Wt/WImage>
#include <Wt/WLink>
#include <Wt/WText>
#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/WBootstrapTheme>
#include <Wt/WNavigationBar>
#include <Wt/WMenu>
#include <Wt/WStackedWidget>
#include <Wt/WDialog>
#include <Wt/WPushButton>

namespace DB
{
	AuthLogin::AuthLogin()
	{
		handleLoginChanged();
		changed().connect(this, &AuthLogin::handleLoginChanged);
	}

	void AuthLogin::handleLoginChanged()
	{
		WApplication *app = APP;

		if(loggedIn())
			_authInfoPtr = app->userDatabase().find(user());
		else
			_authInfoPtr = Wt::Dbo::ptr<AuthInfo>();

		//Make sure user entry for authinfo exists
		if(_authInfoPtr && !_authInfoPtr->user())
		{
			TRANSACTION(app);
			_authInfoPtr.modify()->setUser(app->dboSession().add(new User()));
		}
		_afterLoginChanged.emit();
	}

	WApplication::WApplication(const Wt::WEnvironment &env)
		: Wt::WApplication(env), _keyStateJSignal(this, "keyStateSignal"), _userDatabase(_dboSession, &SERVER->getAuthService())
	{
		//Dbos
		WServer *server = SERVER;
		_dboSession.setConnectionPool(*server->sqlPool());
		mapDboTree(_dboSession);

		//Data files and localization
		messageResourceBundle().use(appRoot() + "strings", false);
		messageResourceBundle().use(appRoot() + "templates", false);

		Wt::WLocale newLocale("en");
		newLocale.setDateFormat(Wt::WString::tr("DateFormat"));
		newLocale.setTimeFormat(Wt::WString::tr("TimeFormat"));
		newLocale.setDateTimeFormat(Wt::WString::tr("DateTimeFormat"));
		newLocale.setDecimalPoint(Wt::WString::tr("DecimalPoint").toUTF8());
		newLocale.setGroupSeparator(Wt::WString::tr("GroupSeparator").toUTF8());
		setLocale(newLocale);

		//Title and stylesheet
		setTitle(Wt::WString::tr("DB.Title"));
		useStyleSheet(Wt::WLink("style.css"));
		styleSheet().addRule("div.Wt-loading",
			"position:absolute;top:0px;right:0;z-index:9999999;background:#457044;padding:10px 15px;color:#fff;border-radius:0 0 0 3px;"
			"-webkit-box-shadow: -1px 1px 2px 0px #000;"
			"-moz-box-shadow: -1px 1px 2px 0px #000;"
			"box-shadow: -1px 1px 2px 0px #000;");

		//Insert key state event handler
		_keyStateJSignal.connect(this, &WApplication::handleKeyStateEvent);
		Wt::WString keyEventJs = Wt::WString::tr("KeyEventJs")
			.arg(Wt::Key_Up).arg(Wt::Key_Down).arg(Wt::Key_Left).arg(Wt::Key_Right)
			.arg(_keyStateJSignal.createCall("e.which", "state"));
		doJavaScript(keyEventJs.toUTF8());

		//Using bootstrap v3
		Wt::WBootstrapTheme *theme = new Wt::WBootstrapTheme();
		theme->setVersion(Wt::WBootstrapTheme::Version3);
		setTheme(theme);

		//Error Dialog
		_errorDialog = new Wt::WDialog(Wt::WString::tr("AnErrorOccurred"), this);
		_errorDialog->setTransient(true);
		_errorDialog->rejectWhenEscapePressed(true);
		_errorDialog->setClosable(true);
		_errorDialogText = new Wt::WText(_errorDialog->contents());
		Wt::WBreak *hr = new Wt::WBreak(_errorDialog->contents());
		hr->setHtmlTagName("hr");
		auto errorOkBtn = new Wt::WPushButton(Wt::WString::tr("Ok"), _errorDialog->contents());
		errorOkBtn->clicked().connect(_errorDialog, &Wt::WDialog::accept);

		//Main stack
		_mainStack = new Wt::WStackedWidget(root());
		_authWidget = new AuthWidget(_mainStack);

		setInternalPathDefaultValid(true);
		handleInternalPathChanged(internalPath());
		internalPathChanged().connect(this, &WApplication::handleInternalPathChanged);
		authLogin().afterLoginChanged().connect(this, &WApplication::handleAuthChanged);

		enableUpdates();
	}

	void WApplication::lazyLoadGame()
	{
		if(_mainContainer)
			return;

		_mainContainer = new Wt::WContainerWidget(_mainStack);

		_navBar = new Wt::WNavigationBar(_mainContainer);
		_navBar->setTitle(Wt::WString::tr("DB.Title"));
		_navStack = new Wt::WStackedWidget(_mainContainer);
		_navBar->addMenu(_menu = new Wt::WMenu(_navStack));

		_gameStack = new Wt::WStackedWidget();
		_matchFinder = new MatchFinder(_gameStack);
		_highscoreWidget = new HighscoreWidget();

		Wt::WMenuItem *gameMenuItem = new Wt::WMenuItem("Game", _gameStack);
		_menu->addItem(gameMenuItem);

		Wt::WMenuItem *highscoreMenuItem = new Wt::WMenuItem("Highscores", _highscoreWidget);
		_menu->addItem(highscoreMenuItem);
	}

	void WApplication::handleAuthChanged()
	{
		if(!authLogin().loggedIn())
		{
			delete _mainContainer;
			_mainContainer = nullptr;
		}
		handleInternalPathChanged(internalPath());
	}

	void WApplication::handleInternalPathChanged(std::string path)
	{
		if(!path.empty() && path.back() != '/')
			path.push_back('/');

		if(!authLogin().loggedIn())
		{
			_mainStack->setCurrentWidget(_authWidget);
		}
		else
		{
			lazyLoadGame();
			_mainStack->setCurrentWidget(_mainContainer);
		}
	}

	void WApplication::handleKeyStateEvent(int key, bool state)
	{
		if(key == Wt::Key_Up)
		{
			_keyState.arrowUp = state;
		}
		else if(key == Wt::Key_Down)
		{
			_keyState.arrowDown = state;
		}
		else if(key == Wt::Key_Left)
		{
			_keyState.arrowLeft = state;
		}
		else if(key == Wt::Key_Right)
		{
			_keyState.arrowRight = state;
		}
		_keyStateUpdated.emit((Wt::Key)key, state);
	}

	void WApplication::showErrorDialog(const Wt::WString &message)
	{
		_errorDialogText->setText(message);
		_errorDialog->show();
	}

	void WApplication::showDbBackendError(const std::string &)
	{
		showErrorDialog(Wt::WString::tr("DbInternalError"));
	}
}
