#pragma once

#include "carrig.h"

namespace Game
{

	class Buggy : public CarRig
	{
	public:
		Buggy(AllResources* p_Resources);
			

		void Initialize(Level* pp_Level, int type);

	};
};

