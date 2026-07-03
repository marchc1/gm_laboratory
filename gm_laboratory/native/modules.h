#pragma once

namespace gm_laboratory {

class Modules {
public:
	static bool Load(const char* name);
	static int LoadFolder(const char* folder);
};

}
