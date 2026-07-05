#include "native/laboratory/laboratory.h"

#include <cfloat>
#include <cmath>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"
#include "implot3d.h"

#include "native/laboratory/icon_texture.h"
#include "native/log.h"

namespace gm_laboratory {

	namespace {

		struct LabApp {
			LabAppDesc desc;
			bool open;
			float launchT;
			float hoverT;
		};

		std::vector<LabApp>& Apps() {
			static std::vector<LabApp> apps;
			return apps;
		}

		bool g_builtinsRegistered = false;

	}

	void RegisterCvarBrowser();

	namespace {

		const float kIconSize = 48.0f;
		const float kIconSpacing = 16.0f;
		const float kSidePad = 26.0f;     
		const float kTopPad = 14.0f;      
		const float kBottomPad = 6.0f;
		const float kJumpHeight = 18.0f;
		const float kLaunchTime = 0.55f;
		const float kReflectFrac = 0.55f; 
		const float kReflectAlpha = 0.42f;
		const float kShelfFan = 1.25f;    
		const float kInset = 12.0f;        
		const float kHoverScale = 0.30f;  
		const float kHoverLift = 5.0f;    
		const float kHoverSpeed = 16.0f;  

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

			LabAppDesc d;

			RegisterCvarBrowser();

			d = LabAppDesc{};
			d.id = "core.imgui_demo";
			d.title = "Dear ImGui Demo";
			d.shortLabel = "Gui";
			d.accent = IM_COL32(126, 128, 140, 255);
			d.iconPath = "oxygen/preferences-system-windows.png";
			d.draw = &DrawImGuiDemo;
			Laboratory::Register(&d);

			d = LabAppDesc{};
			d.id = "core.implot_demo";
			d.title = "ImPlot Demo";
			d.shortLabel = "2D";
			d.accent = IM_COL32(226, 150, 70, 255);
			d.iconPath = "oxygen/kchart.png";
			d.draw = &DrawImPlotDemo;
			Laboratory::Register(&d);

			d = LabAppDesc{};
			d.id = "core.implot3d_demo";
			d.title = "ImPlot3D Demo";
			d.shortLabel = "3D";
			d.accent = IM_COL32(96, 194, 132, 255);
			d.iconPath = "oxygen/step.png";
			d.draw = &DrawImPlot3DDemo;
			Laboratory::Register(&d);
		}

		void GuardedDraw(LabApp& app) {
			ImGuiContext* ctx = ImGui::GetCurrentContext();
			int depth = ctx->CurrentWindowStack.Size;
			try {
				app.desc.draw(&app.open);
			}
			catch (...) {
				app.open = false;
				Log("laboratory", "App '%s' threw during draw; closing it\n", app.desc.id ? app.desc.id : "?");
			}
			while (ctx->CurrentWindowStack.Size > depth)
				ImGui::End();
		}

		ImU32 MulAlpha(ImU32 c, float m) {
			int a = static_cast<int>(((c >> IM_COL32_A_SHIFT) & 0xFF) * m);
			if (a < 0) a = 0;
			if (a > 255) a = 255;
			return (c & ~IM_COL32_A_MASK) | (static_cast<ImU32>(a) << IM_COL32_A_SHIFT);
		}

		void FillQuadV(ImDrawList* dl, ImVec2 tl, ImVec2 tr, ImVec2 br, ImVec2 bl, ImU32 top, ImU32 bot) {
			ImVec2 uv = ImGui::GetFontTexUvWhitePixel();
			dl->PrimReserve(6, 4);
			unsigned int i = dl->_VtxCurrentIdx;
			dl->PrimWriteIdx(static_cast<ImDrawIdx>(i));
			dl->PrimWriteIdx(static_cast<ImDrawIdx>(i + 1));
			dl->PrimWriteIdx(static_cast<ImDrawIdx>(i + 2));
			dl->PrimWriteIdx(static_cast<ImDrawIdx>(i));
			dl->PrimWriteIdx(static_cast<ImDrawIdx>(i + 2));
			dl->PrimWriteIdx(static_cast<ImDrawIdx>(i + 3));
			dl->PrimWriteVtx(tl, uv, top);
			dl->PrimWriteVtx(tr, uv, top);
			dl->PrimWriteVtx(br, uv, bot);
			dl->PrimWriteVtx(bl, uv, bot);
		}

		void AddImageVFade(ImDrawList* dl, ImTextureID tex, ImVec2 tl, ImVec2 br, float uvBot, float topA) {
			ImU32 top = IM_COL32(255, 255, 255, static_cast<int>(topA * 255.0f));
			ImU32 bot = IM_COL32(255, 255, 255, 0);
			dl->PushTexture(tex);
			dl->PrimReserve(6, 4);
			unsigned int i = dl->_VtxCurrentIdx;
			dl->PrimWriteIdx(static_cast<ImDrawIdx>(i));
			dl->PrimWriteIdx(static_cast<ImDrawIdx>(i + 1));
			dl->PrimWriteIdx(static_cast<ImDrawIdx>(i + 2));
			dl->PrimWriteIdx(static_cast<ImDrawIdx>(i));
			dl->PrimWriteIdx(static_cast<ImDrawIdx>(i + 2));
			dl->PrimWriteIdx(static_cast<ImDrawIdx>(i + 3));
			dl->PrimWriteVtx(ImVec2(tl.x, tl.y), ImVec2(0.0f, 1.0f), top);
			dl->PrimWriteVtx(ImVec2(br.x, tl.y), ImVec2(1.0f, 1.0f), top);
			dl->PrimWriteVtx(ImVec2(br.x, br.y), ImVec2(1.0f, uvBot), bot);
			dl->PrimWriteVtx(ImVec2(tl.x, br.y), ImVec2(0.0f, uvBot), bot);
			dl->PopTexture();
		}

		void DrawIcon(ImDrawList* dl, ImVec2 a0, ImVec2 a1, const LabAppDesc& desc) {
			float rounding = (a1.y - a0.y) * 0.225f;

			if (desc.icon) {
				dl->AddImageRounded(desc.icon, a0, a1, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE, rounding);
				return;
			}

			ImU32 accent = desc.accent ? desc.accent : IM_COL32(126, 128, 140, 255);
			float h = a1.y - a0.y;

			dl->AddRectFilled(a0, a1, accent, rounding);
			dl->AddRectFilled(a0, ImVec2(a1.x, a0.y + h * 0.46f),
				IM_COL32(255, 255, 255, 115), rounding, ImDrawFlags_RoundCornersTop);
			dl->AddRectFilled(ImVec2(a0.x, a0.y + h * 0.58f), a1,
				IM_COL32(0, 0, 0, 55), rounding, ImDrawFlags_RoundCornersBottom);
			dl->AddRect(a0, a1, IM_COL32(255, 255, 255, 70), rounding, 0, 1.0f);
			dl->AddRect(ImVec2(a0.x + 1, a0.y + 1), ImVec2(a1.x - 1, a1.y - 1),
				IM_COL32(0, 0, 0, 70), rounding, 0, 1.0f);

			if (desc.shortLabel) {
				ImFont* font = ImGui::GetFont();
				float fontSize = ImGui::GetFontSize() * 1.45f;
				ImVec2 ts = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, desc.shortLabel);
				ImVec2 c((a0.x + a1.x) * 0.5f - ts.x * 0.5f, (a0.y + a1.y) * 0.5f - ts.y * 0.5f);
				dl->AddText(font, fontSize, ImVec2(c.x + 1, c.y + 1), IM_COL32(0, 0, 0, 150), desc.shortLabel);
				dl->AddText(font, fontSize, c, IM_COL32(255, 255, 255, 240), desc.shortLabel);
			}
		}

		void DrawReflection(ImDrawList* dl, ImVec2 a0, ImVec2 a1, float floorY, float floorBottom, const LabAppDesc& desc) {
			float iconH = a1.y - a0.y;
			float rTop = 2.0f * floorY - a1.y;
			float avail = floorBottom - rTop;
			if (avail <= 1.0f || iconH <= 1.0f)
				return;
			float frac = avail / iconH;
			if (frac > 1.0f)
				frac = 1.0f;
			float rBot = rTop + iconH * frac;

			if (desc.icon) {
				AddImageVFade(dl, desc.icon, ImVec2(a0.x, rTop), ImVec2(a1.x, rBot), 1.0f - frac, kReflectAlpha);
				return;
			}
			ImU32 accent = desc.accent ? desc.accent : IM_COL32(126, 128, 140, 255);
			FillQuadV(dl, ImVec2(a0.x, rTop), ImVec2(a1.x, rTop), ImVec2(a1.x, rBot), ImVec2(a0.x, rBot),
				MulAlpha(accent, kReflectAlpha), accent & ~IM_COL32_A_MASK);
		}

		void DrawDock() {
			std::vector<LabApp>& apps = Apps();
			if (apps.empty())
				return;

			const ImGuiViewport* vp = ImGui::GetMainViewport();
			ImDrawList* fg = ImGui::GetForegroundDrawList();
			float dt = ImGui::GetIO().DeltaTime;

			int count = static_cast<int>(apps.size());
			float reflH = kIconSize * kReflectFrac;
			float contentW = count * kIconSize + (count - 1) * kIconSpacing;
			float lipHalf0 = contentW * 0.5f + kSidePad;
			float dockW = (lipHalf0 + reflH * kShelfFan) * 2.0f;
			float dockH = kTopPad + kIconSize + reflH + kBottomPad;

			ImVec2 anchor(vp->WorkPos.x + vp->WorkSize.x * 0.5f, vp->WorkPos.y + vp->WorkSize.y - 12.0f);
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

				float baseY = wp.y + kTopPad;
				float floorY = baseY + kIconSize;
				float floorBottom = floorY + reflH;
				float cxDock = wp.x + ws.x * 0.5f;
				float lipHalf = contentW * 0.5f + kSidePad;
				float backHalf = ws.x * 0.5f;

				ImVec2 lipL(cxDock - lipHalf, floorY);
				ImVec2 lipR(cxDock + lipHalf, floorY);
				ImVec2 backL(cxDock - backHalf, floorBottom);
				ImVec2 backR(cxDock + backHalf, floorBottom);

				FillQuadV(fg, lipL, lipR, backR, backL,
					IM_COL32(212, 226, 246, 90), IM_COL32(212, 226, 246, 30));
				fg->AddLine(lipL, backL, IM_COL32(255, 255, 255, 26), 1.0f);
				fg->AddLine(lipR, backR, IM_COL32(255, 255, 255, 26), 1.0f);

				struct Placed { ImVec2 a0, a1; float cx; bool hovered; };
				std::vector<Placed> placed(count);

				fg->PushClipRect(ImVec2(cxDock - backHalf, floorY), ImVec2(cxDock + backHalf, floorBottom), true);

				float x = cxDock - contentW * 0.5f;
				for (int i = 0; i < count; ++i) {
					LabApp& app = apps[i];

					if (!app.desc.icon && app.desc.iconPath)
						app.desc.icon = IconTexture::Load(app.desc.iconPath);

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

					float target = hovered ? 1.0f : 0.0f;
					app.hoverT += (target - app.hoverT) * (1.0f - expf(-kHoverSpeed * dt));
					float e = app.hoverT * app.hoverT * (3.0f - 2.0f * app.hoverT);

					float size = kIconSize * (1.0f + kHoverScale * e);
					float lift = jump + kHoverLift * e;
					float cx = x + kIconSize * 0.5f;
					ImVec2 a0(cx - size * 0.5f, floorY - size - lift + kInset);
					ImVec2 a1(cx + size * 0.5f, floorY - lift + kInset);
					placed[i] = Placed{ a0, a1, cx, hovered };

					DrawReflection(fg, a0, a1, floorY, floorBottom, app.desc);

					x += kIconSize + kIconSpacing;
				}
				fg->PopClipRect();

				fg->AddLine(ImVec2(backL.x + 3.0f, floorBottom), ImVec2(backR.x - 3.0f, floorBottom), IM_COL32(255, 255, 255, 165), 1.5f);
				fg->AddLine(ImVec2(backL.x + 3.0f, floorBottom + 1.5f), ImVec2(backR.x - 3.0f, floorBottom + 1.5f), IM_COL32(0, 0, 0, 55), 1.0f);

				for (int pass = 0; pass < 2; ++pass) {
					for (int i = 0; i < count; ++i) {
						const Placed& pl = placed[i];
						if (pl.hovered != (pass == 1))
							continue;
						DrawIcon(fg, pl.a0, pl.a1, apps[i].desc);
					}
				}

				for (int i = 0; i < count; ++i) {
					if (apps[i].open)
						fg->AddCircleFilled(ImVec2(placed[i].cx, floorBottom - 2.0f), 2.0f, IM_COL32(255, 255, 255, 210));
				}

				for (int i = 0; i < count; ++i) {
					const Placed& pl = placed[i];
					const LabApp& app = apps[i];
					if (!pl.hovered || !app.desc.title)
						continue;

					ImVec2 tsize = ImGui::CalcTextSize(app.desc.title);
					float px = 8.0f;
					float py = 4.0f;
					float bottom = pl.a0.y - 10.0f;
					ImVec2 bmin(pl.cx - tsize.x * 0.5f - px, bottom - tsize.y - py * 2.0f);
					ImVec2 bmax(pl.cx + tsize.x * 0.5f + px, bottom);
					fg->AddRectFilled(bmin, bmax, IM_COL32(18, 18, 22, 235), 5.0f);
					fg->AddRect(bmin, bmax, IM_COL32(255, 255, 255, 40), 5.0f);
					fg->AddText(ImVec2(pl.cx - tsize.x * 0.5f, bmin.y + py), IM_COL32(255, 255, 255, 240), app.desc.title);
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
		app.hoverT = 0.0f;
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
