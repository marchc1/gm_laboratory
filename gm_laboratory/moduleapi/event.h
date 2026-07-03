#pragma once

#include <functional>
#include <vector>

#include "native/interface_registry.h"

namespace gm_laboratory {
	template <class... A>
	class Event {
	public:
		void operator+=(std::function<void(A...)> handler) { handlers.push_back(std::move(handler)); }
		void Invoke(A... args) {
			for (auto& handler : handlers)
				handler(args...);
		}

	private:
		std::vector<std::function<void(A...)>> handlers;
	};
}