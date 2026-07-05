#include "native/laboratory/laboratory.h"

#include <cstring>
#include <string>
#include <vector>

#include <windows.h>

#include "imgui.h"

#include "interface.h"
#include "tier1/tier1.h"
#include "tier1/convar.h"
#include "icvar.h"
#include "cdll_int.h"

#include "native/interface_registry.h"
#include "defs/dll_names.h"
#include "util/connect_game_libraries.h"

namespace gm_laboratory {

	namespace {

		char s_filter[128] = "";
		char s_command[256] = "";
		std::vector<ConCommandBase*> s_results;
		bool s_dirty = true;

		const void* s_editing = nullptr;
		bool s_focusEdit = false;
		char s_editBuf[256] = "";

		void EnsureCvar() {
			if (g_pCVar)
				return;

			std::vector<CreateInterfaceFn> factories;
			WalkInterfaces(
				[&factories](const char*, void*, void* createInterfaceFn) {
					factories.push_back(reinterpret_cast<CreateInterfaceFn>(createInterfaceFn));
				},
				nullptr, nullptr);

			if (!factories.empty())
				ConnectTier1Libraries(factories.data(), static_cast<int>(factories.size()));
		}

		bool MatchesFilter(const char* name) {
			if (!s_filter[0])
				return true;
			if (!name)
				return false;

			std::string hay = name;
			std::string needle = s_filter;
			for (char& c : hay) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
			for (char& c : needle) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
			return hay.find(needle) != std::string::npos;
		}

		void Rebuild() {
			s_results.clear();
			if (!g_pCVar)
				return;
			ICvar::Iterator it(g_pCVar);
			for (it.SetFirst(); it.IsValid(); it.Next()) {
				ConCommandBase* cmd = it.Get();
				if (cmd && MatchesFilter(cmd->GetName()))
					s_results.push_back(cmd);
			}
			s_dirty = false;
		}

		void AppendFlag(std::string& out, int flags, int bit, const char* name) {
			if (flags & bit) {
				if (!out.empty())
					out += ' ';
				out += name;
			}
		}

		std::string FlagString(int flags) {
			std::string out;
			AppendFlag(out, flags, FCVAR_CHEAT, "CHEAT");
			AppendFlag(out, flags, FCVAR_REPLICATED, "REPLICATED");
			AppendFlag(out, flags, FCVAR_HIDDEN, "HIDDEN");
			AppendFlag(out, flags, FCVAR_DEVELOPMENTONLY, "DEV");
			AppendFlag(out, flags, FCVAR_GAMEDLL, "GAME");
			AppendFlag(out, flags, FCVAR_CLIENTDLL, "CLIENT");
			AppendFlag(out, flags, FCVAR_PROTECTED, "PROTECTED");
			AppendFlag(out, flags, FCVAR_SERVER_CAN_EXECUTE, "SV_EXEC");
			return out;
		}

		void DrawValueCell(ConCommandBase* cmd) {
			if (cmd->IsCommand()) {
				if (ImGui::SmallButton("Run")) {
					if (g_EngineCL)
						g_EngineCL->ClientCmd_Unrestricted(cmd->GetName());
				}
				return;
			}

			ConVar* cv = static_cast<ConVar*>(cmd);

			if (s_editing == cv) {
				ImGui::SetNextItemWidth(-60.0f);
				if (s_focusEdit) {
					ImGui::SetKeyboardFocusHere();
					s_focusEdit = false;
				}
				bool commit = ImGui::InputText("##edit", s_editBuf, sizeof(s_editBuf), ImGuiInputTextFlags_EnterReturnsTrue);
				bool deactivated = ImGui::IsItemDeactivated();
				if (commit) {
					cv->SetValue(s_editBuf);
					s_editing = nullptr;
				} else if (deactivated) {
					s_editing = nullptr;
				}
			} else {
				ImGui::SetNextItemWidth(-60.0f);
				char shown[256];
				_snprintf_s(shown, _TRUNCATE, "%s##val", cv->GetString());
				if (ImGui::Selectable(shown, false)) {
					s_editing = cv;
					s_focusEdit = true;
					strncpy_s(s_editBuf, sizeof(s_editBuf), cv->GetString(), _TRUNCATE);
				}
			}

			ImGui::SameLine();
			if (ImGui::SmallButton("Reset"))
				cv->SetValue(cv->GetDefault());
		}

		void DrawCvars(bool* p_open) {
			ImGui::SetNextWindowSize(ImVec2(720, 480), ImGuiCond_FirstUseEver);
			if (!ImGui::Begin("Console Variables", p_open)) {
				ImGui::End();
				return;
			}

			EnsureCvar();

			if (!g_pCVar) {
				ImGui::TextUnformatted("Cvar system not available yet.");
				ImGui::End();
				return;
			}

			ImGui::SetNextItemWidth(-90.0f);
			bool runCommand = ImGui::InputText("##cmd", s_command, sizeof(s_command), ImGuiInputTextFlags_EnterReturnsTrue);
			ImGui::SameLine();
			ImGui::BeginDisabled(g_EngineCL == nullptr);
			if (ImGui::Button("Execute") || runCommand) {
				if (g_EngineCL && s_command[0]) {
					g_EngineCL->ClientCmd_Unrestricted(s_command);
					s_command[0] = '\0';
				}
			}
			ImGui::EndDisabled();
			if (!g_EngineCL)
				ImGui::TextDisabled("(engine command interface unavailable)");

			ImGui::SetNextItemWidth(-90.0f);
			if (ImGui::InputTextWithHint("##filter", "filter by name...", s_filter, sizeof(s_filter)))
				s_dirty = true;
			ImGui::SameLine();
			if (ImGui::Button("Refresh"))
				s_dirty = true;

			if (s_dirty)
				Rebuild();

			ImGui::Text("%d results", static_cast<int>(s_results.size()));

			ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
				ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY;
			if (ImGui::BeginTable("cvars", 3, tableFlags)) {
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 240.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Flags", ImGuiTableColumnFlags_WidthFixed, 160.0f);
				ImGui::TableHeadersRow();

				ImGuiListClipper clipper;
				clipper.Begin(static_cast<int>(s_results.size()));
				while (clipper.Step()) {
					for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
						ConCommandBase* cmd = s_results[row];
						ImGui::PushID(cmd);
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						ImGui::TextUnformatted(cmd->GetName());
						if (ImGui::IsItemHovered() && cmd->GetHelpText() && cmd->GetHelpText()[0])
							ImGui::SetTooltip("%s", cmd->GetHelpText());

						ImGui::TableSetColumnIndex(1);
						DrawValueCell(cmd);

						ImGui::TableSetColumnIndex(2);
						std::string flags = FlagString(cmd->GetFlags());
						if (!flags.empty())
							ImGui::TextDisabled("%s", flags.c_str());

						ImGui::PopID();
					}
				}
				ImGui::EndTable();
			}

			ImGui::End();
		}

	}

	void RegisterCvarBrowser() {
		LabAppDesc app{};
		app.id = "core.cvars";
		app.title = "Console Variables";
		app.shortLabel = "Var";
		app.accent = IM_COL32(150, 110, 210, 255);
		app.draw = &DrawCvars;
		Laboratory::Register(&app);
	}

}
