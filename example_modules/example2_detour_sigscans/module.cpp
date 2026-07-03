#include "native/module_api.h"
#include "defs/module_help.h"

using namespace gm_laboratory;

MODULE_START()
MODULE_MAIN()
g_api->Log("example", "Loaded Example Module #2: Detouring via Signature Scanning (abi %u)\n", api->abiVersion);
g_api->Log("example", "(TODO: ^^ finish this module...)\n", api->abiVersion);

MODULE_END()