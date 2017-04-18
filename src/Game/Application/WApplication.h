#ifndef DB_WAPPLICATION_H
#define DB_WAPPLICATION_H

#include <Wt/WApplication>
#include <Wt/Dbo/Session>
#include <Wt/WImage>
#include <Wt/WSignal>

namespace DB
{
	class WApplication : public Wt::WApplication
	{
	public:
		WApplication(const Wt::WEnvironment& env);
		//virtual ~WApplication() override;

		static WApplication *instance() { return dynamic_cast<WApplication*>(Wt::WApplication::instance()); }
		static WApplication *createApplication(const Wt::WEnvironment &env) { return new WApplication(env); }

	protected:
		void handleKeyStateEvent(int key, bool state);

		Wt::JSignal<int, bool> _keyStateSignal;
		Wt::Dbo::Session _dboSession;
	};
}

#endif