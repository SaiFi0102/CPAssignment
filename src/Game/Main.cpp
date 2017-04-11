#include "Application/WServer.h"
#include "Application/WApplication.h"

#include <Wt/WApplication>
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WText>


int main(int argc, char *argv[])
{
	/* *************************************************************************
	* *************************  Start Web Server *****************************
	* *************************************************************************/
	
	DB::WServer Server(argc, argv);
	try
	{
		//Initialize
		Server.initialize();
		
		//Configuration
		Server.addEntryPoint(Wt::Application, &DB::WApplication::createApplication);

		//Start Server
		if(Server.start())
		{
			//And wait till a shutdown signal is given
			int sig = Wt::WServer::waitForShutdown(argv[0]);
			Wt::log("gs-info") << "Shutdown (Signal = " << sig << ")";
			Server.stop();
		}
	}
	catch(const Wt::WServer::Exception &e)
	{
		Wt::log("fatal") << "Error starting the server: " << e.what();
		exit(EXIT_FAILURE);
	}
	catch(const std::exception &e)
	{
		Wt::log("fatal") << "Server exception error: " << e.what();
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
