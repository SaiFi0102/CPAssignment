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
		std::string javascriptStr = std::string() +
		"var keyState = { up:false, down:false, left:false, right:false };"
		"handleKeyEvent = function(e, state){"
			"switch(e.which) {"
				"case " + boost::lexical_cast<std::string>(Wt::Key_Up) + ": if(keyState.up == state) return; keyState.up = state; break;"
				"case " + boost::lexical_cast<std::string>(Wt::Key_Down) + ": if(keyState.down == state) return; keyState.down = state; break;"
				"case " + boost::lexical_cast<std::string>(Wt::Key_Left) + ": if(keyState.left == state) return; keyState.left = state; break;"
				"case " + boost::lexical_cast<std::string>(Wt::Key_Right) + ": if(keyState.right == state) return; keyState.right = state; break;"
				"default: return;"
			"}"
			+ _keyStateJSignal.createCall("e.which", "state") +
		"};"
		"$(window).keyup(function(e){ handleKeyEvent(e, false); });"
		"$(window).keydown(function(e){ handleKeyEvent(e, true); });";
		doJavaScript(javascriptStr);

		_gameWidget = new GameWidget(root());

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
		_keyStateUpdated.emit(_keyState);
	}
}
