#include "native/module_api.h"
#include "defs/module_help.h"

#include "imgui.h"
#include "implot.h"
#include "implot3d.h"

#include "convar.h"

using namespace gm_laboratory;

MODULE_START()
static bool g_showDemos = false;

static void ToggleDemos(const CCommand& args) {
	g_showDemos = !g_showDemos;
}
static ConCommand gmlab_demos("gmlab_demos", ToggleDemos, "Toggle the Dear ImGui, ImPlot and ImPlot3D demo windows");

static void OnImGuiFrame() {
	if (!g_showDemos)
		return;

	ImGui::ShowDemoWindow();
	ImPlot::ShowDemoWindow();
	ImPlot3D::ShowDemoWindow();
}

MODULE_MAIN() {
	g_api->Log("example6", "Loaded Example Module #6: ImGui / ImPlot / ImPlot3D (abi %u)\n", api->abiVersion);
	MODULE_IMGUI_CONNECT();
	MODULE_IMPLOT_CONNECT();
	MODULE_IMPLOT3D_CONNECT();
	g_api->OnImGuiFrame(&OnImGuiFrame);
} MODULE_END()
