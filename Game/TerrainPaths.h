#pragma once

namespace Game
{




	class TerrainPaths
	{
	public:
		TerrainPaths(AllResources* p_Resources);


		//bool LoadBinary(int level);
		//bool SaveBinary(int level);


		void AddPath(TerrainPath* _ter_path);

		void AddPoint(int _path_index, PathPoint* _point);


		DirectX::XMVECTOR GetCatRomPoint(int _path_index, int _point_index, float s);

		std::vector<PathPoint> GetPoints(int _path_index);

		int GetTotalPaths() { return terrain_path.size(); }

		void SetBlendArray(float** _tex_blend);
		void ClearArrays();

		void ClearPath(int _path_index);
		

		int GetTotalPoints(int _path_index) 
		{
			if (_path_index > -1 && _path_index < terrain_path.size()) 
				return terrain_path.at(_path_index).path_points.size();
		}

		float** tex_blend;
		int total_y_points, total_x_points;

	private:
		std::vector<TerrainPath> terrain_path;

		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		AllResources* m_Resources;
	};

}