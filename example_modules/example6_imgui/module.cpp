#include "native/module_api.h"
#include "defs/module_help.h"

#include <cmath>

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

static void ExampleAppDraw(bool* p_open) {
	if (ImGui::Begin("Example Module App", p_open)) {
		ImGui::TextUnformatted("Registered from a module via RegisterLabApp.");
		ImGui::Separator();
		if (ImPlot::BeginPlot("Module Plot", ImVec2(-1, 200))) {
			static float xs[100], ys[100];
			for (int i = 0; i < 100; ++i) {
				xs[i] = i * 0.1f;
				ys[i] = sinf(xs[i]);
			}
			ImPlot::PlotLine("sin", xs, ys, 100);
			ImPlot::EndPlot();
		}
	}
	ImGui::End();
}

MODULE_MAIN() {
	g_api->Log("example6", "Loaded Example Module #6: ImGui / ImPlot / ImPlot3D (abi %u)\n", api->abiVersion);
	MODULE_IMGUI_CONNECT();
	MODULE_IMPLOT_CONNECT();
	MODULE_IMPLOT3D_CONNECT();
	g_api->OnImGuiFrame(&OnImGuiFrame);

	LabAppDesc app{};
	app.id = "example6.app";
	app.title = "Example Module App";
	app.shortLabel = "Ex6";
	app.accent = IM_COL32(206, 92, 122, 255);
	app.draw = &ExampleAppDraw;
	g_api->RegisterLabApp(&app);
} MODULE_END()
