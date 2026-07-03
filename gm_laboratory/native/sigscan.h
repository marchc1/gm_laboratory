#pragma once

#include <cstddef>

namespace gm_laboratory {

void* FindSignature(void* module, const char* pattern, std::size_t offset = 0);

}
