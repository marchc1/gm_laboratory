#include "native/sigscan.h"

#include <cstdlib>
#include <string>

#include "Detours.h"

namespace gm_laboratory {

static bool ParsePattern(const char* pattern, std::string& out) {
	const char* p = pattern;
	while (*p) {
		while (*p == ' ')
			++p;
		if (!*p)
			break;

		if (p[0] == '?') {
			out.push_back(static_cast<char>(0x2A));
			++p;
			if (*p == '?')
				++p;
		} else {
			if (!p[1])
				return false;
			char hex[3] = { p[0], p[1], 0 };
			out.push_back(static_cast<char>(static_cast<unsigned char>(std::strtoul(hex, nullptr, 16))));
			p += 2;
		}
	}
	return !out.empty();
}

void* FindSignature(void* module, const char* pattern, std::size_t offset) {
	std::string bytes;
	if (!ParsePattern(pattern, bytes))
		return nullptr;

	void const* found = Detours::Scan::FindSignatureNative(static_cast<HMODULE>(module), bytes.c_str(), '\x2A');
	if (!found)
		return nullptr;

	return const_cast<void*>(static_cast<void const*>(static_cast<char const*>(found) + offset));
}

}
