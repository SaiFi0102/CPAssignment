#ifndef DB_WSERVER_H
#define DB_WSERVER_H

#include <Wt/WServer>

#include <Wt/Dbo/SqlConnectionPool>
#include <Wt/Dbo/Session>

namespace DB
{
	class WServer : public Wt::WServer
	{
	public:
		WServer(int argc, char *argv[], const std::string &wtConfigurationFile = "");
		virtual ~WServer() override;
		void initialize();
		
		static WServer *instance() { return dynamic_cast<WServer*>(Wt::WServer::instance()); }
		bool start();

		Wt::Dbo::SqlConnectionPool *sqlPool() const { return _sqlPool; }

	protected:
		Wt::Dbo::Session _dboSession;
		Wt::Dbo::SqlConnectionPool *_sqlPool = nullptr;

		boost::posix_time::ptime _ptBeforeLoad;
		boost::posix_time::ptime _ptStart;
	};
}

#define SERVER DB::WServer::instance()

#endif
