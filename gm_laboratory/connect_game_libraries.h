#pragma once

class IBaseClientDLL;
class IServerGameDLL;

namespace gm_laboratory {
	extern IBaseClientDLL* g_ClientDLL;
	extern IServerGameDLL* g_ServerDLL;

	void ConnectGameLibraries();
}