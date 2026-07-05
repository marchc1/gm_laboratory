#include "native/overlay.h"

#include <windows.h>
#include <d3d9.h>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx9.h"

#include "native/detour_manager.h"
#include "native/log.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace gm_laboratory {

	using PresentFn = HRESULT(__stdcall*)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
	using ResetFn = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

	static PresentFn g_presentOriginal = nullptr;
	static ResetFn g_resetOriginal = nullptr;

	static ImGuiContext* g_context = nullptr;
	static HWND g_window = nullptr;
	static WNDPROC g_originalWndProc = nullptr;
	static bool g_backendsReady = false;
	static bool g_visible = true;

	struct GuardedCallback {
		void (*fn)();
		bool enabled;
	};

	static std::vector<GuardedCallback>& InitCallbacks() {
		static std::vector<GuardedCallback> callbacks;
		return callbacks;
	}

	static std::vector<GuardedCallback>& FrameCallbacks() {
		static std::vector<GuardedCallback> callbacks;
		return callbacks;
	}

	static void InvokeGuarded(std::vector<GuardedCallback>& callbacks, const char* phase) {
		ImGuiContext* ctx = ImGui::GetCurrentContext();
		for (GuardedCallback& cb : callbacks) {
			if (!cb.enabled)
				continue;

			int windowDepth = ctx->CurrentWindowStack.Size;
			try {
				cb.fn();
			} catch (...) {
				cb.enabled = false;
				Log("overlay", "Disabled an ImGui %s callback after it threw\n", phase);
			}

			while (ctx->CurrentWindowStack.Size > windowDepth)
				ImGui::End();
		}
	}

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_KEYDOWN && wParam == VK_INSERT)
			g_visible = !g_visible;

		if (g_context) {
			ImGui::SetCurrentContext(g_context);
			ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

			if (g_visible) {
				ImGuiIO& io = ImGui::GetIO();
				if (io.WantCaptureMouse && msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST)
					return TRUE;
				if (io.WantCaptureKeyboard && (msg == WM_KEYDOWN || msg == WM_KEYUP || msg == WM_CHAR || msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP))
					return TRUE;
			}
		}

		return CallWindowProcW(g_originalWndProc, hWnd, msg, wParam, lParam);
	}

	static void EnsureBackends(IDirect3DDevice9* device) {
		if (g_backendsReady)
			return;

		D3DDEVICE_CREATION_PARAMETERS params;
		if (FAILED(device->GetCreationParameters(&params))) {
			Log("overlay", "GetCreationParameters failed\n");
			return;
		}
		if (!params.hFocusWindow) {
			Log("overlay", "Device has no focus window\n");
			return;
		}
		g_window = params.hFocusWindow;

		if (!g_context) {
			g_context = ImGui::CreateContext();
			ImGui::StyleColorsDark();
			ImGui::GetIO().IniFilename = nullptr;
		}
		ImGui::SetCurrentContext(g_context);

		ImGui_ImplWin32_Init(g_window);
		ImGui_ImplDX9_Init(device);

		g_originalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(g_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WndProc)));

		g_backendsReady = true;
		Log("overlay", "ImGui backends initialised (hwnd %p)\n", g_window);

		InvokeGuarded(InitCallbacks(), "init");
	}

	static HRESULT __stdcall PresentDetour(IDirect3DDevice9* device, const RECT* src, const RECT* dst, HWND wnd, const RGNDATA* dirty) {
		static bool loggedFirstPresent = false;
		if (!loggedFirstPresent) {
			loggedFirstPresent = true;
			Log("overlay", "Present hook fired (device %p)\n", device);
		}

		EnsureBackends(device);

		if (g_backendsReady) {
			ImGui::SetCurrentContext(g_context);
			ImGui_ImplDX9_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			if (g_visible)
				ImGui::ShowDemoWindow();

			InvokeGuarded(FrameCallbacks(), "frame");

			ImGui::EndFrame();
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		}

		return g_presentOriginal(device, src, dst, wnd, dirty);
	}

	static HRESULT __stdcall ResetDetour(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) {
		if (g_backendsReady)
			ImGui_ImplDX9_InvalidateDeviceObjects();
		return g_resetOriginal(device, params);
	}

	struct ImGuiOverlaySetup : IImplementsDetours {
		void SetupWin64(DetourSetupContext& ctx) override {
			WNDCLASSEXA wc = {};
			wc.cbSize = sizeof(wc);
			wc.lpfnWndProc = DefWindowProcA;
			wc.hInstance = GetModuleHandleA(nullptr);
			wc.lpszClassName = "GMLabOverlayDummy";
			RegisterClassExA(&wc);

			HWND dummy = CreateWindowExA(0, wc.lpszClassName, "", WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, nullptr, nullptr, wc.hInstance, nullptr);
			if (!dummy) {
				Log("overlay", "Failed to create dummy window\n");
				return;
			}

			IDirect3D9* d3d = Direct3DCreate9(D3D_SDK_VERSION);
			if (!d3d) {
				Log("overlay", "Direct3DCreate9 failed\n");
				DestroyWindow(dummy);
				return;
			}

			D3DPRESENT_PARAMETERS pp = {};
			pp.Windowed = TRUE;
			pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
			pp.hDeviceWindow = dummy;
			pp.BackBufferWidth = 2;
			pp.BackBufferHeight = 2;
			pp.BackBufferFormat = D3DFMT_X8R8G8B8;
			pp.BackBufferCount = 1;

			IDirect3DDevice9* device = nullptr;
			HRESULT hr = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, dummy, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &device);
			if (FAILED(hr) || !device) {
				Log("overlay", "CreateDevice failed (0x%08lX)\n", static_cast<unsigned long>(hr));
				d3d->Release();
				DestroyWindow(dummy);
				return;
			}

			void** vtable = *reinterpret_cast<void***>(device);
			g_presentOriginal = reinterpret_cast<PresentFn>(ctx.AddDetourRaw(vtable[17], reinterpret_cast<void*>(&PresentDetour)));
			g_resetOriginal = reinterpret_cast<ResetFn>(ctx.AddDetourRaw(vtable[16], reinterpret_cast<void*>(&ResetDetour)));

			device->Release();
			d3d->Release();
			DestroyWindow(dummy);

			if (g_presentOriginal && g_resetOriginal)
				Log("overlay", "ImGui overlay hooks installed\n");
			else
				Log("overlay", "Failed to install ImGui overlay hooks\n");
		}
	};

	REGISTER_DETOUR(ImGuiOverlaySetup)

	void ImGuiOverlay::AddInitCallback(void (*callback)()) {
		if (callback)
			InitCallbacks().push_back({ callback, true });
	}

	void ImGuiOverlay::AddFrameCallback(void (*callback)()) {
		if (callback)
			FrameCallbacks().push_back({ callback, true });
	}

	ImGuiContext* ImGuiOverlay::GetContext() {
		return g_context;
	}

	void ImGuiOverlay::GetAllocatorFns(ImGuiMemAllocFunc* alloc, ImGuiMemFreeFunc* freeFn, void** userData) {
		ImGui::GetAllocatorFunctions(alloc, freeFn, userData);
	}

}
