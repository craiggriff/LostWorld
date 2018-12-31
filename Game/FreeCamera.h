#pragma once

#include "Camera.h"
#include "Input.h"

namespace Game
{

	class FreeCamera
	{
	public:
		FreeCamera(Camera* p_Camera, PadInput^ p_PadInput);

		Camera* m_Camera;

		PadInput^ m_PadInput;



		btVector3 cam_pos, cam_rot;
		btMatrix3x3 cam_rot_mat;
		btVector3 cam_point_dir_x;
		btVector3 cam_point_dir_y;
		btVector3 cam_point_dir_z;

		float cam_at_y;

		void Update(DX::StepTimer const& timer);


	};

}