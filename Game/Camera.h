#pragma once

#include "CameraPath.h"

namespace Game
{

	class Camera
	{
	public:
		Camera(const std::shared_ptr<DX::DeviceResources>& deviceResources);

		void CreateWindowSizeDependentResources();

		void UpdateConstantBuffer(DirectX::XMFLOAT4X4 &proj_matrix, DirectX::XMFLOAT4X4 &view_matrix);

		void UpdateConstantBuffer(bool bUpdateCameraPos=false);
		void UpdateConstantBufferOrth();
		void SetFlatModelMatrix();

		float Within3DManhattanDistance(float x, float y, float z, float x1, float y1, float z1, float distance);

		CameraPath* m_CameraPath; // fly path for level start


		void SetViewParams(
			_In_ DirectX::XMFLOAT3 eye,
			_In_ DirectX::XMFLOAT3 lookAt,
			_In_ DirectX::XMFLOAT3 up);

		void SetViewAndTan(float vx, float vy, float vz, float tx, float ty, float tz);

		DirectX::XMFLOAT4X4* GetViewMatrix();

		DirectX::XMFLOAT3 view_dir;
		DirectX::XMFLOAT3 view_tan;

		float Distance(float x, float y, float z, float x1, float y1, float z1);
		float DistanceEst(float x, float y, float z, float x1, float y1, float z1);

		float plane_dist[6];

		void SetModelMatrix(DirectX::XMFLOAT4X4* p_ModelMatrix);
		void SetViewMatrix(DirectX::XMFLOAT4X4* p_ViewMatrix);
		void SetProjectionMatrix(DirectX::XMFLOAT4X4* p_ProjectionMatrix);

		DirectX::XMFLOAT3 GetEye();

		void Update(DX::StepTimer const& timer);

		void buildWorldFrustumPlanes();
		bool CheckPlanes(float x, float y, float z, float radius);

		float CheckPoint(FLOAT x, FLOAT y, FLOAT z, FLOAT radius, float dist, float* full_dist=nullptr);
		float CheckPoint(FLOAT x, FLOAT y, FLOAT z, FLOAT radius, float* full_dist = nullptr);
		float DistanceFromEye(float x, float y, float z);


		void Camera::CreateCubeProjection(
			_In_ float minimumFieldOfView,  // the minimum horizontal or vertical field of view, in degrees
			_In_ float aspectRatio,         // the aspect ratio of the projection (width / height)
			_In_ float nearPlane,           // depth to map to 0
			_In_ float farPlane             // depth to map to 1
		);

		DirectX::XMFLOAT4X4 Camera::MakeViewParams(
			_In_ DirectX::XMFLOAT3 eye,
			_In_ DirectX::XMFLOAT3 lookAt,
			_In_ DirectX::XMFLOAT3 up);

		void GetWorldLine(UINT pixelX, UINT pixelY, DirectX::XMFLOAT3* outPoint, DirectX::XMFLOAT3* outDir);

		void SetSkyCubeProjection();
		void SetSkyProjection();
		void SetCloseProjection();
		void SetCubeProjection();

		float LookingTanX();
		float LookingTanZ();
		float LookingX();
		float LookingY();
		float LookingZ();
		float PositionX();
		float PositionZ();

		DirectX::XMFLOAT3 m_eye;
		DirectX::XMFLOAT3 m_lookAt;
		DirectX::XMFLOAT3 m_up;

		DirectX::XMFLOAT4	mFrustumPlanes[6]; 	// [0] = near, [1] = far, [2] = left, [3] = right, [4] = top, [5] = bottom

		DirectX::XMFLOAT4X4 m_inverseView; // inverse for culling

		DirectX::XMFLOAT4X4 m_reflectionView; // water reflection

		DirectX::XMFLOAT4X4 close_projection;
		DirectX::XMFLOAT4X4 sky_projection;
		DirectX::XMFLOAT4X4 skycube_projection;
		DirectX::XMFLOAT4X4 cube_projection;

		Windows::Foundation::Size outputSize;

		btVector3 vec_looking_tan;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer	 m_constantBufferData;
		ModelViewProjectionConstantBuffer	 m_orthBufferData; // for def rendering


		CameraBufferType m_constantbuffercameraPos;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_cameraBuffer;

		DirectX::XMFLOAT4X4 flat_model_matrix; // flat model matrix for particle effects etc

		float far_dist;
		void SetFarDist(float _dist);
	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

	};

}