#pragma once

#include "tier1/interface.h"
#include "tier1/tier1.h"
#include "tier2/tier2.h"
#include "tier3/tier3.h"

#include "native/interface_registry.h"
#include "connect_game_libraries.h"

#define SOURCE_SDK_CONNECT() { \
	std::vector<CreateInterfaceFn> factories; \
	WalkInterfaces( \
		[&factories](const char*, void*, void* createInterfaceFn) { \
			factories.push_back(reinterpret_cast<CreateInterfaceFn>(createInterfaceFn)); \
		}, \
		nullptr, nullptr); \
ConnectTier1Libraries(factories.data(), static_cast<int>(factories.size())); \
ConnectTier2Libraries(factories.data(), static_cast<int>(factories.size())); \
ConnectTier3Libraries(factories.data(), static_cast<int>(factories.size())); \
ConnectGameLibraries(); \
ConVar_Register(0); \
}