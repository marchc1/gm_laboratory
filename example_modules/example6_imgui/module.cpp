#include "native/module_api.h"
#include "defs/module_help.h"

#include "imgui.h"

using namespace gm_laboratory;

MODULE_START()
static void OnImGuiFrame() {
	ImGui::Begin("Example Module #6");
	ImGui::Text("Hello from a gm_laboratory module!");
	ImGui::End();
}

MODULE_MAIN() {
	g_api->Log("example6", "Loaded Example Module #6: ImGui (abi %u)\n", api->abiVersion);
	MODULE_IMGUI_CONNECT();
	g_api->OnImGuiFrame(&OnImGuiFrame);
} MODULE_END()
