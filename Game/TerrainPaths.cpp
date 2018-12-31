#include "pch.h"
#include "Content\ShaderStructures.h"
#include "TerrainPaths.h"


using namespace Game;
using namespace DirectX;
using namespace DX;
using namespace std;



TerrainPaths::TerrainPaths(AllResources* p_Resources)
{
	m_Resources = p_Resources;
	m_deviceResources = m_Resources->m_deviceResources;

	TerrainPath path;
	path.path_type = 0;

	AddPath(&path);
}

/*
bool TerrainPaths::LoadBinary(int level)
{
	int i;

	char info_filename[140];

	if (m_Resources->bContentFolder == false)
	{
		sprintf_s(info_filename, "%s\\LevelBinary\\pathblend1%d.bmp", m_Resources->local_file_folder, level);
	}
	else
	{
		sprintf_s(info_filename, "Assets\\LevelBinary\\pathblend1%d.bmp", level);
	}

	FILE * pFile;

	fopen_s(&pFile, info_filename, "rb");
	if (pFile != NULL)
	{
		for (i = 0; i < total_y_points; i++)
		{
			fread(tex_blend[i], sizeof(float), total_x_points, pFile);
		}
	

		fclose(pFile);
	
		return true;
	}
	else
	{
		ClearArrays();
		return false;
	}
}

bool TerrainPaths::SaveBinary(int level)
{
	int i;
	char info_filename[140];

	sprintf_s(info_filename, "%s\\LevelBinary\\pathblend1%d.bmp", m_Resources->local_file_folder, level);

	FILE * pFile;

	fopen_s(&pFile, info_filename, "wb");
	if (pFile != NULL)
	{
		for (i = 0; i < total_y_points; i++)
		{
			fwrite(tex_blend[i], sizeof(float), total_x_points, pFile);
		}

		fclose(pFile);
	}
	else
	{
		return false;
	}

	return true;


}
*/

void TerrainPaths::SetBlendArray(float** _tex_blend)
{
	tex_blend = _tex_blend;

}



void TerrainPaths::ClearPath(int _path_index)
{
	terrain_path.at(_path_index).path_points.clear();
}

void TerrainPaths::ClearArrays()
{
	for (int i = 0; i <total_y_points; i++)
	{
		//tex_blend[i] = new float[total_x_points];
		for (int j = 0; j < total_x_points; j++)
		{
			tex_blend[i][j] = 0.0f;
		}
	}
}

void TerrainPaths::AddPath(TerrainPath* _ter_path)
{
	terrain_path.push_back(*_ter_path);

}

void TerrainPaths::AddPoint(int _path_index, PathPoint* _point)
{
	terrain_path.at(_path_index).path_points.push_back(*_point);
}

std::vector<PathPoint> TerrainPaths::GetPoints(int _path_index)
{
	return terrain_path.at(_path_index).path_points;
}

DirectX::XMVECTOR TerrainPaths::GetCatRomPoint(int _path_index, int _point_index, float s)
{
	int space_at1 = _point_index -1;
	int space_at2 = _point_index;
	int space_at3 = _point_index+1;
	int space_at4 = _point_index+2;

	if(space_at1 < 0)
		space_at1 = terrain_path.at(_path_index).path_points.size()-1;

	if (space_at3 > terrain_path.at(_path_index).path_points.size()-1 )
		space_at3 -= (terrain_path.at(_path_index).path_points.size());

	if (space_at4 > terrain_path.at(_path_index).path_points.size()-1 )
		space_at4 -= (terrain_path.at(_path_index).path_points.size() );

	float place_in_quad_vector = s;// -(float)space_at;



	XMVECTOR res = XMVectorCatmullRom(XMLoadFloat3(&terrain_path.at(_path_index).path_points.at(space_at1).pos),
		XMLoadFloat3(&terrain_path.at(_path_index).path_points.at(space_at2).pos),
		XMLoadFloat3(&terrain_path.at(_path_index).path_points.at(space_at3).pos),
		XMLoadFloat3(&terrain_path.at(_path_index).path_points.at(space_at4).pos),
										place_in_quad_vector);

	return res;

}