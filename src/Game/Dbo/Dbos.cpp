#include "Dbos.h"

namespace SM
{
	void mapDboTree(Wt::Dbo::Session &dboSession)
	{
		dboSession.mapClass<User>("user");
		dboSession.mapClass<AuthInfo>("auth_info");
		dboSession.mapClass<AuthIdentity>("auth_identity");
		dboSession.mapClass<AuthToken>("auth_token");
		dboSession.mapClass<Game>("game");
	}
}
