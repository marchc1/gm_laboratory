#include "launcher_main_windows.h"
#include <cstdio>
#include <cstdlib>
#include <system_error>
#include "windows.h"
#include "scoped_dll.h"
#include "native/detour_manager.h"
#include "native/modules.h"
#include "moduleapi/hooks.h"

using namespace source;

extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
}

template <size_t buffer_size>
const char* GetBaseDirectory(const char(&buffer)[buffer_size], char(&out_base_directory)[buffer_size]) {
	constexpr char target[]{ "GarrysMod" };

	strcpy_s(out_base_directory, buffer);

	while (true) {
		size_t size{ strlen(out_base_directory) };
		while (size > 0 && (out_base_directory[size - 1] == '\\' || out_base_directory[size - 1] == '/'))
			out_base_directory[--size] = '\0';

		if (size == 0)
			return nullptr;

		char* separator{ strrchr(out_base_directory, '\\') };
		char* fwd{ strrchr(out_base_directory, '/') };
		if (fwd > separator) separator = fwd;

		const char* component{ separator ? separator + 1 : out_base_directory };

		if (_stricmp(component, target) == 0)
			return out_base_directory;

		if (!separator)
			return nullptr;

		*separator = '\0';
	}
}

int ShowErrorBoxAndExitWithCode(_In_z_ const char* error_message,
	std::error_code exit_code) {
	const auto system_error = exit_code.message();

	char entire_error_message[2048];
	_snprintf_s(entire_error_message, _TRUNCATE, "%s\n\n%s", error_message, system_error.c_str());
	MessageBoxA(nullptr, entire_error_message, "Garry's Mod Experiment Toolkit [Launcher] - Error", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
	return exit_code.value();
}

std::error_code GetLastErrorCode(DWORD errc = GetLastError()) {
	return std::error_code{ static_cast<int>(errc), std::system_category() };
}


int ApplyProcessMitigation_BlockPreloadingDLLs() {
	if (!::SetDllDirectoryA(""))
		return ShowErrorBoxAndExitWithCode("ApplyProcessMitigation_BlockPreloadingDLLs: FAILED", GetLastErrorCode());
	return 0;
}

int ApplyProcessMitigation_TerminateOnHeapCorruption() {
	if (!HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0))
		return ShowErrorBoxAndExitWithCode("ApplyProcessMitigation_TerminateOnHeapCorruption: FAILED", GetLastErrorCode());
	return 0;
}

int ApplyProcessMitigation_EnableASLR() {
	PROCESS_MITIGATION_ASLR_POLICY policy = {};
	policy.EnableForceRelocateImages = policy.DisallowStrippedImages = true;
	if (!SetProcessMitigationPolicy(ProcessASLRPolicy, &policy, sizeof(policy)) && ERROR_ACCESS_DENIED != ::GetLastError())
		return ShowErrorBoxAndExitWithCode("ApplyProcessMitigation_EnableASLR: FAILED", GetLastErrorCode());
	return 0;
}

int ApplyProcessMitigation_EnableStrictHandlePolicy() {
	PROCESS_MITIGATION_STRICT_HANDLE_CHECK_POLICY policy = {};
	policy.HandleExceptionsPermanentlyEnabled = policy.RaiseExceptionOnInvalidHandleReference = true;
	if (!SetProcessMitigationPolicy(ProcessStrictHandleCheckPolicy, &policy, sizeof(policy)) && ERROR_ACCESS_DENIED != ::GetLastError())
		return ShowErrorBoxAndExitWithCode("ApplyProcessMitigation_EnableStrictHandlePolicy: FAILED", GetLastErrorCode());
	return 0;
}

int ApplyProcessMitigation_EnableExtensionPointsPolicy() {
	PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY policy = {};
	policy.DisableExtensionPoints = true;
	if (!SetProcessMitigationPolicy(ProcessExtensionPointDisablePolicy, &policy, sizeof(policy)) && ERROR_ACCESS_DENIED != ::GetLastError())
		return ShowErrorBoxAndExitWithCode("ApplyProcessMitigation_EnableExtensionPointsPolicy: FAILED", GetLastErrorCode());
	return 0;
}

int ApplyProcessMitigation_HardenImageLoadPolicy() {
	PROCESS_MITIGATION_IMAGE_LOAD_POLICY policy = {};
	policy.NoRemoteImages = true;
	policy.NoLowMandatoryLabelImages = true;
	policy.PreferSystem32Images = true;

	if (!SetProcessMitigationPolicy(ProcessImageLoadPolicy, &policy,
		sizeof(policy)) &&
		ERROR_ACCESS_DENIED != ::GetLastError()) {
		return ShowErrorBoxAndExitWithCode("ApplyProcessMitigation_HardenImageLoadPolicy: FAILED", GetLastErrorCode());
	}

	return 0;
}

int ApplyProcessMitigations() {
	int err;
	if (err = ApplyProcessMitigation_BlockPreloadingDLLs())
		return err;

	if (err = ApplyProcessMitigation_TerminateOnHeapCorruption())
		return err;

	if (err = ApplyProcessMitigation_EnableASLR())
		return err;

	if (err = ApplyProcessMitigation_EnableStrictHandlePolicy())
		return err;

	if (err = ApplyProcessMitigation_EnableExtensionPointsPolicy())
		return err;

	if (err = ApplyProcessMitigation_HardenImageLoadPolicy())
		return err;

	return 0;
}

#define VALVE_OB_STRINGIFY(x) #x
#define VALVE_OB_TOSTRING(x) VALVE_OB_STRINGIFY(x)

int Run(_In_ HINSTANCE instance, _In_opt_ HINSTANCE old_instance, _In_ LPSTR cmd_line, _In_ int window_flags) {
	const int rc{ ApplyProcessMitigations() };
	if (rc)
		return static_cast<int>(rc);

	char module_name[MAX_PATH];
	if (!::GetModuleFileNameA(instance, module_name, MAX_PATH))
		return ShowErrorBoxAndExitWithCode("Please check game installed in the folder with less than " VALVE_OB_TOSTRING(MAX_PATH) " chars deep.\n\nUnable to get module file name from GetModuleFileName.", GetLastErrorCode());

	char base_directory_path[MAX_PATH], launcher_dll_path[MAX_PATH];

	const char* baseDir = GetBaseDirectory(module_name, base_directory_path);
	_snprintf_s(launcher_dll_path, _TRUNCATE, "%s\\bin\\win64\\launcher.dll", baseDir);

	char user_error[1024];
	const source::ScopedDll launcher_dll{ launcher_dll_path, LOAD_WITH_ALTERED_SEARCH_PATH };
	if (!launcher_dll) {
		_snprintf_s(user_error, _TRUNCATE,
			"Please ensure gm_laboratory_launcher.exe was installed in a folder with less "
			"than " VALVE_OB_TOSTRING(MAX_PATH) " chars deep. It should live in GarrysMod/bin/win64. If you are not on the x86-64 branch, please switch to it.\n\n"
			"Attempted to load DLL from path: %s.",
			launcher_dll_path);

		return ShowErrorBoxAndExitWithCode(user_error, launcher_dll.error_code());
	}

	SetCurrentDirectoryA(baseDir);

	using LauncherMainFunction = int (*)(HINSTANCE, HINSTANCE, LPSTR, int);
	constexpr char launcher_main_function_name[]{ "LauncherMain" };

	const auto [launcher_main, errc] = launcher_dll.GetFunction<LauncherMainFunction>(launcher_main_function_name);
	if (!launcher_main) {
		_snprintf_s(user_error, _TRUNCATE, "This shouldn't happen: %s procedure not found in %s's exports", launcher_main_function_name, launcher_dll_path);
		return ShowErrorBoxAndExitWithCode(user_error, errc);
	}

	if (GetConsoleWindow() == NULL) 
		if (!AttachConsole(ATTACH_PARENT_PROCESS))
			if (!AllocConsole()) 
				return ShowErrorBoxAndExitWithCode("AllocConsole failure.", GetLastErrorCode());

	// wtf
	FILE* dummyFile;
	freopen_s(&dummyFile, "CONIN$", "r", stdin);
	freopen_s(&dummyFile, "CONOUT$", "w", stdout);
	freopen_s(&dummyFile, "CONOUT$", "w", stderr);

	// install spew callback

	char modules_dir[MAX_PATH];
	_snprintf_s(modules_dir, _TRUNCATE, "%s\\gm_laboratory\\modules", getenv("USERPROFILE"));
	gm_laboratory::Modules::LoadFolder(modules_dir);
	gm_laboratory::DetourManager::Bootstrap();
	gm_laboratory::Hooks::PreLauncherMain.Invoke();

	return launcher_main(instance, old_instance, cmd_line, window_flags);
}

int APIENTRY WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE old_instance, _In_ LPSTR cmd_line, _In_ int window_flags) {
	const auto rc = Run(instance, old_instance, cmd_line, window_flags);
	exit(rc);
}