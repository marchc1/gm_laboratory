#pragma once

#include <cstring>
#include <typeinfo>

namespace gm_laboratory {
	int VTableIndexOfThunk(const void* thunk);

	template <class F>
	inline int VTableIndexOf(F mfp) {
		void* code = nullptr;
		std::memcpy(&code, &mfp, sizeof(code));
		return VTableIndexOfThunk(code);
	}

	struct VTableEditor {
		void* Module = nullptr;          
		const char* ClassName = nullptr; 
		void** VTable = nullptr;         
		bool Valid = false;

		void* HookIndex(int index, void* detour) const;

		void* HookInterfaceMethod(const char* interfaceRawName, int index, void* detour) const;

		template <class C, class R, class... A>
		void* Hook(R (C::* mfp)(A...), void* detour) const {
			return HookInterfaceMethod(typeid(C).raw_name(), VTableIndexOf(mfp), detour);
		}
		template <class C, class R, class... A>
		void* Hook(R (C::* mfp)(A...) const, void* detour) const {
			return HookInterfaceMethod(typeid(C).raw_name(), VTableIndexOf(mfp), detour);
		}
	};

	VTableEditor GetVTable(const char* module, const char* className);
}
