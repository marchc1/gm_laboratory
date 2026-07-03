#pragma once
#include "defs/sourcesdk_help.h"
#include "native/module_api.h"

#define MODULE_SOURCE_SDK_CONNECT()	SOURCE_SDK_CONNECT()

#define MODULE_START() static const ModuleAPI* g_api = nullptr;
#define MODULE_MAIN() extern "C" __declspec(dllexport) void ModuleMain(const ModuleAPI* api) { \
	g_api = api; \

#define MODULE_END() }