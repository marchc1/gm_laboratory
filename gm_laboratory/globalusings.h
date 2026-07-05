#pragma once
#include <functional>
#include <string_view>

namespace gm_laboratory {
	struct InterfaceRegistryEditor;
	struct ClientClassEditor;
	struct ServerClassEditor;
}

using namespace gm_laboratory;

using strv = std::string_view;
using RewriteInterfacesFn = std::function<void(InterfaceRegistryEditor& editor)>;
using RewriteClientClassesFn = std::function<void(ClientClassEditor& editor)>;
using RewriteServerClassesFn = std::function<void(ServerClassEditor& editor)>;