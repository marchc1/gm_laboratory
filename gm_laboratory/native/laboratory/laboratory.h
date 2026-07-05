#pragma once

#include "native/laboratory/lab_app.h"

namespace gm_laboratory {

	struct Laboratory {
		static void Register(const LabAppDesc* desc);
		static void Init();
		static void Render();
	};

}
