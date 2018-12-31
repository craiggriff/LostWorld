#include "pch.h"


using namespace Game;
using namespace DirectX;
using namespace Windows::Foundation;
using namespace concurrency;
using namespace Windows::System::Threading;

#include "Camera.h"

Camera::Camera(const std::shared_ptr<DX::DeviceResources>& deviceResources):
	m_deviceResources(deviceResources)
{
	auto rotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	auto translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	XMMATRIX scaleMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX scaleRotationMatrix = XMMatrixMultiply(scaleMatrix, rotationMatrix);
	XMMATRIX modelMatrix = XMMatrixMultiplyTranspose(scaleRotationMatrix, translationMatrix);

	XMStoreFloat4x4(&flat_model_matrix, XMMatrixTranspose(rotationMatrix * translationMatrix));


	m_CameraPath = new CameraPath(deviceResources);

	// Update the 3D projection matrix of the environment camera.
	CreateCubeProjection(
		90.0f,                                                  // Use a 90-degree vertical field of view
		static_cast<float>(512) /
		static_cast<float>(512),              // Specify the aspect ratio of the window.
		0.1f,                                                  // Specify the nearest Z-distance at which to draw vertices.
		2500.0f                                                  // Specify the farthest Z-distance at which to draw vertices.
	);

	skycube_projection = cube_projection;

	// Update the 3D projection matrix of the environment camera.
	CreateCubeProjection(
		90.0f,                                                  // Use a 90-degree vertical field of view
		static_cast<float>(512) /
		static_cast<float>(512),              // Specify the aspect ratio of the window.
		0.1f,                                                  // Specify the nearest Z-distance at which to draw vertices.
		LIGHT_RADIUS                                                  // Specify the farthest Z-distance at which to draw vertices.
	);

	far_dist = 60.0f;
}

void Camera::UpdateConstantBufferOrth()
{
	XMStoreFloat4x4(&m_orthBufferData.mvp, XMLoadFloat4x4(&m_orthBufferData.projection) * XMLoadFloat4x4(&m_orthBufferData.view) *XMLoadFloat4x4(&m_orthBufferData.model));

	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &m_orthBufferData, sizeof(m_orthBufferData), sizeof(m_orthBufferData));
}



void Camera::CreateCubeProjection(
	_In_ float minimumFieldOfView,  // the minimum horizontal or vertical field of view, in degrees
	_In_ float aspectRatio,         // the aspect ratio of the projection (width / height)
	_In_ float nearPlane,           // depth to map to 0
	_In_ float farPlane             // depth to map to 1
)
{
	float minScale = 1.0f / tan(minimumFieldOfView * PI_F / 360.0f);
	float xScale = 1.0f;
	float yScale = 1.0f;
	if (aspectRatio < 1.0f)
	{
		xScale = minScale;
		yScale = minScale * aspectRatio;
	}
	else
	{
		xScale = minScale / aspectRatio;
		yScale = minScale;
	}
	float zScale = farPlane / (farPlane - nearPlane);
	cube_projection = DirectX::XMFLOAT4X4(
		xScale, 0.0f, 0.0f, 0.0f,
		0.0f, yScale, 0.0f, 0.0f,
		0.0f, 0.0f, -zScale, -nearPlane*zScale,
		0.0f, 0.0f, -1.0f, 0.0f
	);
}


DirectX::XMFLOAT4X4* Camera::GetViewMatrix()
{
	return &m_inverseView;

}

float Camera::LookingTanX()
{
	return view_tan.x;
}

float Camera::LookingTanZ()
{
	return view_tan.z;
}

float Camera::LookingX()
{
	return view_dir.x;
}

float Camera::LookingY()
{
	return view_dir.y;
}

float Camera::LookingZ()
{
	return view_dir.z;
}

float Camera::PositionX()
{

	return m_eye.x;
}

float Camera::PositionZ()
{
	return m_eye.z;
}

void Camera::SetFarDist(float _dist)
{
	far_dist = _dist;
	outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 1.3f;
	}

	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.1f,
		far_dist
		);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&close_projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);

	m_constantBufferData.projection = close_projection;
}

void Camera::buildWorldFrustumPlanes()
{
	XMMATRIX identityMatrix = XMMatrixIdentity();
	XMMATRIX projectionMatrix = XMLoadFloat4x4(&m_constantBufferData.projection);
	XMMATRIX viewMatrix = XMLoadFloat4x4(&m_constantBufferData.view);

	projectionMatrix = XMMatrixInverse(nullptr, projectionMatrix);
	
	viewMatrix = viewMatrix;// *XMLoadFloat4x4(&m_deviceResources->GetOrientationTransform3D());
	viewMatrix = XMMatrixInverse(nullptr, viewMatrix);
		
	XMMATRIX VP = XMMatrixMultiply(viewMatrix, projectionMatrix)*XMMatrixTranspose(XMLoadFloat4x4(&m_deviceResources->GetOrientationTransform3D()));;

	VP = XMMatrixInverse(nullptr, VP);
	XMFLOAT4X4 VPvalues;
	XMStoreFloat4x4(&VPvalues, VP);

	XMVECTOR col0 = XMVectorSet(VPvalues(0, 0), VPvalues(0, 1), VPvalues(0, 2), VPvalues(0, 3));
	XMVECTOR col1 = XMVectorSet(VPvalues(1, 0), VPvalues(1, 1), VPvalues(1, 2), VPvalues(1, 3));
	XMVECTOR col2 = XMVectorSet(VPvalues(2, 0), VPvalues(2, 1), VPvalues(2, 2), VPvalues(2, 3));
	XMVECTOR col3 = XMVectorSet(VPvalues(3, 0), VPvalues(3, 1), VPvalues(3, 2), VPvalues(3, 3));

	// Planes face inward.
	XMStoreFloat4(&mFrustumPlanes[0], -(col2 + col1)); // near
	XMStoreFloat4(&mFrustumPlanes[1], -(col2 - col1)); // far
	XMStoreFloat4(&mFrustumPlanes[2], -(col3 + col0)); // left
	XMStoreFloat4(&mFrustumPlanes[3], -(col3 - col0)); // right
	XMStoreFloat4(&mFrustumPlanes[4], -(col3 + col1)); // top
	XMStoreFloat4(&mFrustumPlanes[5], -(col3 - col1)); // bottom

	for (int i = 0; i < 6; i++)
		XMStoreFloat4(&mFrustumPlanes[i], XMPlaneNormalize(XMLoadFloat4(&mFrustumPlanes[i])));

}

float Camera::DistanceFromEye(float x, float y, float z)
{
	static float distance = sqrt((m_eye.x - x) * (m_eye.x - x) +
		(m_eye.y - y) * (m_eye.y - y) +
		(m_eye.z - z) * (m_eye.z - z));

	return distance;
}

bool Camera::CheckPlanes(float x, float y, float z, float radius)
{
	int i;
	XMVECTOR dist;
	XMVECTOR rel_pos = XMLoadFloat3(&XMFLOAT3(x - m_eye.x , y - m_eye.y, z - m_eye.z));
	float f_dist;

	for (i = 0; i < 6; i++) // 2 to 4 only does left to right
	{
		//if (i == 1)
		//	continue;
		//XMVector3Dot
		XMStoreFloat(&f_dist, XMVector3Dot(XMLoadFloat4(&mFrustumPlanes[i]), rel_pos));
		plane_dist[i] = f_dist;

		if (f_dist >radius)
		{
			return false;
		}
	}
	return true;
}

float Camera::DistanceEst(float x, float y, float z, float x1, float y1, float z1)
{
	/*
	if (abs(x1 - x) > 30.0f)
		return 999.9f;

	if (abs(y1 - y) > 30.0f)
		return 999.9f;

	if (abs(z1 - z) > 30.0f)
		return 999.9f;
		*/

	float dist1 = (x1 - x) * (x1 - x) +
		(y1 - y) * (y1 - y) +
		(z1 - z) * (z1 - z);

	XMStoreFloat(&dist1,XMVectorSqrtEst(XMLoadFloat(&dist1)));

	return dist1;
}

float Camera::Within3DManhattanDistance(float x, float y, float z, float x1, float y1, float z1, float distance)
{
	float dx = abs(x - x1);
	if (dx > distance) return -1.0f; // too far in x direction

	float dz = abs(z - z1);
	if (dz > distance) return -1.0f; // too far in z direction

	float dy = abs(y - y1);
	if (dy > distance) return -1.0f; // too far in y direction

	return 1.0f; // we're within the cube
}


float Camera::Distance(float x, float y, float z, float x1, float y1, float z1)
{
	float dist1 =	(x1 - x) * (x1 - x) +
					(y1 - y) * (y1 - y) +
					(z1 - z) * (z1 - z);

	XMStoreFloat(&dist1, XMVectorSqrt(XMLoadFloat(&dist1)));

	return dist1;
}



float Camera::CheckPoint(FLOAT x, FLOAT y, FLOAT z, FLOAT radius, float dist,float* full_dist)
{
	if (Within3DManhattanDistance(x, y, z, m_eye.x, m_eye.y, m_eye.z, dist) < 0.0f)
		return -1.0f;

	float dist_b = DistanceEst(m_eye.x, m_eye.y, m_eye.z, x, y, z);

	if (full_dist != nullptr)
	{
		*full_dist = dist_b;
	}
	if (CheckPlanes(x, y, z, radius) == true && dist_b < dist)
	{
		return dist_b;
	}
	else
	{
		return -1.0f;
	}
}

float Camera::CheckPoint(FLOAT x, FLOAT y, FLOAT z, FLOAT radius, float* full_dist)
{
	if (Within3DManhattanDistance(x, y, z, m_eye.x, m_eye.y, m_eye.z, far_dist+ radius) < 0.0f)
		return -1.0f;

	float dist = DistanceEst(m_eye.x, m_eye.y, m_eye.z, x, y, z);

	if (full_dist != nullptr)
	{
		*full_dist = dist;
	}
	if (CheckPlanes(x, y, z, radius) == true && dist < far_dist)
	{
		return dist;
	}
	else
	{
		return -1.0f;
	}
}


void Camera::GetWorldLine(UINT pixelX, UINT pixelY, DirectX::XMFLOAT3* outPoint, DirectX::XMFLOAT3* outDir)
{

	DirectX::XMFLOAT4 p0 = DirectX::XMFLOAT4((float)pixelX, (float)pixelY, 0, 1);
	DirectX::XMFLOAT4 p1 = DirectX::XMFLOAT4((float)pixelX, (float)pixelY, 1, 1);

	DirectX::XMVECTOR screen0 = DirectX::XMLoadFloat4(&p0);
	DirectX::XMVECTOR screen1 = DirectX::XMLoadFloat4(&p1);

	DirectX::XMMATRIX projMat = XMLoadFloat4x4(&m_constantBufferData.projection);
	DirectX::XMMATRIX viewMat = XMMatrixTranspose(XMLoadFloat4x4(&m_constantBufferData.view));

	//projMat = XMMatrixTranspose(projMat);
	//viewMat = XMMatrixTranspose(viewMat);

	XMMATRIX matScale = XMMatrixScaling(1.75f, 1.75f, 1.75f);
	matScale = XMMatrixInverse(nullptr, matScale);
	DirectX::XMVECTOR pp0 = DirectX::XMVector3Unproject(screen0, 0, 0, (float)outputSize.Width, (float)outputSize.Height, 0, 1, projMat*matScale, viewMat, XMMatrixIdentity());
	DirectX::XMVECTOR pp1 = DirectX::XMVector3Unproject(screen1, 0, 0, (float)outputSize.Width, (float)outputSize.Height, 0, 1, projMat*matScale, viewMat, XMMatrixIdentity());


	pp1 -= pp0;
	//pp1 = -pp1;
	DirectX::XMStoreFloat3(outPoint, pp0);
	DirectX::XMStoreFloat3(outDir, pp1);

	//outDir->x -= outPoint->x;
	//outDir->y -= outPoint->y;
	//outDir->z -= outPoint->z;




	/*
	XMMATRIX identityMatrix = XMMatrixTranspose(XMMatrixIdentity());
	XMMATRIX projectionMatrix = XMLoadFloat4x4(&m_constantBufferData.projection);
	XMMATRIX viewMatrix = XMLoadFloat4x4(&m_constantBufferData.view);


	//projectionMatrix = XMMatrixTranspose(projectionMatrix);
	
	//viewMatrix = viewMatrix;// *XMLoadFloat4x4(&m_deviceResources->GetOrientationTransform3D());
	//viewMatrix = XMMatrixTranspose(viewMatrix);
		
	XMMATRIX VP;// = XMMatrixMultiply(XMMatrixInverse(projectionMatrix), XMMatrixTranspose(viewMatrix));// *XMMatrixTranspose(XMLoadFloat4x4(&m_deviceResources->GetOrientationTransform3D()));;
	
	VP = XMMatrixMultiply(projectionMatrix, XMMatrixInverse(nullptr,XMMatrixTranspose(viewMatrix)));


	VP = XMMatrixInverse(nullptr, XMMatrixTranspose(VP));

	float winZ = 1.0;
	XMFLOAT4 in;
	XMVECTOR vIn;
	XMVECTOR vTran;

	XMFLOAT4 out;

	in.x = (2.0f*((float)(pixelX - 0) / ((float)outputSize.Width - 0))) - 1.0f,
	in.y = 1.0f - (2.0f*((float)(pixelY - 0) / ((float)outputSize.Height - 0)));
	in.z = 2.0* winZ - 1.0;
	in.w = 1.0;

	vIn = XMLoadFloat4(&in);
	vTran = XMVector4Transform( vIn , VP);

	XMStoreFloat4(&out, vTran);

	outDir->x = out.x * (1.0f / out.w);
	outDir->y = out.y * (1.0f / out.w);
	outDir->z = out.z * (1.0f / out.w);
	*/
}

void Camera::SetModelMatrix(DirectX::XMFLOAT4X4* p_ModelMatrix)
{
	m_constantBufferData.model = *p_ModelMatrix;

}

void Camera::SetViewMatrix(DirectX::XMFLOAT4X4* p_ViewMatrix)
{
	m_constantBufferData.view = *p_ViewMatrix;

}

void Camera::SetProjectionMatrix(DirectX::XMFLOAT4X4* p_ProjectionMatrix)
{
	m_constantBufferData.projection = *p_ProjectionMatrix;

}

void Camera::SetFlatModelMatrix()
{
	m_constantBufferData.model = flat_model_matrix;
}

void Camera::Update(DX::StepTimer const& timer)
{



}

void Camera::UpdateConstantBuffer(DirectX::XMFLOAT4X4 &proj_matrix, DirectX::XMFLOAT4X4 &view_matrix)
{
	m_constantBufferData.view = view_matrix;
	m_constantBufferData.projection = proj_matrix;

	XMStoreFloat4x4(&m_constantBufferData.mvp, XMLoadFloat4x4(&m_constantBufferData.projection) * XMLoadFloat4x4(&m_constantBufferData.view) *XMLoadFloat4x4(&m_constantBufferData.model));

	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0);
}

void Camera::UpdateConstantBuffer(bool bUpdateCameraPos)
{
	
	XMStoreFloat4x4(&m_constantBufferData.mvp, XMLoadFloat4x4(&m_constantBufferData.projection) * XMLoadFloat4x4(&m_constantBufferData.view) *XMLoadFloat4x4(&m_constantBufferData.model));
	
	
	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0);



	if (bUpdateCameraPos == true)
	{
		// Not needed get eye pos from view matrix
		/*
		m_constantbuffercameraPos.cameraPosition = m_eye;
		m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_cameraBuffer.Get(), 0, NULL, &m_constantbuffercameraPos, 0, 0);
		*/
	}
}

void Camera::CreateWindowSizeDependentResources()
{
	outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	/*
	if (outputSize.Height > outputSize.Width)
	{
		aspectRatio = outputSize.Height / outputSize.Width;
	}
	*/

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 1.3f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		1.0f,
		far_dist
		);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&close_projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);
	
	m_constantBufferData.projection = close_projection;


	perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.5f,
		2500.0f
	);

	orientation = m_deviceResources->GetOrientationTransform3D();

	orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&sky_projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
	);




	// make orth matrixes

	// Initialize the world matrix to the identity matrix.
	XMStoreFloat4x4(&m_orthBufferData.model, XMMatrixIdentity());

	XMStoreFloat4x4(&m_orthBufferData.projection, XMMatrixOrthographicRH(outputSize.Width, outputSize.Height, 0.1f, 100.0f));

	XMFLOAT3 pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	XMStoreFloat4x4(&m_orthBufferData.view, XMMatrixLookAtRH(XMLoadFloat3(&pos), XMLoadFloat3(&look), XMLoadFloat3(&up)));




	//SetViewParams(XMFLOAT3(0.0f, 30.0f, 0.5f), XMFLOAT3(0.0f, -2.1f, 0.0f));
}



void Camera::SetSkyCubeProjection()
{
	m_constantBufferData.projection = skycube_projection;
}


void Camera::SetCubeProjection()
{
	m_constantBufferData.projection = cube_projection;
}

void Camera::SetSkyProjection()
{
	m_constantBufferData.projection = sky_projection;
}

void Camera::SetCloseProjection()
{
	m_constantBufferData.projection = close_projection;
}
DirectX::XMFLOAT3 Camera::GetEye()
{
	return m_eye;
}

void Camera::SetViewAndTan(float vx, float vy, float vz, float tx, float ty, float tz)
{
	view_dir = DirectX::XMFLOAT3(vx, vy, vz);
	view_tan = DirectX::XMFLOAT3(tx, ty, tz);

}

void Camera::SetViewParams(
	_In_ DirectX::XMFLOAT3 eye,
	_In_ DirectX::XMFLOAT3 lookAt,
	_In_ DirectX::XMFLOAT3 up)
{
	m_eye = eye;
	m_lookAt = lookAt;
	m_up = up;

	

	XMMATRIX view = XMMatrixLookAtRH(
		XMLoadFloat3(&m_eye),
		XMLoadFloat3(&m_lookAt),
		XMLoadFloat3(&m_up)
		);

	XMMATRIX inverseView = XMMatrixInverse(nullptr, view);
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(view));

	XMMATRIX prjident = XMMatrixIdentity();
	XMMATRIX inverseView2 = XMMatrixInverse(nullptr, XMMatrixMultiply(prjident, inverseView));
	XMStoreFloat4x4(&m_inverseView, inverseView);


	XMFLOAT3 water_pos= XMFLOAT3(0.0f, 1.0f, 0.0f); // position of water plane
	XMFLOAT3 water_norm = XMFLOAT3(0.0f, 1.0f, 0.0f); // normal of water plane

	XMVECTOR water_plane = XMPlaneFromPointNormal(XMLoadFloat3(&water_pos), XMLoadFloat3(&water_norm));
	
	XMMATRIX reflect_matrix = XMMatrixReflect(water_plane);
	XMMATRIX m_reflectionView = XMMatrixMultiply(reflect_matrix, view);
	

}



DirectX::XMFLOAT4X4 Camera::MakeViewParams(
	_In_ DirectX::XMFLOAT3 eye,
	_In_ DirectX::XMFLOAT3 lookAt,
	_In_ DirectX::XMFLOAT3 up)
{
	DirectX::XMFLOAT4X4 return_val;


	XMMATRIX view = XMMatrixLookAtRH(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&lookAt),
		XMLoadFloat3(&up)
	);

	XMStoreFloat4x4(&return_val, XMMatrixTranspose(view));

	return return_val;
}