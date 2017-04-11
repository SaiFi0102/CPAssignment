#ifndef DB_WAPPLICATION_H
#define DB_WAPPLICATION_H

#include <Wt/WApplication>
#include <Wt/Dbo/Session>
#include<WT/WImage>

namespace DB
{
	class WApplication : public Wt::WApplication
	{
	public:
		WApplication(const Wt::WEnvironment& env);
		//virtual ~WApplication() override;

		static WApplication *instance() { return dynamic_cast<WApplication*>(Wt::WApplication::instance()); }
		static WApplication *createApplication(const Wt::WEnvironment &env) { return new WApplication(env); }
		void showImage();

	protected:
		void handleMouseEvent(bool down, const Wt::WKeyEvent &e);

		Wt::Dbo::Session _dboSession;
	};
}

#endif