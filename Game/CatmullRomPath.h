#pragma once

namespace Game
{


	typedef struct
	{
		DirectX::XMVECTOR pos;
	} space_point;

	class CatmullRomPath
	{
	public:
		CatmullRomPath();


		std::vector<DirectX::XMVECTOR> space_point;

		void AddPoint(DirectX::XMFLOAT3* pos);


		DirectX::XMVECTOR CatmullRomPath::GetPoint(float s);



	};

}