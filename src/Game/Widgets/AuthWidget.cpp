#include "Widgets/AuthWidget.h"
#include "Application/WApplication.h"
#include "Application/WServer.h"

namespace SM
{

	AuthWidget::AuthWidget(Wt::WContainerWidget *parent)
		: Wt::Auth::AuthWidget(SERVER->getAuthService(), APP->userDatabase(), APP->authLogin(), parent)
	{
		WServer *server = SERVER;

		model()->addPasswordAuth(&server->getPasswordService());
		model()->addOAuth(server->getOAuthServices());
		setRegistrationEnabled(true);
		processEnvironment();
	}

	Wt::WDialog *AuthWidget::showDialog(const Wt::WString &title, Wt::WWidget *contents)
	{
		Wt::WDialog *d = Wt::Auth::AuthWidget::showDialog(title, contents);
		d->resize(Wt::WLength(70, Wt::WLength::Percentage), Wt::WLength::Auto);
		d->setMaximumSize(Wt::WLength::Auto, Wt::WLength(90, Wt::WLength::Percentage));
		d->contents()->setOverflow(Wt::WContainerWidget::OverflowAuto);
		d->setClosable(true);
		return d;
	}

	Wt::Auth::RegistrationModel *AuthWidget::createRegistrationModel()
	{
		Wt::Auth::RegistrationModel *m = Wt::Auth::AuthWidget::createRegistrationModel();
		m->setEmailPolicy(Wt::Auth::RegistrationModel::EmailOptional);
		return m;
	}

}
