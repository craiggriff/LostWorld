#pragma once

#include "AnimChar.h"


namespace Game
{

	class Bert : public AnimChar
	{
	public:
		Bert(AllResources* p_Resources, Level* pp_Level);

		void Initialize(int char_num);
		

	};
};

