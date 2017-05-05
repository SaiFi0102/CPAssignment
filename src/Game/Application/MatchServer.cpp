#include "Application/MatchServer.h"
#include "Application/GameServer.h"
#include "Application/WApplication.h"
#include "Application/WServer.h"
#include "Widgets/MatchFinder.h"

#include <Wt/WLogger>
#include <boost/thread/lock_guard.hpp>

//boost::lock_guard<boost::mutex> lock(_mutex); //lock

namespace DB
{

	MatchMakingServer::MatchMakingServer(Wt::Dbo::Session &session)
		: dboSession(session)
	{ }

	MatchMakingServer::~MatchMakingServer()
	{
		for(const auto &it : _sessionClientMap)
			delete it.second;
		for(const auto &server : _gameServers)
			delete server;
	}

	void MatchMakingServer::connect()
	{
		boost::lock_guard<boost::mutex> lock(_mutex);

		WApplication *app = APP;
		_connect(app->sessionId(), app->authLogin().authInfoPtr().id());
	}

	void MatchMakingServer::_connect(const std::string &sessionId, long long authInfoId)
	{
		MatchMakingClient *client = new MatchMakingClient(sessionId, authInfoId);
		_sessionClientMap.insert(std::make_pair(sessionId, client));
		_authInfoClientMap.insert(std::make_pair(authInfoId, client));
		
		//Copy authInfoIds only into a vector
		IdVector onlineAuthInfoIds;
		for(const auto &c : _sessionClientMap)
			onlineAuthInfoIds.push_back(c.second->authInfoId);

		//Notify client that is connected
		SERVER->post(sessionId, boost::bind(&MatchFinder::_connected, _matchList, onlineAuthInfoIds));

		//Broadcast to all other clients about new client
		for(const auto &c : _sessionClientMap)
		{
			if(c.second != client)
				SERVER->post(c.second->sessionId, boost::bind(&MatchFinder::_playerConnected, authInfoId));
		}
	}

	void MatchMakingServer::disconnect()
	{
		boost::lock_guard<boost::mutex> lock(_mutex);

		WApplication *app = APP;
		_disconnect(app->sessionId());
	}

	void MatchMakingServer::_disconnect(const std::string &sessionId)
	{
		SessionToClientMap::const_iterator fItr = _sessionClientMap.find(sessionId);
		if(fItr == _sessionClientMap.end())
			return;

		MatchMakingClient *client = fItr->second;
		if(client->currentMatch)
			_leaveMatch(sessionId);

		_sessionClientMap.erase(sessionId);
		_authInfoClientMap.erase(client->authInfoId);

		//Notify client that it is disconnected
		SERVER->post(sessionId, boost::bind(&MatchFinder::_disconnected));

		//Broadcast to all other clients that a client has been disconnected	
		for(const auto &c : _sessionClientMap)
			SERVER->post(c.second->sessionId, boost::bind(&MatchFinder::_playerDisconnected, client->authInfoId));

		delete client;
	}

	void MatchMakingServer::createMatch()
	{
		boost::lock_guard<boost::mutex> lock(_mutex);

		WApplication *app = APP;
		TRANSACTION(app);
		_createMatch(app->sessionId(), app->authLogin().authInfoPtr()->identity(Wt::Auth::Identity::LoginName));
	}

	void MatchMakingServer::_createMatch(const std::string &sessionId, const Wt::WString &matchName)
	{
		SessionToClientMap::const_iterator fItr = _sessionClientMap.find(sessionId);
		if(fItr == _sessionClientMap.end())
			return;

		MatchMakingClient *creatorClient = fItr->second;
		Match *newMatch = new Match(++_nextMatchId);
		newMatch->matchName = APP->authLogin().authInfoPtr()->identity(Wt::Auth::Identity::LoginName);
		newMatch->clientList.push_back(creatorClient);
		_matchList.push_back(newMatch);
		creatorClient->currentMatch = newMatch;

		//Notify client
		SERVER->post(sessionId, boost::bind(&MatchFinder::_matchJoined, newMatch));

		//Broadcast
		for(const auto &c : _sessionClientMap)
			SERVER->post(c.second->sessionId, boost::bind(&MatchFinder::_matchAdded, newMatch));
	}

	void MatchMakingServer::joinMatch(long long matchId)
	{
		boost::lock_guard<boost::mutex> lock(_mutex);

		WApplication *app = APP;
		_joinMatch(app->sessionId(), matchId);
	}

	void MatchMakingServer::_joinMatch(const std::string &sessionId, long long matchId)
	{
		SessionToClientMap::const_iterator fItr = _sessionClientMap.find(sessionId);
		if(fItr == _sessionClientMap.end())
			return;

		MatchMakingClient *client = fItr->second;
		if(client->currentMatch)
			return;

		Match *match = nullptr;
		for(const auto &c : _matchList)
		{
			if(c->matchId == matchId)
			{
				match = c;
				break;
			}
		}
		if(!match)
			return;

		match->clientList.push_back(client);
		client->currentMatch = match;

		//Notify client
		SERVER->post(sessionId, boost::bind(&MatchFinder::_matchJoined, match));

		//Broadcast to all other joined players
		for(const auto &c : match->clientList)
		{
			if(c != client)
				SERVER->post(c->sessionId, boost::bind(&MatchFinder::_matchPlayerAdded, client->authInfoId));
		}
	}

	void MatchMakingServer::leaveMatch()
	{
		boost::lock_guard<boost::mutex> lock(_mutex);

		WApplication *app = APP;
		_leaveMatch(app->sessionId());
	}

	void MatchMakingServer::_leaveMatch(const std::string &sessionId)
	{
		SessionToClientMap::const_iterator fItr = _sessionClientMap.find(sessionId);
		if(fItr == _sessionClientMap.end())
			return;

		MatchMakingClient *client = fItr->second;
		Match *match = client->currentMatch;
		if(!match)
			return;

		//Find client in match
		MMClientVector::const_iterator matchClientItr = std::find(match->clientList.begin(), match->clientList.end(), client);
		if(matchClientItr == match->clientList.end())
			return;

		//Remove player and broadcast to players in the match
		match->clientList.erase(matchClientItr);
		client->currentMatch = nullptr;
		for(const auto &c : match->clientList)
			SERVER->post(c->sessionId, boost::bind(&MatchFinder::_matchPlayerRemoved, client->authInfoId));
		
		//If no clients left, remove the match
		if(match->clientList.empty())
		{
			//Broadcast match removed
			for(const auto &c : _sessionClientMap)
				SERVER->post(c.second->sessionId, boost::bind(&MatchFinder::_matchRemoved, match->matchId));

			MatchVector::const_iterator matchListItr = std::find(_matchList.begin(), _matchList.end(), match);
			_matchList.erase(matchListItr);
			delete match;
		}

		//Notify client
		SERVER->post(sessionId, boost::bind(&MatchFinder::_matchLeft));
	}

	void MatchMakingServer::readyMatch()
	{
		boost::lock_guard<boost::mutex> lock(_mutex);

		WApplication *app = APP;
		_readyMatch(app->sessionId());
	}

	void MatchMakingServer::_readyMatch(const std::string &sessionId)
	{
		SessionToClientMap::const_iterator fItr = _sessionClientMap.find(sessionId);
		if(fItr == _sessionClientMap.end())
			return;

		MatchMakingClient *client = fItr->second;
		Match *match = client->currentMatch;
		if(!match)
			return;

		//Find client in match
		MMClientVector::const_iterator matchClientItr = std::find(match->clientList.begin(), match->clientList.end(), client);
		if(matchClientItr == match->clientList.end())
			return;

		client->ready = !client->ready;

		//Broadcast player's ready state
		for(const auto &c : match->clientList)
			SERVER->post(c->sessionId, boost::bind(&MatchFinder::_matchPlayerReadyChanged, client->authInfoId, client->ready));

		//Check if every player is ready
		bool allReady = true;
		if(client->ready)
		{
			for(const auto &c : match->clientList)
			{
				if(!c->ready)
				{
					allReady = false;
					break;
				}
			}
		}

		//If all are ready start the game
		if(allReady)
			startGame(match);
	}

	void MatchMakingServer::startGame(Match *match)
	{
		//Create and connect clients to game server
		GameServer *gameServer = new GameServer(match->matchName, match->clientList, dboSession);
		_gameServers.push_back(gameServer);

		//Notify clients that the game has begun
		for(const auto &c : match->clientList)
			SERVER->post(c->sessionId, boost::bind(&MatchFinder::_gameStarted, gameServer));

		//Disconnect clients, will also delete match
		MMClientVector tempClientVector(match->clientList);
		for(const auto &c : tempClientVector)
			_disconnect(c->sessionId);
	}

}
