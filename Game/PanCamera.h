#pragma once
#include "Camera.h"
#include "Level.h"

namespace Game
{

	class PanCamera
	{
	public:
		PanCamera(Camera* pp_Camera,Level* pp_Level);
		~PanCamera();

		Camera* p_Camera;
		Level* p_Level;

		void SetEyeAt(float ex, float ey, float ez, float ax, float ay, float az, bool bDefin = false);
		void SetEyeAtUp(float ex, float ey, float ez, float ax, float ay, float az, float ux, float uy, float uz, bool bDefin);
		void Update(float timeDelta);

		btVector3 vEye, vAt, vEyeTo, vAtTo;
		btVector3 vec_looking_tan;
		btVector3 vec_looking;
		btVector3 vec_up;
	};
};

