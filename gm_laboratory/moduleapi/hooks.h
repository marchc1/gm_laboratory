#pragma once

#include <functional>
#include <vector>

#include "native/interface_registry.h"
#include "moduleapi/event.h"
#include "globalusings.h"

class ClientClass;
class ServerClass;

namespace gm_laboratory {
	namespace Hooks {
#define DEFINE_MODULE_HOOK(RetType, Name, ...) extern Event<__VA_ARGS__> Name;
#include "defs/hook_events.h"
#undef DEFINE_MODULE_HOOK

		extern Event<InterfaceReg*> RewriteInterfaces;

		void AddInterfaceRewriter(RewriteInterfacesFn fn);
		void ExecRewriteInterfaces(InterfaceReg* reg);

		void AddClientClassRewriter(RewriteClientClassesFn fn);
		void AddServerClassRewriter(RewriteServerClassesFn fn);
	}
}
