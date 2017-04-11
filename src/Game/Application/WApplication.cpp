#include "Application/WApplication.h"
#include "Application/WServer.h"
#include<WT/WImage>
#include<Wt/WLink>
#include <Wt/WText>
#include <Wt/WContainerWidget>

namespace DB
{
	WApplication::WApplication(const Wt::WEnvironment &env)
		: Wt::WApplication(env)
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

		root()->addWidget(new Wt::WText("akif is ashit bag"));
		showImage();
		//Enable server push
		enableUpdates();
	}
	//did some useless shit

	
	void WApplication::showImage()
	{
		Wt::WContainerWidget *container = new Wt::WContainerWidget(root());

		Wt::WImage *image = new Wt::WImage(Wt::WLink("icons/aw.png"));
		image->setPositionScheme(Wt::Fixed);
		image->setAlternateText("Wt logo");

		globalKeyWentDown().connect(boost::bind(this, &WApplication::handleMouseEvent, true, _1));
		globalKeyWentUp().connect(boost::bind(this, &WApplication::handleMouseEvent, false, _1));

		root()->addWidget(image);

		Wt::WText *out = new Wt::WText("asd", container);
		out->setMargin(10, Wt::Left);
		root()->addWidget(new Wt::WText("\ngth "));
		log("db-info") << "found ur stuff shakeeb";

	}
	void WApplication::handleMouseEvent(bool down, const Wt::WKeyEvent &e)
	{
		if ( Wt::Key::Key_Up == e.key)
		{
			root()->addWidget(new Wt::WText("Up"));
		}
		/*else if (e.key == Wt::Key::Key_Down)
		{

		}
		else if (e.key == Wt::Key::Key_Left)
		{

		}
		else if (e.key == Wt::Key::Key_Right)
		{

		}*/
	}
}