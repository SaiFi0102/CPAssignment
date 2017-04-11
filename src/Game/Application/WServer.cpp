#include "Application/WServer.h"

#include <Wt/WMessageResourceBundle>
#include <Wt/Dbo/FixedSqlConnectionPool>
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/Dbo/Exception>

namespace DB
{
	WServer::WServer(int argc, char *argv[], const std::string &wtConfigurationFile)
		: Wt::WServer(argv[0], wtConfigurationFile)
	{
		_ptBeforeLoad = boost::posix_time::microsec_clock::local_time();
		setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
	}

	WServer::~WServer()
	{
		delete _sqlPool; //Also deletes SQLConnections
		delete localizedStrings();
	}

	void WServer::initialize()
	{
		auto resolver = new Wt::WMessageResourceBundle();
		resolver->use(appRoot() + "strings");
		setLocalizedStrings(resolver);

		/* *************************************************************************
		* ***********************  Connect to SQL Server  *************************
		* *************************************************************************/
		try
		{
			log("db-info") << "Connecting to database backend";

			Wt::Dbo::SqlConnection *sqlConnection = new Wt::Dbo::backend::Sqlite3(":memory:");
			sqlConnection->setProperty("show-queries", "true");
			_sqlPool = new Wt::Dbo::FixedSqlConnectionPool(sqlConnection, 1);

			log("success") << "Successfully connected to database";
		}
		catch (const Wt::Dbo::Exception &e)
		{
			log("fatal") << "Database error connecting to database: " << e.what();
			throw;
		}
		catch (const std::exception &e)
		{
			log("fatal") << "Error connecting to database: " << e.what();
			throw;
		}

		_dboSession.setConnectionPool(*_sqlPool);
	}

	bool WServer::start()
	{
		if (Wt::WServer::start())
		{
			//Load Finish Time
			_ptStart = boost::posix_time::microsec_clock::local_time();

			log("success") << "Server successfully started! Time taken to start: "
				<< boost::posix_time::time_duration(_ptStart - _ptBeforeLoad).total_milliseconds()
				<< " ms";
			return true;
		}
		return false;
	}
}