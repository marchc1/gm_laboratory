#pragma once
#include <functional>
#include <string_view>

namespace gm_laboratory {
	struct InterfaceRegistryEditor;
	struct ClientClassesEditor;
	struct ServerClassesEditor;
}

using namespace gm_laboratory;

using strv = std::string_view;
using RewriteInterfacesFn = std::function<void(InterfaceRegistryEditor& editor)>;
using RewriteClientClassesFn = std::function<void(ClientClassesEditor& editor)>;
using RewriteServerClassesFn = std::function<void(ServerClassesEditor& editor)>;