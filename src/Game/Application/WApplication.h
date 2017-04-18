#ifndef DB_WAPPLICATION_H
#define DB_WAPPLICATION_H

#include <Wt/WApplication>
#include <Wt/Dbo/Session>
#include <Wt/WSignal>

namespace DB
{
	class GameWidget;

	struct KeyState
	{
		bool arrowUp = false;
		bool arrowDown = false;
		bool arrowLeft = false;
		bool arrowRight = false;
	};

	class WApplication : public Wt::WApplication
	{
	public:
		WApplication(const Wt::WEnvironment& env);
		//virtual ~WApplication() override;

		static WApplication *instance() { return dynamic_cast<WApplication*>(Wt::WApplication::instance()); }
		static WApplication *createApplication(const Wt::WEnvironment &env) { return new WApplication(env); }

		GameWidget *gameWidget() const { return _gameWidget; }

		Wt::Signal<KeyState> &keyStateUpdated() { return _keyStateUpdated; }

	protected:
		void handleKeyStateEvent(int key, bool state);

		GameWidget *_gameWidget = nullptr;

		KeyState _keyState;
		Wt::JSignal<int, bool> _keyStateJSignal;
		Wt::Signal<KeyState> _keyStateUpdated;

		Wt::Dbo::Session _dboSession;
	};
}

#endif