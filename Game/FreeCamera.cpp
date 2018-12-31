#include "pch.h"
#include "FreeCamera.h"

using namespace Game;
using namespace DirectX;

FreeCamera::FreeCamera(Camera* p_Camera, PadInput^ p_PadInput):
	m_Camera(p_Camera),m_PadInput(p_PadInput)
{
	cam_pos = btVector3(0.0f , 40.0f , -10.0f);
	cam_rot = btVector3(0.0f ,  0.0f ,   0.0f);

	cam_at_y = -1.5f;

	m_PadInput = ref new PadInput(CoreWindow::GetForCurrentThread());
}


void FreeCamera::Update(DX::StepTimer const& timer)
{
	if (m_PadInput == nullptr)
		return;

	btVector3 sideways = cam_point_dir_x;
	sideways.setY(0.0f);
	sideways.normalize();
	btVector3 forwards = cam_point_dir_z;
	forwards.setY(0.0f);
	forwards.normalize();

	float movespeed;

	if (m_PadInput->KeyState(Windows::System::VirtualKey::PageDown) == true)
	{
		movespeed = 1.5f;
	}
	else
	{
		movespeed = 1.0f;
	}

	// Camera movement
	if (m_PadInput->KeyState(Windows::System::VirtualKey::Right) == true)
	{
		cam_pos.setX(cam_pos.getX() - (movespeed*sideways.getX()));
		cam_pos.setZ(cam_pos.getZ() - (movespeed*sideways.getZ()));
	}
	if (m_PadInput->KeyState(Windows::System::VirtualKey::Left) == true)
	{
		cam_pos.setX(cam_pos.getX() + (movespeed*sideways.getX()));
		cam_pos.setZ(cam_pos.getZ() + (movespeed*sideways.getZ()));
	}

	if (m_PadInput->KeyState(Windows::System::VirtualKey::NumberPad9) == true)
		cam_pos.setY(cam_pos.getY() + movespeed);
	if (m_PadInput->KeyState(Windows::System::VirtualKey::NumberPad3) == true)
		cam_pos.setY(cam_pos.getY() - movespeed);

	if (m_PadInput->KeyState(Windows::System::VirtualKey::NumberPad8) == true)
	{
		cam_pos.setX(cam_pos.getX() + (movespeed*forwards.getX()));
		cam_pos.setZ(cam_pos.getZ() + (movespeed*forwards.getZ()));
	}

	if (m_PadInput->KeyState(Windows::System::VirtualKey::NumberPad2) == true)
	{
		cam_pos.setX(cam_pos.getX() - (movespeed*forwards.getX()));
		cam_pos.setZ(cam_pos.getZ() - (movespeed*forwards.getZ()));
	}


	if (m_PadInput->KeyState(Windows::System::VirtualKey::NumberPad6) == true)
		cam_rot.setX(cam_rot.getX() + 0.04f);
	if (m_PadInput->KeyState(Windows::System::VirtualKey::NumberPad4) == true)
		cam_rot.setX(cam_rot.getX() - 0.04f);


	if (m_PadInput->KeyState(Windows::System::VirtualKey::Up) == true)
	{
		cam_at_y += 0.02f;
	}
	if (m_PadInput->KeyState(Windows::System::VirtualKey::Down) == true)
	{
		cam_at_y -= 0.02f;
	}
	/*
	if (m_controller->CamRoll() == 1)
	cam_rot.setZ(cam_rot.getZ() + 0.02f);
	if (m_controller->CamRoll() == -1)
	cam_rot.setZ(cam_rot.getZ() - 0.02f);
	*/
	if (cam_rot.getX() > M_PI*2.0f)
		cam_rot.setX(cam_rot.getX() - M_PI*2.0f);
	if (cam_rot.getY() > M_PI*2.0f)
		cam_rot.setY(cam_rot.getY() - M_PI*2.0f);
	if (cam_rot.getZ() > M_PI*2.0f)
		cam_rot.setZ(cam_rot.getZ() - M_PI*2.0f);

	if (cam_rot.getX() < 0.0f)
		cam_rot.setX(cam_rot.getX() + M_PI*2.0f);
	if (cam_rot.getY() < 0.0f)
		cam_rot.setY(cam_rot.getY() + M_PI*2.0f);
	if (cam_rot.getZ() < 0.0f)
		cam_rot.setZ(cam_rot.getZ() + M_PI*2.0f);


	btQuaternion quat = btQuaternion(cam_rot.getX(), cam_rot.getY(), cam_rot.getZ());

	cam_rot_mat = btMatrix3x3(quat);

	cam_point_dir_x = btVector3(1.0f, 0.0f, 0.0f);
	cam_point_dir_y = btVector3(0.0f, 1.0f, 0.0f);
	cam_point_dir_z = btVector3(0.0f, 0.0f, 1.0f);

	cam_point_dir_x = cam_point_dir_x * cam_rot_mat;
	cam_point_dir_y = cam_point_dir_y * cam_rot_mat;
	cam_point_dir_z = cam_point_dir_z * cam_rot_mat;




	m_Camera->SetViewParams(XMFLOAT3(cam_pos.getX(), cam_pos.getY(), cam_pos.getZ()), XMFLOAT3(cam_pos.getX() + cam_point_dir_z.getX(), cam_pos.getY() + cam_at_y, cam_pos.getZ() + cam_point_dir_z.getZ()), XMFLOAT3(0.0f, 1.0f, 0.0f));

}


