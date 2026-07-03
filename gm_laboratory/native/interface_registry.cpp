#include "native/interface_registry.h"

#include <cstring>

#include "Detours.h"

#include "tier1/interface.h"

#include "native/detour_manager.h"
#include "native/log.h"

namespace gm_laboratory {

InterfaceReg* GetInterfaceRegistryFromCreateInterfacePtr(void* createInterfacePtr) {
	unsigned char* code = static_cast<unsigned char*>(createInterfacePtr);

	for (int i = 0; i + 7 <= 64; i++) {
		unsigned char rex = code[i];
		if ((rex & 0xF8) != 0x48)
			continue;
		if (code[i + 1] != 0x8B)
			continue;
		if ((code[i + 2] & 0xC7) != 0x05)
			continue;

		char* instrAddr = static_cast<char*>(createInterfacePtr) + i;
		int ripOffset = *reinterpret_cast<int*>(code + i + 3);
		void* absAddress = instrAddr + 7 + ripOffset;
		InterfaceReg* firstReg = *reinterpret_cast<InterfaceReg**>(absAddress);
		return firstReg;
	}

	return nullptr;
}

InterfaceReg* GetInterfaceRegistry(void* module) {
	void* createInterfacePtr = reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(module), "CreateInterface"));
	if (!createInterfacePtr)
		return nullptr;
	return GetInterfaceRegistryFromCreateInterfacePtr(createInterfacePtr);
}

static bool __WalkInterfaces(void* createInterfacePtr, const WalkInterfaceFn& walker, const WalkRegistryStartFn& walkRegistry) {
	InterfaceReg* firstReg = GetInterfaceRegistryFromCreateInterfacePtr(createInterfacePtr);
	if (firstReg == nullptr)
		return false;

	if (walkRegistry)
		walkRegistry(createInterfacePtr, firstReg);

	while (firstReg != nullptr) {
		const char* managedName = firstReg->m_pName ? firstReg->m_pName : "";
		if (walker)
			walker(managedName, reinterpret_cast<void*>(firstReg->m_CreateFn));
		firstReg = firstReg->m_pNext;
	}
	return true;
}

void WalkInterfaces(WalkModuleFn moduleWalk, WalkInterfaceFn walker, WalkRegistryStartFn walkRegistry) {
	GetOrLoadModule("lua_shared.dll"); // I don't remember why this is here!

	Detours::PPEB pPEB = Detours::GetPEB();
	if (!pPEB || !pPEB->Ldr)
		return;

	Detours::PPEB_LDR_DATA pLdr = pPEB->Ldr;
	for (LIST_ENTRY* entry = pLdr->InLoadOrderModuleList.Flink; entry != &pLdr->InLoadOrderModuleList; entry = entry->Flink) {
		auto* pEntry = CONTAINING_RECORD(entry, Detours::LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

		HMODULE moduleBase = static_cast<HMODULE>(pEntry->DllBase);
		void* createInterfacePtr = reinterpret_cast<void*>(GetProcAddress(moduleBase, "CreateInterface"));
		if (!createInterfacePtr)
			continue;

		char moduleName[256] = {};
		if (pEntry->BaseDllName.Buffer) {
			WideCharToMultiByte(CP_UTF8, 0, pEntry->BaseDllName.Buffer, pEntry->BaseDllName.Length / sizeof(wchar_t), moduleName, sizeof(moduleName) - 1, nullptr, nullptr);
		}

		if (moduleWalk)
			moduleWalk(moduleName, moduleBase, createInterfacePtr);

		// add module name here
		if (!__WalkInterfaces(createInterfacePtr, walker, walkRegistry))
			Log("interfaces", "ERROR: %s failed to produce a s_pInterfaceRegs linked list\n", moduleName);
	}
}

InterfaceRegistration InterfaceRegistryEditor::GetInterface(const char* lookup) {
	InterfaceReg* last = nullptr;
	InterfaceReg* reg = ListStart;
	InterfaceReg* next = ListStart == nullptr ? nullptr : ListStart->m_pNext;
	while (reg != nullptr) {
		const char* name = reg->m_pName ? reg->m_pName : "";
		if (std::strcmp(name, lookup) == 0) {
			InterfaceRegistration result;
			result.Last = last;
			result.Curr = reg;
			result.Next = next;
			result.Valid = true;
			return result;
		}

		last = reg;
		reg = reg->m_pNext;
		if (reg != nullptr)
			next = reg->m_pNext;
	}
	return {};
}

void InterfaceRegistration::Reroute(void* (*fn)()) {
	Curr->m_CreateFn = fn;
}

}
