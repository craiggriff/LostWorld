#include "pch.h"
#include "CatmullRomPath.h"


using namespace Game;
using namespace DirectX;
using namespace DX;
using namespace std;



CatmullRomPath::CatmullRomPath()
{




}

void CatmullRomPath::AddPoint(XMFLOAT3* pos)
{
	space_point.push_back(XMLoadFloat3(pos));
}



DirectX::XMVECTOR CatmullRomPath::GetPoint(float s)
{
	XMFLOAT3 noww = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMVECTOR nowhere = XMLoadFloat3(&noww);

	int space_at = (int)s;
	float place_in_quad_vector = (float)s - (float)space_at;

	if (s < 1.0f)
		return nowhere;

	if (s > space_point.size()-1)
		return nowhere;

	XMVECTOR res = XMVectorCatmullRom(space_point.at(space_at), space_point.at(space_at + 1), space_point.at(space_at + 2), space_point.at(space_at + 3), place_in_quad_vector);

	return res;

}