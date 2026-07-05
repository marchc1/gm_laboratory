#include "native/vtable_editor.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <windows.h>

#include "Detours.h"

#include "native/detour_manager.h"
#include "native/log.h"

namespace gm_laboratory {
	int VTableIndexOfThunk(const void* thunk) {
		const auto* p = static_cast<const unsigned char*>(thunk);
		if (!p)
			return -1;

		for (int hops = 0; hops < 4 && p[0] == 0xE9; ++hops)
			p = p + 5 + *reinterpret_cast<const std::int32_t*>(p + 1);

		int i = 0;
		if (p[i] == 0x48 && p[i + 1] == 0x8B && p[i + 2] == 0x01)
			i += 3;

		if (p[i] == 0xFF) {
			const unsigned char modrm = p[i + 1];
			if (modrm == 0x20)
				return 0;
			if (modrm == 0x60)
				return static_cast<unsigned char>(p[i + 2]) / static_cast<int>(sizeof(void*));
			if (modrm == 0xA0)
				return *reinterpret_cast<const std::int32_t*>(p + i + 2) / static_cast<int>(sizeof(void*));
		}
		return -1;
	}

	static std::unordered_map<std::string, std::unique_ptr<Detours::RTTI::Object>>& ObjectCache() {
		static std::unordered_map<std::string, std::unique_ptr<Detours::RTTI::Object>> cache;
		return cache;
	}

	static std::vector<std::unique_ptr<Detours::Hook::VTableFunctionHook>>& InstalledHooks() {
		static std::vector<std::unique_ptr<Detours::Hook::VTableFunctionHook>> hooks;
		return hooks;
	}

	static Detours::RTTI::Object* FindObjectCached(HMODULE module, const char* className, unsigned int offset) {
		std::string key = std::to_string(reinterpret_cast<std::uintptr_t>(module)) + "|" + className + "|" + std::to_string(offset);
		auto& cache = ObjectCache();
		auto it = cache.find(key);
		if (it != cache.end())
			return it->second.get();

		auto object = Detours::RTTI::FindObject(module, className, nullptr, true, offset);
		Detours::RTTI::Object* raw = object.get();
		cache.emplace(std::move(key), std::move(object));
		return raw;
	}

	static int OffsetOfBase(HMODULE module, Detours::RTTI::Object* concrete, const char* interfaceRawName) {
		using namespace Detours::RTTI;
		const auto base = reinterpret_cast<std::uintptr_t>(module);

		PRTTI_CLASS_HIERARCHY_DESCRIPTOR chd = concrete->GetClassHierarchyDescriptor();
		if (!chd)
			return -1;

		auto* bca = reinterpret_cast<PRTTI_BASE_CLASS_ARRAY>(base + chd->m_unBaseClassArray);
		for (unsigned int i = 0; i < chd->m_unNumberOfBaseClasses; ++i) {
			auto* bcd = reinterpret_cast<PRTTI_BASE_CLASS_DESCRIPTOR>(base + bca->m_unBaseClassDescriptors[i]);
			auto* td = reinterpret_cast<PRTTI_TYPE_DESCRIPTOR>(base + bcd->m_unTypeDescriptor);
			if (std::strcmp(td->m_szName, interfaceRawName) == 0)
				return bcd->m_Where.m_nMDisp;
		}
		return -1;
	}

	static void* InstallVTableHook(void** vtable, int index, void* detour) {
		auto hook = std::make_unique<Detours::Hook::VTableFunctionHook>();
		if (!hook->Set(vtable, static_cast<std::size_t>(index)) || !hook->Hook(detour)) {
			Log("vtable", "failed to hook vtable %p slot %d\n", static_cast<void*>(vtable), index);
			return nullptr;
		}
		void* original = hook->GetOriginal();
		InstalledHooks().push_back(std::move(hook));
		return original;
	}

	void* VTableEditor::HookIndex(int index, void* detour) const {
		if (!Valid || !VTable || index < 0 || !detour) {
			Log("vtable", "HookIndex: invalid editor/args (index %d)\n", index);
			return nullptr;
		}
		return InstallVTableHook(VTable, index, detour);
	}

	void* VTableEditor::HookInterfaceMethod(const char* interfaceRawName, int index, void* detour) const {
		if (!Valid || index < 0 || !detour) {
			Log("vtable", "Hook: could not decode a vtable slot for %s (index %d)\n",
				interfaceRawName ? interfaceRawName : "?", index);
			return nullptr;
		}

		auto module = static_cast<HMODULE>(Module);
		Detours::RTTI::Object* concrete = FindObjectCached(module, ClassName, 0);
		if (!concrete)
			return nullptr;

		int mdisp = 0;
		if (interfaceRawName && std::strcmp(concrete->GetTypeDescriptor()->m_szName, interfaceRawName) != 0) {
			mdisp = OffsetOfBase(module, concrete, interfaceRawName);
			if (mdisp < 0) {
				Log("vtable", "%s is not a base of %s; assuming primary vtable\n", interfaceRawName, ClassName);
				mdisp = 0;
			}
		}

		void** vtable = VTable;
		if (mdisp != 0) {
			Detours::RTTI::Object* sub = FindObjectCached(module, ClassName, static_cast<unsigned int>(mdisp));
			if (!sub || !sub->GetVTable()) {
				Log("vtable", "could not resolve subobject vtable at +%d of %s\n", mdisp, ClassName);
				return nullptr;
			}
			vtable = sub->GetVTable();
		}

		return InstallVTableHook(vtable, index, detour);
	}

	VTableEditor GetVTable(const char* module, const char* className) {
		VTableEditor editor;
		editor.ClassName = className;

		auto handle = static_cast<HMODULE>(GetOrLoadModule(module));
		if (!handle) {
			Log("vtable", "GetVTable: module %s not loaded\n", module);
			return editor;
		}
		editor.Module = handle;

		Detours::RTTI::Object* object = FindObjectCached(handle, className, 0);
		if (!object || !object->GetVTable()) {
			Log("vtable", "GetVTable: no RTTI/vtable for %s in %s\n", className, module);
			return editor;
		}

		editor.VTable = object->GetVTable();
		editor.Valid = true;
		return editor;
	}
}
