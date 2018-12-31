#pragma once

namespace Game
{




	class CameraPath
	{
	public:
		CameraPath(const std::shared_ptr<DX::DeviceResources>& deviceResources);


		bool LoadBinary(int level);
		bool SaveBinary(int level);


		void AddPath(TerrainPath* _ter_path);

		void AddPoint(int _path_index, PathPoint* _point);


		DirectX::XMVECTOR GetCatRomPoint(int _path_index, int _point_index, float s);

		std::vector<PathPoint> GetPoints(int _path_index);

		int GetTotalPaths() { return terrain_path.size(); }

		int GetTotalPoints(int _path_index)
		{
			if (_path_index > -1 && _path_index < terrain_path.size())
				return terrain_path.at(_path_index).path_points.size();
		}



	private:
		std::vector<TerrainPath> terrain_path;

		std::shared_ptr<DX::DeviceResources> m_deviceResources;

	};

}