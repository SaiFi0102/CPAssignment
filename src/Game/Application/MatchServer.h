#ifndef SM_MATCHSERVER_H
#define SM_MATCHSERVER_H

#include <boost/thread/mutex.hpp>
#include <Wt/Dbo/Session>
#include <Wt/WString>

namespace SM
{
	struct Match;
	struct Client;
	struct MatchMakingClient;
	typedef std::vector<Client*> ClientVector;
	typedef std::vector<MatchMakingClient*> MMClientVector;
	typedef std::vector<Match*> MatchVector;

	struct Client
	{
		Client(const std::string &sessionId, long long authInfoId) : sessionId(sessionId), authInfoId(authInfoId) { }
		std::string sessionId;
		long long authInfoId;
	};
	struct MatchMakingClient : public Client
	{
		MatchMakingClient(const std::string &sessionId, long long authInfoId) : Client(sessionId, authInfoId) { }
		Match *currentMatch = nullptr;
		bool ready = false;
	};

	struct Match
	{
		Match(long long matchId) : matchId(matchId) { }
		long long matchId;
		Wt::WString matchName;
		MMClientVector clientList;
	};

	class MatchMakingServer
	{
	public:
		typedef std::vector<long long> IdVector;

		MatchMakingServer(Wt::Dbo::Session &session);
		~MatchMakingServer();
		void connect();
		void disconnect();
		void createMatch();
		void joinMatch(long long matchId);
		void readyMatch();
		void leaveMatch();

	protected:
		typedef std::map<std::string, MatchMakingClient*> SessionToClientMap;
		typedef std::map<long long, MatchMakingClient*> AuthInfoToClientMap;

		void startGame(Match *match);

		void _connect(const std::string &sessionId, long long authInfoId);
		void _disconnect(const std::string &sessionId);
		void _createMatch(const std::string &sessionId, const Wt::WString &matchName);
		void _joinMatch(const std::string &sessionId, long long matchId);
		void _readyMatch(const std::string &sessionId);
		void _leaveMatch(const std::string &sessionId);

		SessionToClientMap _sessionClientMap;
		AuthInfoToClientMap _authInfoClientMap;
		MatchVector _matchList;
		long long _nextMatchId = -1;
		Wt::Dbo::Session &dboSession;

	private:
		mutable boost::mutex _mutex;
	};
}

#endif