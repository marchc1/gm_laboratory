#pragma once
#include "defs/sourcesdk_help.h"
#include "native/module_api.h"

#define MODULE_SOURCE_SDK_CONNECT()	g_api->PostHostInit([](bool) { SOURCE_SDK_CONNECT(); });

#define MODULE_IMGUI_CONNECT() g_api->OnImGuiInit([]() { \
	ImGui::SetCurrentContext(g_api->GetImGuiContext()); \
	ImGuiMemAllocFunc allocFn; ImGuiMemFreeFunc freeFn; void* userData; \
	g_api->GetImGuiAllocatorFns(&allocFn, &freeFn, &userData); \
	ImGui::SetAllocatorFunctions(allocFn, freeFn, userData); \
});

#define MODULE_START() static const ModuleAPI* g_api = nullptr;
#define MODULE_MAIN() extern "C" __declspec(dllexport) void ModuleMain(const ModuleAPI* api) { \
	g_api = api; \
	MODULE_SOURCE_SDK_CONNECT()
#define MODULE_END() }