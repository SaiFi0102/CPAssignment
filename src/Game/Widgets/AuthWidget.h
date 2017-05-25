#ifndef SM_AUTH_WIDGET_H
#define SM_AUTH_WIDGET_H

#include <Wt/Auth/AuthWidget>
#include <Wt/Auth/RegistrationModel>
#include <Wt/WDialog>

namespace SM
{
	class AuthWidget : public Wt::Auth::AuthWidget
	{
	public:
		AuthWidget(Wt::WContainerWidget *parent = nullptr);
	
	protected:
		virtual Wt::Auth::RegistrationModel *createRegistrationModel() override;
		virtual Wt::WDialog *showDialog(const Wt::WString &title, Wt::WWidget *contents) override;
	};
}

#endif