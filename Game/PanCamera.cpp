#include "pch.h"
#include "PanCamera.h"

using namespace Game;

PanCamera::PanCamera(Camera* pp_Camera, Level* pp_Level)
{
	p_Camera = pp_Camera;
	p_Level = pp_Level;

	SetEyeAt(0.0f, 5.0f, 0.0f, 0.0f, 0.1f, 10.0f, true);
}


PanCamera::~PanCamera()
{


}

void PanCamera::Update(float timeDelta)
{
	vEye += ((vEyeTo - vEye)*(timeDelta*4.5))-((vAtTo - vAt)*(timeDelta*6.5));
	vAt += (vAtTo - vAt)*(timeDelta*6.5);

	//vEye.setY(vEye.getY() + 1.0f);

	vec_looking = vAt - vEye;
	vec_looking.setY(0.0f);
	vec_looking.normalize();
	vec_looking_tan = vec_looking.rotate(btVector3(0.0f, 1.0f, 0.0f), -3.14159*0.5f);

	float t_height=p_Level->GetTerrainHeight(vEye.getX(), vEye.getZ());

	if (vEye.getY() < t_height + 2.0f)
	{
		vEye.setY(t_height + 2.0f);
	}

	p_Camera->SetViewParams(DirectX::XMFLOAT3(vEye.getX(), vEye.getY(), vEye.getZ()), DirectX::XMFLOAT3(vAt.getX(), vAt.getY(), vAt.getZ()), DirectX::XMFLOAT3(vec_up.getX(), vec_up.getY(), vec_up.getZ()));
	p_Camera->SetViewAndTan(vec_looking.getX(), vec_looking.getY(), vec_looking.getZ(), vec_looking_tan.getX(), vec_looking_tan.getY(), vec_looking_tan.getZ());
}

void PanCamera::SetEyeAtUp(float ex, float ey, float ez, float ax, float ay, float az, float ux,float uy,float uz, bool bDefin)
{
	vEyeTo.setX(ex);
	vEyeTo.setY(ey);
	vEyeTo.setZ(ez);

	vAtTo.setX(ax);
	vAtTo.setY(ay);
	vAtTo.setZ(az);

	if (bDefin == true)
	{
		vEye.setX(ex);
		vEye.setY(ey);
		vEye.setZ(ez);

		vAt.setX(ax);
		vAt.setY(ay);
		vAt.setZ(az);
	}

	vec_up = btVector3(ux, uy, uz);
}

void PanCamera::SetEyeAt(float ex, float ey, float ez, float ax, float ay, float az, bool bDefin)
{
	vEyeTo.setX(ex);
	vEyeTo.setY(ey);
	vEyeTo.setZ(ez);

	vAtTo.setX(ax);
	vAtTo.setY(ay);
	vAtTo.setZ(az);

	if (bDefin == true)
	{
		vEye.setX(ex);
		vEye.setY(ey);
		vEye.setZ(ez);

		vAt.setX(ax);
		vAt.setY(ay);
		vAt.setZ(az);
	}

	vec_up = btVector3(0.00001f, 1.0f, 0.0f);
}
