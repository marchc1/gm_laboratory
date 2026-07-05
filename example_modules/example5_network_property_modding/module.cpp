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
	g_api->Log("example", "%s.%s <- %d (object %d)\n",
		kTargetClass, kTargetProp, pData->m_Value.m_Int, pData->m_ObjectID);
	if (g_originalHealthProxy)
		g_originalHealthProxy(pData, pStruct, pOut);
}

static void RewriteClientClasses(ClientClassEditor& editor) {
	g_api->Log("example", "props on %s:\n", kTargetClass);
	editor.ForEachProp(kTargetClass, [](const char* name, int type, int offset, int flags) {
		g_api->Log("example", "  %s : %s (offset %d, flags 0x%X)\n", name, NetPropTypeName(type), offset, flags);
		});

	RecvPropHandle health = editor.GetProp(kTargetClass, kTargetProp);
	if (health.Valid) {
		g_originalHealthProxy = health.OriginalProxy();
		health.RerouteProxy(&HealthRecvProxy);
		g_api->Log("example", "rerouted %s.%s recv proxy\n", kTargetClass, kTargetProp);
	}
	else {
		g_api->Log("example", "could not find %s.%s -- available client classes:\n", kTargetClass, kTargetProp);
		editor.ForEachClass([](const char* className) {
			g_api->Log("example", "  %s\n", className);
			});
	}
}

MODULE_MAIN() {
	g_api->Log("example", "Loaded Example Module #5: Modifying RecvProp/SendProps (abi %u)\n", api->abiVersion);
	api->AddClientClassRewriter(&RewriteClientClasses);
} MODULE_END()
