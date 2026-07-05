#include "moduleapi/hooks.h"
#include "globalusings.h"

class ClientClass;
class ServerClass;

namespace gm_laboratory {
	namespace Hooks {

#define DEFINE_MODULE_HOOK(RetType, Name, ...) Event<__VA_ARGS__> Name;
#include "defs/hook_events.h"
#undef DEFINE_MODULE_HOOK

		static std::vector<RewriteInterfacesFn>& RewriteInterfacesFns() {
			static std::vector<RewriteInterfacesFn> fns;
			return fns;
		}

		void AddInterfaceRewriter(RewriteInterfacesFn fn) {
			WalkInterfaces(nullptr, nullptr, [fn](void*, InterfaceReg* reg) {
				InterfaceRegistryEditor editor;
				editor.ListStart = reg;
				fn(editor);
				});
			RewriteInterfacesFns().push_back(fn);
		}

		void ExecRewriteInterfaces(InterfaceReg* reg) {
			if (reg == nullptr)
				return;

			InterfaceRegistryEditor editor;
			editor.ListStart = reg;
			for (auto& fn : RewriteInterfacesFns())
				fn(editor);
		}

	}
}
