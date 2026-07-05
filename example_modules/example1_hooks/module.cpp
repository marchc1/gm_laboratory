#include "native/module_api.h"
#include "defs/module_help.h"

using namespace gm_laboratory;

MODULE_START()
static void PreBootAppSystemCreate() {
	g_api->Log("example1", "PreBootAppSystemCreate fired\n");
}

static void PostLoadLibrary(const char* filename, void* /*returned*/) {
	g_api->Log("example1", "library loaded: %s\n", filename);
}

MODULE_MAIN() {
	g_api->Log("example1", "Loaded Example Module #1: Using Provided Laboratory Hooks (abi %u)\n", api->abiVersion);
	g_api->PreBootAppSystemCreate(&PreBootAppSystemCreate);
	g_api->PostLoadLibrary(&PostLoadLibrary);
} MODULE_END()