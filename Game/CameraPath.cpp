#include "pch.h"
#include "Content\ShaderStructures.h"
#include "CameraPath.h"


using namespace Game;
using namespace DirectX;
using namespace DX;
using namespace std;



CameraPath::CameraPath(const std::shared_ptr<DX::DeviceResources>& deviceResources)
{

	m_deviceResources = deviceResources;

	TerrainPath path;
	path.path_type = 0;

	AddPath(&path);
}


bool CameraPath::LoadBinary(int level)
{
	return true;
}

bool CameraPath::SaveBinary(int level)
{
	return true;
}




void CameraPath::AddPath(TerrainPath* _ter_path)
{
	terrain_path.push_back(*_ter_path);

}

void CameraPath::AddPoint(int _path_index, PathPoint* _point)
{
	terrain_path.at(_path_index).path_points.push_back(*_point);
}

std::vector<PathPoint> CameraPath::GetPoints(int _path_index)
{
	return terrain_path.at(_path_index).path_points;
}

DirectX::XMVECTOR CameraPath::GetCatRomPoint(int _path_index, int _point_index, float s)
{
	int space_at1 = _point_index - 1;
	int space_at2 = _point_index;
	int space_at3 = _point_index + 1;
	int space_at4 = _point_index + 2;

	if (space_at1 < 0)
		space_at1 = terrain_path.at(_path_index).path_points.size() - 1;

	if (space_at3 > terrain_path.at(_path_index).path_points.size() - 1)
		space_at3 -= (terrain_path.at(_path_index).path_points.size());

	if (space_at4 > terrain_path.at(_path_index).path_points.size() - 1)
		space_at4 -= (terrain_path.at(_path_index).path_points.size());

	float place_in_quad_vector = s;// -(float)space_at;

	XMVECTOR res = XMVectorCatmullRom(XMLoadFloat3(&terrain_path.at(_path_index).path_points.at(space_at1).pos),
		XMLoadFloat3(&terrain_path.at(_path_index).path_points.at(space_at2).pos),
		XMLoadFloat3(&terrain_path.at(_path_index).path_points.at(space_at3).pos),
		XMLoadFloat3(&terrain_path.at(_path_index).path_points.at(space_at4).pos),
		place_in_quad_vector);

	return res;

}