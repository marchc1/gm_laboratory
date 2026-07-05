#include "native/laboratory/laboratory.h"

#include <cfloat>
#include <cmath>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"
#include "implot3d.h"

#include "native/log.h"

namespace gm_laboratory {

	namespace {

		struct LabApp {
			LabAppDesc desc;
			bool open;
			float launchT;
		};

		std::vector<LabApp>& Apps() {
			static std::vector<LabApp> apps;
			return apps;
		}

		bool g_builtinsRegistered = false;

		const float kIconSize = 48.0f;
		const float kIconSpacing = 14.0f;
		const float kPadding = 12.0f;
		const float kJumpHeight = 16.0f;
		const float kLaunchTime = 0.5f;

		void DrawWelcome(bool* p_open) {
			if (ImGui::Begin("Laboratory", p_open)) {
				ImGui::TextUnformatted("gm_laboratory");
				ImGui::Separator();
				ImGui::TextWrapped("Open tools from the dock below. Windows can be docked to each other or floated freely inside the game window.");
			}
			ImGui::End();
		}

		void DrawImGuiDemo(bool* p_open) {
			ImGui::ShowDemoWindow(p_open);
		}

		void DrawImPlotDemo(bool* p_open) {
			ImPlot::ShowDemoWindow(p_open);
		}

		void DrawImPlot3DDemo(bool* p_open) {
			ImPlot3D::ShowDemoWindow(p_open);
		}

		void RegisterBuiltins() {
			if (g_builtinsRegistered)
				return;
			g_builtinsRegistered = true;

			LabAppDesc d{};
			d.id = "core.welcome";
			d.title = "Laboratory";
			d.shortLabel = "Lab";
			d.accent = IM_COL32(96, 148, 226, 255);
			d.draw = &DrawWelcome;
			d.startOpen = true;
			Laboratory::Register(&d);

			d = LabAppDesc{};
			d.id = "core.imgui_demo";
			d.title = "Dear ImGui Demo";
			d.shortLabel = "Gui";
			d.accent = IM_COL32(126, 128, 140, 255);
			d.draw = &DrawImGuiDemo;
			Laboratory::Register(&d);

			d = LabAppDesc{};
			d.id = "core.implot_demo";
			d.title = "ImPlot Demo";
			d.shortLabel = "2D";
			d.accent = IM_COL32(226, 150, 70, 255);
			d.draw = &DrawImPlotDemo;
			Laboratory::Register(&d);

			d = LabAppDesc{};
			d.id = "core.implot3d_demo";
			d.title = "ImPlot3D Demo";
			d.shortLabel = "3D";
			d.accent = IM_COL32(96, 194, 132, 255);
			d.draw = &DrawImPlot3DDemo;
			Laboratory::Register(&d);
		}

		void GuardedDraw(LabApp& app) {
			ImGuiContext* ctx = ImGui::GetCurrentContext();
			int depth = ctx->CurrentWindowStack.Size;
			try {
				app.desc.draw(&app.open);
			} catch (...) {
				app.open = false;
				Log("laboratory", "App '%s' threw during draw; closing it\n", app.desc.id ? app.desc.id : "?");
			}
			while (ctx->CurrentWindowStack.Size > depth)
				ImGui::End();
		}

		void DrawIcon(ImDrawList* dl, ImVec2 a0, ImVec2 a1, const LabAppDesc& desc) {
			float rounding = (a1.y - a0.y) * 0.22f;

			if (desc.icon) {
				dl->AddImageRounded(desc.icon, a0, a1, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE, rounding);
				dl->AddRect(a0, a1, IM_COL32(0, 0, 0, 90), rounding, 0, 1.0f);
				return;
			}

			ImU32 accent = desc.accent ? desc.accent : IM_COL32(126, 128, 140, 255);
			float h = a1.y - a0.y;

			dl->AddRectFilled(a0, a1, accent, rounding);

			ImVec2 glossMax(a1.x, a0.y + h * 0.46f);
			dl->AddRectFilled(a0, glossMax, IM_COL32(255, 255, 255, 115), rounding, ImDrawFlags_RoundCornersTop);

			ImVec2 shadeMin(a0.x, a0.y + h * 0.58f);
			dl->AddRectFilled(shadeMin, a1, IM_COL32(0, 0, 0, 55), rounding, ImDrawFlags_RoundCornersBottom);

			dl->AddRect(a0, a1, IM_COL32(255, 255, 255, 70), rounding, 0, 1.0f);
			dl->AddRect(ImVec2(a0.x + 1, a0.y + 1), ImVec2(a1.x - 1, a1.y - 1), IM_COL32(0, 0, 0, 70), rounding, 0, 1.0f);

			if (desc.shortLabel) {
				ImFont* font = ImGui::GetFont();
				float fontSize = ImGui::GetFontSize() * 1.45f;
				ImVec2 ts = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, desc.shortLabel);
				ImVec2 c((a0.x + a1.x) * 0.5f - ts.x * 0.5f, (a0.y + a1.y) * 0.5f - ts.y * 0.5f);
				dl->AddText(font, fontSize, ImVec2(c.x + 1, c.y + 1), IM_COL32(0, 0, 0, 150), desc.shortLabel);
				dl->AddText(font, fontSize, c, IM_COL32(255, 255, 255, 240), desc.shortLabel);
			}
		}

		void DrawDock() {
			std::vector<LabApp>& apps = Apps();
			if (apps.empty())
				return;

			const ImGuiViewport* vp = ImGui::GetMainViewport();
			ImDrawList* fg = ImGui::GetForegroundDrawList();
			float dt = ImGui::GetIO().DeltaTime;

			int count = static_cast<int>(apps.size());
			float contentW = count * kIconSize + (count - 1) * kIconSpacing;
			float dockW = contentW + kPadding * 2.0f;
			float dockH = kIconSize + kPadding * 2.0f;

			ImVec2 anchor(vp->WorkPos.x + vp->WorkSize.x * 0.5f, vp->WorkPos.y + vp->WorkSize.y - 14.0f);
			ImGui::SetNextWindowPos(ImVec2(anchor.x, anchor.y), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
			ImGui::SetNextWindowSize(ImVec2(dockW, dockH), ImGuiCond_Always);

			ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
				ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBringToFrontOnFocus;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			if (ImGui::Begin("##gmlab_dock", nullptr, flags)) {
				ImVec2 wp = ImGui::GetWindowPos();
				ImVec2 ws = ImGui::GetWindowSize();

				ImVec2 p0 = wp;
				ImVec2 p1 = ImVec2(wp.x + ws.x, wp.y + ws.y);
				float panelRound = 16.0f;
				fg->AddRectFilled(p0, p1, IM_COL32(26, 26, 30, 190), panelRound);
				fg->AddRectFilled(p0, ImVec2(p1.x, p0.y + ws.y * 0.5f), IM_COL32(255, 255, 255, 20), panelRound, ImDrawFlags_RoundCornersTop);
				fg->AddRect(p0, p1, IM_COL32(255, 255, 255, 45), panelRound, 0, 1.5f);

				float x = wp.x + kPadding;
				float baseY = wp.y + kPadding;

				for (int i = 0; i < count; ++i) {
					LabApp& app = apps[i];

					ImGui::SetCursorScreenPos(ImVec2(x, baseY));
					ImGui::PushID(i);
					ImGui::InvisibleButton("icon", ImVec2(kIconSize, kIconSize));
					bool hovered = ImGui::IsItemHovered();
					if (ImGui::IsItemClicked()) {
						app.open = !app.open;
						if (app.open)
							app.launchT = kLaunchTime;
					}
					ImGui::PopID();

					float jump = 0.0f;
					if (app.launchT > 0.0f) {
						app.launchT -= dt;
						if (app.launchT < 0.0f)
							app.launchT = 0.0f;
						float t = 1.0f - (app.launchT / kLaunchTime);
						float p = (t < 0.5f) ? (t * 2.0f) : (2.0f - t * 2.0f);
						jump = kJumpHeight * sinf(p * 1.57079633f);
					}

					float scale = hovered ? 1.12f : 1.0f;
					float half = kIconSize * 0.5f * scale;
					ImVec2 center(x + kIconSize * 0.5f, baseY + kIconSize * 0.5f - jump);
					ImVec2 a0(center.x - half, center.y - half);
					ImVec2 a1(center.x + half, center.y + half);
					DrawIcon(fg, a0, a1, app.desc);

					if (app.open) {
						ImVec2 dot(x + kIconSize * 0.5f, wp.y + ws.y - 5.0f);
						fg->AddCircleFilled(dot, 2.5f, IM_COL32(255, 255, 255, 220));
					}

					if (hovered && app.desc.title) {
						ImVec2 tsize = ImGui::CalcTextSize(app.desc.title);
						float px = 8.0f;
						float py = 4.0f;
						float cx = x + kIconSize * 0.5f;
						float bottom = baseY - 12.0f;
						ImVec2 bmin(cx - tsize.x * 0.5f - px, bottom - tsize.y - py * 2.0f);
						ImVec2 bmax(cx + tsize.x * 0.5f + px, bottom);
						fg->AddRectFilled(bmin, bmax, IM_COL32(18, 18, 22, 235), 5.0f);
						fg->AddRect(bmin, bmax, IM_COL32(255, 255, 255, 40), 5.0f);
						fg->AddText(ImVec2(cx - tsize.x * 0.5f, bmin.y + py), IM_COL32(255, 255, 255, 240), app.desc.title);
					}

					x += kIconSize + kIconSpacing;
				}
			}
			ImGui::End();
			ImGui::PopStyleVar();
		}

	}

	void Laboratory::Register(const LabAppDesc* desc) {
		if (!desc || !desc->draw)
			return;
		LabApp app;
		app.desc = *desc;
		app.open = desc->startOpen;
		app.launchT = 0.0f;
		Apps().push_back(app);
	}

	void Laboratory::Init() {
		RegisterBuiltins();
	}

	void Laboratory::Render() {
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

		for (LabApp& app : Apps()) {
			if (app.open)
				GuardedDraw(app);
		}

		DrawDock();
	}

}
