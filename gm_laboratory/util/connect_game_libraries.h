#pragma once

#include "cdll_int.h"
#include "eiface.h"

class IBaseClientDLL;
class IServerGameDLL;

namespace gm_laboratory {
	extern IBaseClientDLL* g_ClientDLL;
	extern IVEngineClient013* g_EngineCL;

	extern IServerGameDLL* g_ServerDLL;
	extern IVEngineServer021* g_EngineSV;

	void ConnectGameLibraries();
}