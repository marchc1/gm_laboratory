#include "native/module_api.h"
#include "native/net_class_editor.h"
#include "defs/module_help.h"

#include "dt_common.h"

using namespace gm_laboratory;

MODULE_START()

static const char* kTargetClass = "CBasePlayer";
static const char* kTargetProp = "m_iHealth";

static RecvVarProxyFn g_originalHealthProxy = nullptr;

static void HealthRecvProxy(const CRecvProxyData* pData, void* pStruct, void* pOut) {
	g_api->Log("example", "%s.%s <- %d (object %d)\n", kTargetClass, kTargetProp, pData->m_Value.m_Int, pData->m_ObjectID);
	if (g_originalHealthProxy)
		g_originalHealthProxy(pData, pStruct, pOut);
}

static void RewriteClientClasses(ClientClassesEditor& editor) {
	ClientClassEditor cls = editor.GetClass(kTargetClass);
	if (!cls.Valid()) {
		g_api->Log("example", "could not find client class %s -- available:\n", kTargetClass);
		editor.ForEachClass([](const char* name) { g_api->Log("example", "  %s\n", name); });
		return;
	}

	g_api->Log("example", "props on %s:\n", kTargetClass);
	cls.ForEachProp([](const char* name, int type, int offset, int flags) {
		g_api->Log("example", "  %s : %s (offset %d, flags 0x%X)\n", name, NetPropTypeName(type), offset, flags);
		});

	RecvPropEditor health = cls.GetProp(kTargetProp);
	if (health.Valid()) {
		g_originalHealthProxy = health.OriginalProxy();
		health.RerouteProxy(&HealthRecvProxy);
		g_api->Log("example", "rerouted %s.%s recv proxy\n", kTargetClass, kTargetProp);
	}
	else {
		g_api->Log("example", "could not find %s.%s\n", kTargetClass, kTargetProp);
	}
}

static void RewriteServerClasses(ServerClassesEditor& editor) {
	ServerClassEditor entity = editor.GetClass("CBaseEntity");
	if (!entity.Valid())
		return;

	SendPropEditor origin = entity.GetProp("m_vecOrigin");
	if (origin.Valid()) {
		origin.SetFlags(0);
		origin.AddFlags(SPROP_NOSCALE);
		g_api->Log("example", "set CBaseEntity.m_vecOrigin to noscale (flags now 0x%X)\n", origin.Flags());
	}
}

MODULE_MAIN() {
	g_api->Log("example", "Loaded Example Module #5: Modifying RecvProp/SendProps (abi %u)\n", api->abiVersion);
	api->AddClientClassRewriter(&RewriteClientClasses);
	api->AddServerClassRewriter(&RewriteServerClasses);
} MODULE_END()
