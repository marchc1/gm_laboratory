#include "native/module_api.h"
#include "native/vtable_editor.h"
#include "defs/module_help.h"
#include "defs/dll_names.h"

#include "inetchannel.h"

using namespace gm_laboratory;

MODULE_START()

using SendDatagramFn = int (*)(void* self, void* data);
static SendDatagramFn g_originalSendDatagram = nullptr;
static int g_sendCount = 0;

static int SendDatagram_Detour(void* self, void* data) {
	if (g_sendCount < 5)
		g_api->Log("example4", "CNetChan::SendDatagram #%d on channel %p\n", g_sendCount, self);
	else if (g_sendCount == 5)
		g_api->Log("example4", "CNetChan::SendDatagram still firing -- silencing further logs\n");
	++g_sendCount;

	return g_originalSendDatagram(self, data);
}

static void InstallNetChannelHook(bool) {
	VTableEditor editor = GetVTable(ENGINE_DLL, "CNetChan");
	if (!editor.Valid) {
		g_api->Log("example4", "could not locate CNetChan vtable in %s\n", ENGINE_DLL);
		return;
	}

	g_originalSendDatagram = reinterpret_cast<SendDatagramFn>(editor.Hook(&INetChannel::SendDatagram, reinterpret_cast<void*>(&SendDatagram_Detour)));

	if (g_originalSendDatagram)
		g_api->Log("example4", "hooked CNetChan's INetChannel::SendDatagram vtable slot\n");
	else
		g_api->Log("example4", "failed to hook SendDatagram\n");
}

MODULE_MAIN() {
	g_api->Log("example4", "Loaded Example Module #4: Detouring via Virtual Function Table Overrides (abi %u)\n", api->abiVersion);
	api->PostHostInit(&InstallNetChannelHook);
} MODULE_END()
