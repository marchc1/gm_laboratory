#pragma once

#include <functional>

class InterfaceReg;

namespace gm_laboratory {

	using WalkModuleFn = std::function<void(const char* name, void* addr, void* createInterfaceFn)>;
	using WalkInterfaceFn = std::function<void(const char* name, void* createFn)>;
	using WalkRegistryStartFn = std::function<void(void* createInterfaceFn, InterfaceReg* reg)>;

	InterfaceReg* GetInterfaceRegistryFromCreateInterfacePtr(void* createInterfacePtr);
	InterfaceReg* GetInterfaceRegistry(void* module);
	void WalkInterfaces(WalkModuleFn moduleWalk, WalkInterfaceFn walker, WalkRegistryStartFn walkRegistry = {});

	struct InterfaceRegistration {
		InterfaceReg* Last = nullptr;
		InterfaceReg* Curr = nullptr;
		InterfaceReg* Next = nullptr;
		bool Valid = false;

		void Reroute(void* (*fn)());
	};

	struct InterfaceRegistryEditor {
		InterfaceReg* ListStart = nullptr;

		InterfaceRegistration GetInterface(const char* lookup);
	};

}
