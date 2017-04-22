#include "Application/WApplication.h"
#include "Application/WServer.h"
#include "Widgets/GameWidget.h"

#include <Wt/WImage>
#include <Wt/WLink>
#include <Wt/WText>
#include <Wt/WContainerWidget>

namespace DB
{
	WApplication::WApplication(const Wt::WEnvironment &env)
		: Wt::WApplication(env), _keyStateJSignal(this, "keyStateSignal")
	{
		messageResourceBundle().use(appRoot() + "strings", false);
		messageResourceBundle().use(appRoot() + "templates", false);

		WServer *server = SERVER;
		_dboSession.setConnectionPool(*server->sqlPool());

		Wt::WLocale newLocale("en");
		newLocale.setDateFormat(Wt::WString::tr("DateFormat"));
		newLocale.setTimeFormat(Wt::WString::tr("TimeFormat"));
		newLocale.setDateTimeFormat(Wt::WString::tr("DateTimeFormat"));
		newLocale.setDecimalPoint(Wt::WString::tr("DecimalPoint").toUTF8());
		newLocale.setGroupSeparator(Wt::WString::tr("GroupSeparator").toUTF8());
		setLocale(newLocale);

		setTitle(Wt::WString::tr("DB.Title"));
		useStyleSheet(Wt::WLink("style.css"));

		_keyStateJSignal.connect(this, &WApplication::handleKeyStateEvent);
		Wt::WString keyEventJs = Wt::WString::tr("KeyEventJs")
			.arg(Wt::Key_Up).arg(Wt::Key_Down).arg(Wt::Key_Left).arg(Wt::Key_Right)
			.arg(_keyStateJSignal.createCall("e.which", "state"));
		doJavaScript(keyEventJs.toUTF8());

		_gameWidget = new GameWidget(root());
		_gameWidget->resize(500, 500);
		_debugText = new Wt::WText(root());
		_debugText->setId("debugWText");

		enableUpdates();
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
}
