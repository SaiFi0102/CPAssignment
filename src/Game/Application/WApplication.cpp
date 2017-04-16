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

		showImage();

		enableUpdates();
	}

	void WApplication::showImage()
	{
	//	Wt::WContainerWidget *container = new Wt::WContainerWidget(root());
		//container->setLayout(new Wt::WFitLayout());
		//container->decorationStyle().backgroundColor().setRgb(255, 0, 0);

		root()->addWidget(new Wt::WText("akif is ashit bag"));

		image = new Wt::WImage(Wt::WLink("icons/aw.png"));
		image->setPositionScheme(Wt::Fixed);
		image->setAlternateText("Wt logo");
		image->setOffsets(100, Wt::Top);
		image->setOffsets(100, Wt::Left);

		trop = new Wt::WImage(Wt::WLink("icons/trop.png"));
		trop->setPositionScheme(Wt::Fixed);
		trop->setAlternateText("Wt logo 2");
		trop->setOffsets(500, Wt::Top);
		trop->setOffsets(500, Wt::Left);

		globalKeyWentDown().connect(boost::bind(&WApplication::handleMouseEvent, this, true, _1));
		//globalKeyWentUp().connect(boost::bind(&WApplication::handleMouseEvent, this, false, _1));

		root()->addWidget(image);
		root()->addWidget(trop);

		log("db-info") << "found ur stuff shakeeb";

	}
	void WApplication::handleMouseEvent(bool down, const Wt::WKeyEvent &e)
	{
		if (e.key() == Wt::Key_Up)
		{
			if (image)
			{
				image->setOffsets(Wt::WLength(image->offset(Wt::Top).value() - 5), Wt::Top);
				winn(image);
			}
		}
		else if (e.key() == Wt::Key_Down)
		{
			if (image)
			{
				image->setOffsets(Wt::WLength(image->offset(Wt::Top).value() + 5), Wt::Top);
				winn(image);
			}
		}
		else if (e.key() == Wt::Key_Left)
		{
			if (image)
			{
				image->setOffsets(Wt::WLength(image->offset(Wt::Left).value() - 5), Wt::Left);
				winn(image);
			}
		}
		else if (e.key() == Wt::Key_Right)
		{
			if (image)
			{
				image->setOffsets(Wt::WLength(image->offset(Wt::Left).value() + 5), Wt::Left);
				winn(image);
			}
		}
		
	}
	void	WApplication::winn(Wt::WImage *images)
	{
		if (images->offset(Wt::Left).value() > 445 && images->offset(Wt::Left).value() > 655 && images->offset(Wt::Top).value()>515 && images->offset(Wt::Top).value()< 675)
		{
			delete image;
			image = nullptr;
			delete trop;
			cong = new Wt::WImage(Wt::WLink("icons/cong.png"));
			cong->setPositionScheme(Wt::Fixed);
			cong->setAlternateText("Wt logo");
			cong->setOffsets(100, Wt::Top);
			cong->setOffsets(100, Wt::Left);

			root()->addWidget(new Wt::WText("akif is ashit bag"));

			root()->addWidget(cong);

			root()->addWidget(new Wt::WText("akif is ashit bag"));

		}
	}
}
/*

root()->addWidget(new Wt::WText("akif is ashit bag"));

Wt::WText *out = new Wt::WText("asd", container);
out->setMargin(10, Wt::Left);
root()->addWidget(new Wt::WText("\ngth "));

*/