#include "Widgets/MatchFinder.h"
#include "Widgets/GameWidget.h"
#include "Application/WApplication.h"
#include "Application/WServer.h"
#include "Application/MatchServer.h"

#include <Wt/WComboBox>
#include <Wt/WText>
#include <Wt/WStackedWidget>

namespace SM
{
	const Wt::WFormModel::Field MatchFinder::modeField = "mode";
	const Wt::WFormModel::Field MatchFinder::nPlayersField = "nPlayers";

	MatchFinder::MatchFinder(Wt::WContainerWidget *parent /*= nullptr*/)
		: Wt::WTemplateFormView(tr("SM.MatchFinder"), parent)
	{
		_formModel = new Wt::WFormModel(this);
		_formModel->addField(modeField);
		_formModel->addField(nPlayersField);

		Wt::WComboBox *modeEdit = new Wt::WComboBox();
		modeEdit->insertItem(0, "Standard");
		setFormWidget(modeField, modeEdit);

		Wt::WComboBox *nPlayersFieldEdit = new Wt::WComboBox();
		nPlayersFieldEdit->insertItem(0, "2");
		setFormWidget(nPlayersField, nPlayersFieldEdit);

		Wt::WPushButton *createBtn = new Wt::WPushButton(tr("Create"));
		createBtn->clicked().connect(boost::bind(&MatchFinder::createClicked, this));
		bindWidget("createBtn", createBtn);

		Wt::WPushButton *leaveBtn = new Wt::WPushButton(tr("Leave"));
		leaveBtn->clicked().connect(boost::bind(&MatchFinder::leaveClicked, this));
		bindWidget("leaveBtn", leaveBtn);

		Wt::WPushButton *readyBtn = new Wt::WPushButton(tr("Ready"));
		readyBtn->clicked().connect(boost::bind(&MatchFinder::readyClicked, this));
		bindWidget("readyBtn", readyBtn);

		_matchesContainer = new Wt::WContainerWidget();
		bindWidget("matches", _matchesContainer);
		
		_onlinePlayersContainer = new Wt::WContainerWidget();
		bindWidget("onlinePlayers", _onlinePlayersContainer);
		
		_matchPlayersContainer = new Wt::WContainerWidget();
		bindWidget("matchPlayers", _matchPlayersContainer);

		setFormView();		
		updateView(_formModel);

		SERVER->matchMakingServer()->connect();
	}

	MatchFinder::~MatchFinder()
	{
		SERVER->matchMakingServer()->disconnect();
	}

	//UI STUFF
	Wt::WPushButton * MatchFinder::createMatchButton(const Wt::WString &matchName)
	{
		Wt::WPushButton *btn = new Wt::WPushButton(matchName);
		btn->setStyleClass("list-group-item");
		btn->clicked().connect(boost::bind(&MatchFinder::matchClicked, this, btn));
		return btn;
	}

	Wt::WPushButton * MatchFinder::createPlayerButton(Wt::Dbo::ptr<AuthInfo> authInfoPtr)
	{
		TRANSACTION(APP);
		Wt::WPushButton *btn = new Wt::WPushButton(authInfoPtr->identity(Wt::Auth::Identity::LoginName));
		btn->setStyleClass("list-group-item");
		btn->clicked().connect(boost::bind(&MatchFinder::playerClicked, this, btn));
		return btn;
	}

	Wt::WTemplate * MatchFinder::createMatchPlayerTemplate(Wt::Dbo::ptr<AuthInfo> authInfoPtr)
	{
		TRANSACTION(APP);
		Wt::WTemplate *w = new Wt::WTemplate(tr("SM.MatchPlayerItem"));
		w->bindString("name", authInfoPtr->identity(Wt::Auth::Identity::LoginName));
		w->bindString("ready", Wt::WString::tr("NotReady"));
		return w;
	}

	void MatchFinder::setMatchView()
	{
		bindString("matchName", _currentMatch.name);
		setCondition("show-form", false);
		setCondition("show-match", true);
	}

	void MatchFinder::setFormView()
	{
		bindEmpty("matchName");
		setCondition("show-form", true);
		setCondition("show-match", false);
	}

	//Client event handlers
	void MatchFinder::createClicked()
	{
		if(_isConnected && _currentMatch.id == -1)
			SERVER->matchMakingServer()->createMatch();
	}

	void MatchFinder::leaveClicked()
	{
		if(_isConnected && _currentMatch.id != -1)
			SERVER->matchMakingServer()->leaveMatch();
	}

	void MatchFinder::readyClicked()
	{
		if(_isConnected && _currentMatch.id != -1)
			SERVER->matchMakingServer()->readyMatch();
	}

	void MatchFinder::matchClicked(Wt::WPushButton *btn)
	{
		if(_isConnected && _currentMatch.id != -1)
			return;

		int idx = _matchesContainer->indexOf(btn);
		if(idx == -1)
			return;

		SERVER->matchMakingServer()->joinMatch(_matchList[idx].id);
	}

	void MatchFinder::playerClicked(Wt::WPushButton *btn)
	{
		
	}

	//Server event handlers
	void MatchFinder::_connected(const MatchVector &matchVector, const MatchMakingServer::IdVector &onlinePlayerAutoInfoIds)
	{
		WApplication *app = APP;
		MatchFinder *w = app->matchFinder();
		if(!w)
			return;
		
		w->_isConnected = true;

		for(const auto &match : matchVector)
		{
			w->_matchList.push_back(ClientMatch(match));
		}
		while(w->_matchesContainer->count() < w->_matchList.size())
		{
			Wt::WPushButton *btn = w->createMatchButton(w->_matchList[w->_matchesContainer->count()].name);
			w->_matchesContainer->addWidget(btn);
		}

		for(const auto &authInfoId : onlinePlayerAutoInfoIds)
		{
			w->_onlinePlayerList.push_back(app->dboSession().loadLazy<AuthInfo>(authInfoId));
		}
		while(w->_onlinePlayersContainer->count() < w->_onlinePlayerList.size())
		{
			Wt::WPushButton *btn = w->createPlayerButton(w->_onlinePlayerList[w->_onlinePlayersContainer->count()]);
			w->_onlinePlayersContainer->addWidget(btn);
		}

		app->triggerUpdate();
	}

	void MatchFinder::_disconnected()
	{
		WApplication *app = APP;
		MatchFinder *w = app->matchFinder();
		if(!w)
			return;

		w->_isConnected = false;

		w->_matchList.clear();
		while(w->_matchesContainer->count() != 0)
			delete w->_matchesContainer->widget(w->_matchesContainer->count() - 1);

		w->_onlinePlayerList.clear();
		while(w->_onlinePlayersContainer->count() != 0)
			delete w->_onlinePlayersContainer->widget(w->_onlinePlayersContainer->count() - 1);

		app->triggerUpdate();
	}

	void MatchFinder::_playerConnected(long long authInfoId)
	{
		WApplication *app = APP;
		MatchFinder *w = app->matchFinder();
		if(!w || !w->_isConnected)
			return;

		Wt::Dbo::ptr<AuthInfo> authInfoPtr = app->dboSession().loadLazy<AuthInfo>(authInfoId);
		w->_onlinePlayerList.push_back(authInfoPtr);
		Wt::WPushButton *btn = w->createPlayerButton(authInfoPtr);
		w->_onlinePlayersContainer->addWidget(btn);

		app->triggerUpdate();
	}

	void MatchFinder::_playerDisconnected(long long authInfoId)
	{
		WApplication *app = APP;
		MatchFinder *w = app->matchFinder();
		if(!w || !w->_isConnected)
			return;

		//Find index of the player disconnected
		AuthInfoVector::iterator fItr = std::find(w->_onlinePlayerList.begin(), w->_onlinePlayerList.end(), app->dboSession().loadLazy<AuthInfo>(authInfoId));
		if(fItr == w->_onlinePlayerList.end())
			return;

		size_t idx = fItr - w->_onlinePlayerList.begin();
		w->_onlinePlayerList.erase(fItr);
		delete w->_onlinePlayersContainer->widget(idx);

		app->triggerUpdate();
	}

	void MatchFinder::_matchAdded(Match *match)
	{
		WApplication *app = APP;
		MatchFinder *w = app->matchFinder();
		if(!w || !w->_isConnected)
			return;
		
		if(!match)
			return;

		w->_matchList.push_back(ClientMatch(match));
		Wt::WPushButton *btn = w->createMatchButton(match->matchName);
		w->_matchesContainer->addWidget(btn);

		app->triggerUpdate();
	}

	void MatchFinder::_matchRemoved(long long matchId)
	{
		WApplication *app = APP;
		MatchFinder *w = app->matchFinder();
		if(!w || !w->_isConnected)
			return;

		//Find index of the match remove
		ClientMatchVector::const_iterator fItr;
		for(fItr = w->_matchList.begin(); fItr != w->_matchList.end(); ++fItr)
		{
			if(fItr->id == matchId)
				break;
		}
		if(fItr == w->_matchList.end())
			return;

		size_t idx = fItr - w->_matchList.begin();
		w->_matchList.erase(fItr);
		delete w->_matchesContainer->widget(idx);

		app->triggerUpdate();
	}

	void MatchFinder::_matchJoined(Match *match)
	{
		WApplication *app = APP;
		MatchFinder *w = app->matchFinder();
		if(!w || !w->_isConnected)
			return;

		if(!match)
			return;

		w->_currentMatch = ClientMatch(match);
		w->_matchPlayerList.clear();
		w->_matchPlayersContainer->clear();

		for(const auto &c : match->clientList)
		{
			Wt::Dbo::ptr<AuthInfo> authInfoPtr = app->dboSession().loadLazy<AuthInfo>(c->authInfoId);
			w->_matchPlayerList.push_back(authInfoPtr);
			Wt::WWidget *playerWidget = w->createMatchPlayerTemplate(authInfoPtr);
			w->_matchPlayersContainer->addWidget(playerWidget);
		}

		w->setMatchView();
		app->triggerUpdate();
	}

	void MatchFinder::_matchLeft()
	{
		WApplication *app = APP;
		MatchFinder *w = app->matchFinder();
		if(!w || !w->_isConnected)
			return;

		w->_currentMatch.id = -1;
		w->_currentMatch.name = Wt::WString::Empty;
		w->_matchPlayerList.clear();
		w->_matchPlayersContainer->clear();

		w->setFormView();
		app->triggerUpdate();
	}

	void MatchFinder::_matchPlayerAdded(long long authInfoId)
	{
		WApplication *app = APP;
		MatchFinder *w = app->matchFinder();
		if(!w || !w->_isConnected)
			return;

		if(w->_currentMatch.id == -1)
			return;

		Wt::Dbo::ptr<AuthInfo> authInfoPtr = app->dboSession().loadLazy<AuthInfo>(authInfoId);
		w->_matchPlayerList.push_back(authInfoPtr);
		Wt::WTemplate *playerWidget = w->createMatchPlayerTemplate(authInfoPtr);
		w->_matchPlayersContainer->addWidget(playerWidget);
		
		app->triggerUpdate();
	}

	void MatchFinder::_matchPlayerRemoved(long long authInfoId)
	{
		WApplication *app = APP;
		MatchFinder *w = app->matchFinder();
		if(!w || !w->_isConnected)
			return;

		if(w->_currentMatch.id == -1)
			return;

		Wt::Dbo::ptr<AuthInfo> authInfoPtr = app->dboSession().loadLazy<AuthInfo>(authInfoId);
		AuthInfoVector::const_iterator matchPlayerListItr = std::find(w->_matchPlayerList.begin(), w->_matchPlayerList.end(), authInfoPtr);
		if(matchPlayerListItr == w->_matchPlayerList.end())
			return;

		size_t idx = matchPlayerListItr - w->_matchPlayerList.begin();
		w->_matchPlayerList.erase(matchPlayerListItr);
		delete w->_matchPlayersContainer->widget(idx);

		app->triggerUpdate();
	}

	void MatchFinder::_matchPlayerReadyChanged(long long authInfoId, bool ready)
	{
		WApplication *app = APP;
		MatchFinder *w = app->matchFinder();
		if(!w || !w->_isConnected)
			return;

		if(w->_currentMatch.id == -1)
			return;

		Wt::Dbo::ptr<AuthInfo> authInfoPtr = app->dboSession().loadLazy<AuthInfo>(authInfoId);
		AuthInfoVector::const_iterator matchPlayerListItr = std::find(w->_matchPlayerList.begin(), w->_matchPlayerList.end(), authInfoPtr);
		if(matchPlayerListItr == w->_matchPlayerList.end())
			return;

		size_t idx = matchPlayerListItr - w->_matchPlayerList.begin();
		Wt::WTemplate *playerWidget = static_cast<Wt::WTemplate*>(w->_matchPlayersContainer->widget(idx));
		if(ready)
			playerWidget->bindString("ready", Wt::WString::tr("Ready"));
		else
			playerWidget->bindString("ready", Wt::WString::tr("NotReady"));

		app->triggerUpdate();
	}

	void MatchFinder::_gameStarted(GameServer *server)
	{
		WApplication *app = APP;
		MatchFinder *w = app->matchFinder();
		if(!w || !w->_isConnected)
			return;

		if(w->_currentMatch.id == -1)
			return;

		Wt::WStackedWidget *parentStack = static_cast<Wt::WStackedWidget*>(w->parent());
		GameWidget *newGame = new GameWidget(server, parentStack);
		app->setGameWidget(newGame);
		parentStack->setCurrentWidget(newGame);

		app->triggerUpdate();
	}

}

