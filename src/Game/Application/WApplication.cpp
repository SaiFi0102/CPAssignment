#include "Application/WApplication.h"
#include "Application/WServer.h"

#include <Wt/WImage>
#include <Wt/WLink>
#include <Wt/WText>
#include <Wt/WContainerWidget>
#include <Wt/WFitLayout>

namespace DB
{
	WApplication::WApplication(const Wt::WEnvironment &env)
		: Wt::WApplication(env), _keyStateSignal(this, "keyStateSignal")
	{
		messageResourceBundle().use(appRoot() + "templates");
		messageResourceBundle().use(appRoot() + "strings");

		WServer *server = SERVER;
		_dboSession.setConnectionPool(*server->sqlPool());

		setTitle(Wt::WString::tr("DB.Title"));

		Wt::WLocale newLocale("en");
		newLocale.setDateFormat(Wt::WString::tr("DateFormat"));
		newLocale.setTimeFormat(Wt::WString::tr("TimeFormat"));
		newLocale.setDateTimeFormat(Wt::WString::tr("DateTimeFormat"));
		newLocale.setDecimalPoint(Wt::WString::tr("DecimalPoint").toUTF8());
		newLocale.setGroupSeparator(Wt::WString::tr("GroupSeparator").toUTF8());
		setLocale(newLocale);

		_keyStateSignal.connect(this, &WApplication::handleKeyStateEvent);
		std::string javascriptStr = std::string() +
		"var keyState = { up:false, down:false, left:false, right:false };"
		"handleKeyEvent = function(e, state){"
			"if(e.repeat) return;"
			"switch(e.which) {"
				"case " + boost::lexical_cast<std::string>(Wt::Key_Up) + ": if(keyState.up == state) return; keyState.up = state; break;"
				"case " + boost::lexical_cast<std::string>(Wt::Key_Down) + ": if(keyState.down == state) return; keyState.down = state; break;"
				"case " + boost::lexical_cast<std::string>(Wt::Key_Left) + ": if(keyState.left == state) return; keyState.left = state; break;"
				"case " + boost::lexical_cast<std::string>(Wt::Key_Right) + ": if(keyState.right == state) return; keyState.right = state; break;"
				"default: return;"
			"}"
			+ _keyStateSignal.createCall("e.which", "state") +
		"};"
		"$(window).keyup(function(e){ handleKeyEvent(e, false); });"
		"$(window).keydown(function(e){ handleKeyEvent(e, true); });";
		doJavaScript(javascriptStr);

		enableUpdates();
	}

	void WApplication::handleKeyStateEvent(int key, bool state)
	{
		if(key == Wt::Key_Up)
		{
			new Wt::WText("Up" + boost::lexical_cast<std::string>(state), root());
		}
		else if(key == Wt::Key_Down)
		{
			new Wt::WText("Down" + boost::lexical_cast<std::string>(state), root());
		}
		else if(key == Wt::Key_Left)
		{
			new Wt::WText("Left" + boost::lexical_cast<std::string>(state), root());
		}
		else if(key == Wt::Key_Right)
		{
			new Wt::WText("Right" + boost::lexical_cast<std::string>(state), root());
		}
	}
}
/*

root()->addWidget(new Wt::WText("akif is ashit bag"));

Wt::WText *out = new Wt::WText("asd", container);
out->setMargin(10, Wt::Left);
root()->addWidget(new Wt::WText("\ngth "));

*/