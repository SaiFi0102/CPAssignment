#ifndef SM_WSERVER_H
#define SM_WSERVER_H

#include <Wt/WServer>

#include <Wt/Dbo/SqlConnectionPool>
#include <Wt/Dbo/Session>

#include <Wt/Auth/AuthService>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/OAuthService>

namespace SM
{
	class MatchMakingServer;
	typedef std::vector<const Wt::Auth::OAuthService*> OAuthServiceMap;

	class WServer : public Wt::WServer
	{
	public:
		WServer(int argc, char *argv[], const std::string &wtConfigurationFile = "");
		virtual ~WServer() override;
		void initialize();
		
		static WServer *instance() { return dynamic_cast<WServer*>(Wt::WServer::instance()); }
		bool start();

		Wt::Dbo::SqlConnectionPool *sqlPool() const { return _sqlPool; }

		const Wt::Auth::AuthService &getAuthService() const { return _authService; }
		const Wt::Auth::PasswordService &getPasswordService() const { return _passwordService; }
		const OAuthServiceMap &getOAuthServices() const { return _oAuthServices; }

		MatchMakingServer *matchMakingServer() const { return _matchMakingServer; }

	protected:
		void configureAuth();

		Wt::Dbo::Session _dboSession;
		Wt::Dbo::SqlConnectionPool *_sqlPool = nullptr;

		MatchMakingServer *_matchMakingServer = nullptr;

		Wt::Auth::AuthService _authService;
		Wt::Auth::PasswordService _passwordService;
		OAuthServiceMap _oAuthServices;

		boost::posix_time::ptime _ptBeforeLoad;
		boost::posix_time::ptime _ptStart;
	};
}

#define SERVER SM::WServer::instance()

#endif
