#include "pch.h"
#include "DefParticle.h"
#include "Lights.h"


using namespace Game;

using namespace concurrency;
using namespace DirectX;
using namespace Windows::Foundation;
using namespace Microsoft::WRL;


Lights::Lights(std::shared_ptr<DX::DeviceResources> pm_deviceResources, Camera* p_Camera)
{
	m_deviceResources = pm_deviceResources;
	m_Camera = p_Camera;
	SetLights1(); // Set to a default value

	// Update the 3D projection matrix of the environment camera.
	CreateSpotProjection(
		90.0f,   
		1.0f,
		// Use a 90-degree vertical field of view
		//static_cast<float>(10) /
		//static_cast<float>(10),								// Specify the aspect ratio of the window.
		0.1f,                                                  // Specify the nearest Z-distance at which to draw vertices.
		LIGHT_RADIUS                                           // Specify the farthest Z-distance at which to draw vertices.
	);
	//m_lightBufferData.dirProj = spot_projection;

//	XMFLOAT3 fvec = XMFLOAT3(10.0f, 12.0f, 2.0f);

//	XMVECTOR vec1 = XMLoadFloat3(&fvec);
	
//	XMVECTOR vec2 = XMVector3Transform(vec1, XMLoadFloat4x4(&m_lightBufferData.dirInvProj));

//	XMFLOAT3 vec3;
//	XMStoreFloat3(&vec3, vec2);
	ResetLights();

	m_RenderCubeArray = new RenderCubeArray();
	m_RenderCubeArray->Initialize(m_deviceResources,1024,1024, MAX_POINT_SHADOWS, DXGI_FORMAT_R16_FLOAT);

	m_SpotTextureArray = new RenderTextureArray;
	m_SpotTextureArray->Initialize(m_deviceResources->GetD3DDevice(), 512, 512, MAX_SPOT_SHADOWS, DXGI_FORMAT_R16_FLOAT);


	CreateStructuredBuffer(m_deviceResources->GetD3DDevice(), MAX_POINT_LIGHTS, true, false, m_pointLightBuffer.GetAddressOf(), m_pointLightView.GetAddressOf(),
		m_pointLightAccessView.GetAddressOf());

	CreateStructuredBuffer(m_deviceResources->GetD3DDevice(), MAX_SPOT_LIGHTS, true, false, m_spotLightBuffer.GetAddressOf(), m_spotLightView.GetAddressOf(),
		m_spotLightAccessView.GetAddressOf());


	buildPointFrustumPlanes();
}



void Lights::AddSpot(CG_SPOT_LIGHT &spot_data)
{
	if (m_spotLights.size() == MAX_SPOT_LIGHTS)
		return;

	//return;

	PointLightType spot_light;

	spot_data.ambient.x *= spot_data.ambient.w;
	spot_data.ambient.y *= spot_data.ambient.w;
	spot_data.ambient.z *= spot_data.ambient.w;
	spot_data.ambient.w = 1.0f;

	spot_data.diffuse.x *= spot_data.diffuse.w;
	spot_data.diffuse.y *= spot_data.diffuse.w;
	spot_data.diffuse.z *= spot_data.diffuse.w;
	spot_data.diffuse.w = 1.0f;

	spot_data.specular.x *= spot_data.specular.w;
	spot_data.specular.y *= spot_data.specular.w;
	spot_data.specular.z *= spot_data.specular.w;
	spot_data.specular.w = 1.0f;





	spot_light.ambient = spot_data.ambient;
	spot_light.diffuse = spot_data.diffuse;
	spot_light.specular = spot_data.specular;
	spot_light.pos = spot_data.pos;
	spot_light.radius = LIGHT_RADIUS;
	spot_light.spot = 3.0f;
	spot_light.shadow_index = 0;
	spot_light.specular_power = spot_data._specular_power;
	spot_light.lightmap = spot_data.lightmap;
	spot_light.dir = spot_data.dir;
	m_eye = spot_data.pos;


	//spot_data.pos.x += 5.0f;
	DirectX::XMFLOAT3 look_at = XMFLOAT3(spot_data.pos.x + spot_data.dir.x, spot_data.pos.y + spot_data.dir.y, spot_data.pos.z + spot_data.dir.z);
	DirectX::XMFLOAT3 position = spot_data.pos;
	DirectX::XMFLOAT3 up_dir = spot_data.up;
	//DirectX::XMFLOAT4X4 view = m_Camera->MakeViewParams(pos, look_at, up);
	spotView = m_Camera->MakeViewParams(position, look_at, up_dir);
	//m_lightBufferData.dirView = spotView;
	
	spot_light.dirView = spotView;
	spot_light.dirProj = spot_projection;


	if (spot_data.bCastShadows == true && current_spot_shadow_index < MAX_SPOT_SHADOWS)
	{
		spot_light.shadow_index = current_spot_shadow_index;
		spot_shadow[current_spot_shadow_index].spot_index = m_spotLights.size();
		spot_shadow[current_spot_shadow_index].dirView = spot_light.dirView;
		spot_shadow[current_spot_shadow_index].dirProj = spot_light.dirProj;
		//point_light.shadow_cube_index = m_lightBufferData.lights[num_point_lights].shadow_cube_index;
		//m_RenderCube[current_shadow_index]->SetRenderPosition(point_data.pos.x, point_data.pos.y, point_data.pos.z);
		//m_RenderCubeArray->SetRenderPosition(current_spot_shadow_index, spot_data.pos.x, spot_data.pos.y, spot_data.pos.z);
		current_spot_shadow_index++;

	}
	else
	{
		//m_lightBufferData.lights[num_point_lights].shadow_cube_index = 0;
		spot_light.shadow_index = -1;
	}


	m_lightBufferData.numSpotLights++;


	m_spotLights.push_back(spot_light);

	//far_dist = spot_data.radius;

	//BuildSpotFrustumPlanes();
}





void Lights::AddPoint(CG_POINT_LIGHT &point_data)
{
	if (m_pointLights.size() == MAX_POINT_LIGHTS)
		return;

	PointLightType point_light;

	point_data.ambient.x *= point_data.ambient.w;
	point_data.ambient.y *= point_data.ambient.w;
	point_data.ambient.z *= point_data.ambient.w;
	point_data.ambient.w = 1.0f;

	point_data.diffuse.x *= point_data.diffuse.w;
	point_data.diffuse.y *= point_data.diffuse.w;
	point_data.diffuse.z *= point_data.diffuse.w;
	point_data.diffuse.w = 1.0f;

	point_data.specular.x *= point_data.specular.w;
	point_data.specular.y *= point_data.specular.w;
	point_data.specular.z *= point_data.specular.w;
	point_data.specular.w = 1.0f;


	point_light.ambient = point_data.ambient;
	point_light.diffuse = point_data.diffuse;
	point_light.specular = point_data.specular;
	point_light.pos = point_data.pos;
	point_light.radius = LIGHT_RADIUS;
	point_light.spot = 0.0f;
	point_light.specular_power = point_data._specular_power;


	if (point_data.bCastShadows == true && current_point_shadow_index < MAX_POINT_SHADOWS)
	{
		point_light.shadow_index = current_point_shadow_index;

		//point_light.shadow_cube_index = m_lightBufferData.lights[num_point_lights].shadow_cube_index;
		//m_RenderCube[current_shadow_index]->SetRenderPosition(point_data.pos.x, point_data.pos.y, point_data.pos.z);
		m_RenderCubeArray->SetRenderPosition(current_point_shadow_index, point_data.pos.x, point_data.pos.y, point_data.pos.z);
		current_point_shadow_index++;

	}
	else
	{
		//m_lightBufferData.lights[num_point_lights].shadow_cube_index = 0;
		point_light.shadow_index = -1;
	}

	m_lightBufferData.numPointLights++;

	m_pointLights.push_back(point_light);

}

void Lights::UpdateConstantBuffer()
{
	m_lightBufferData.numPointLights = m_pointLights.size();
	m_lightBufferData.numSpotLights = m_spotLights.size();

	m_lightBufferData.eye_position = m_Camera->GetEye();
	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_lightBuffer.Get(), 0, NULL, &m_lightBufferData, 0, 0);
	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_VlightBuffer.Get(), 0, NULL, &m_lightBufferData, 0, 0);

	UpdatePointLightBuffer(m_deviceResources->GetD3DDeviceContext());
	UpdateSpotLightBuffer(m_deviceResources->GetD3DDeviceContext());
}


void Lights::SetEyePosition(float x, float y, float z)
{
	m_lightBufferData.eye_position = DirectX::XMFLOAT3(x, y, z);
}



bool Lights::UpdateSpotLightBuffer(ID3D11DeviceContext* deviceContext)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result;
	PointLightType* lightsPtr;

	result = deviceContext->Map(m_spotLightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	lightsPtr = (PointLightType*)mappedResource.pData;

	memcpy(lightsPtr, m_spotLights.data(), (sizeof(PointLightType) * (m_spotLights.size())));

	deviceContext->Unmap(m_spotLightBuffer.Get(), 0);

	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(51, 1, m_spotLightView.GetAddressOf());
	m_deviceResources->GetD3DDeviceContext()->VSSetShaderResources(51, 1, m_spotLightView.GetAddressOf());

	return true;
}

bool Lights::UpdatePointLightBuffer(ID3D11DeviceContext* deviceContext)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result;
	PointLightType* lightsPtr;

	result = deviceContext->Map(m_pointLightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	lightsPtr = (PointLightType*)mappedResource.pData;

	memcpy(lightsPtr, m_pointLights.data(), (sizeof(PointLightType) * (m_pointLights.size())));

	deviceContext->Unmap(m_pointLightBuffer.Get(), 0);

	m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(50, 1, m_pointLightView.GetAddressOf());
	m_deviceResources->GetD3DDeviceContext()->VSSetShaderResources(50, 1, m_pointLightView.GetAddressOf());

	return true;
}



HRESULT Lights::CreateStructuredBuffer(
	ID3D11Device*               pd3dDevice,
	const UINT                  iNumElements,
	const bool                  isCpuWritable,
	const bool                  isGpuWritable,
	ID3D11Buffer**              ppBuffer,
	ID3D11ShaderResourceView**  ppSRV,
	ID3D11UnorderedAccessView** ppUAV,
	const PointLightType*       pInitialData)
{
	HRESULT hr = S_OK;

	assert(pd3dDevice != NULL);
	assert(ppBuffer != NULL);
	assert(ppSRV != NULL);

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = iNumElements * sizeof(PointLightType);

	if ((!isCpuWritable) && (!isGpuWritable))
	{
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	}
	else if (isCpuWritable && (!isGpuWritable))
	{
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	}
	else if ((!isCpuWritable) && isGpuWritable)
	{
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.BindFlags = (D3D11_BIND_SHADER_RESOURCE |
			D3D11_BIND_UNORDERED_ACCESS);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	}
	else
	{
		assert((!(isCpuWritable && isGpuWritable)));
	}

	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof(PointLightType);

	D3D11_SUBRESOURCE_DATA bufferInitData;
	ZeroMemory((&bufferInitData), sizeof(bufferInitData));
	bufferInitData.pSysMem = pInitialData;

	pd3dDevice->CreateBuffer((&bufferDesc),
		(pInitialData) ? (&bufferInitData) : NULL,
		ppBuffer);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.ElementWidth = iNumElements;
	pd3dDevice->CreateShaderResourceView(*ppBuffer, &srvDesc, ppSRV);

	if (isGpuWritable)
	{
		assert(ppUAV != NULL);

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory((&uavDesc), sizeof(uavDesc));
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.NumElements = iNumElements;
		pd3dDevice->CreateUnorderedAccessView(*ppBuffer, &uavDesc, ppUAV);
	}

	return hr;
}


void Lights::RestoreDepthViewToSpotView()
{

	UpdateConstantBuffer();
}


void Lights::setPointFrustumPlanePosition(float x, float y, float z)
{
	int i,j;

	XMMATRIX t_mat = XMMatrixTranslation(x, y, z);

	for (i = 0; i < 6; i++)
	{
		for (j = 0; j < 6; j++)
		{
			XMStoreFloat4(&mTransPointFrustumPlanes[i][j], XMPlaneTransform(XMLoadFloat4(&mPointFrustumPlanes[i][j]), t_mat));
		}
	}
}

void Lights::setPointFrustumPlanePosition(int _cube_index)
{
	int i, j;

	current_shadow_cube_index = _cube_index;

	//XMMATRIX t_mat = XMMatrixTranslation(m_RenderCube[_cube_index]->render_position.x, m_RenderCube[_cube_index]->render_position.y, m_RenderCube[_cube_index]->render_position.z);

	XMMATRIX t_mat = XMMatrixTranslation(m_RenderCubeArray->render_position[_cube_index].x, m_RenderCubeArray->render_position[_cube_index].y, m_RenderCubeArray->render_position[_cube_index].z);


	for (i = 0; i < 6; i++)
	{
		for (j = 0; j < 6; j++)
		{
			XMStoreFloat4(&mTransPointFrustumPlanes[i][j], XMPlaneTransform(XMLoadFloat4(&mPointFrustumPlanes[i][j]), t_mat));
		}
	}

}

float Lights::Within3DManhattanDistance(float x, float y, float z, float x1, float y1, float z1, float distance)
{
	float dx = abs(x - x1);
	if (dx > distance) return -1.0f; // too far in x direction

	float dz = abs(z - z1);
	if (dz > distance) return -1.0f; // too far in z direction

	float dy = abs(y - y1);
	if (dy > distance) return -1.0f; // too far in y direction

	return 1.0f; // we're within the cube
}

float Lights::PointCubeWithin3DManhattanDistance(float x, float y, float z, float distance)
{
	float dx = abs(x - m_RenderCubeArray->render_position[current_shadow_cube_index].x);
	if (dx > distance) return -1.0f; // too far in x direction

	float dz = abs(z - m_RenderCubeArray->render_position[current_shadow_cube_index].z);
	if (dz > distance) return -1.0f; // too far in z direction

	float dy = abs(y - m_RenderCubeArray->render_position[current_shadow_cube_index].y);
	if (dy > distance) return -1.0f; // too far in y direction

	return 1.0f; // we're within the cube
}



bool Lights::CheckPointPlanes(int _direction, float x, float y, float z, float radius)
{
	int i;
	
	if (Within3DManhattanDistance(x, y, z, m_Camera->m_eye.x, m_Camera->m_eye.y, m_Camera->m_eye.z, MAX_LIGHT_DISTANCE+ radius) < 0.0f)
		return false;

	
	XMVECTOR dist;
	XMVECTOR rel_pos;
	float f_dist;

	rel_pos = XMLoadFloat3(&XMFLOAT3(x - m_RenderCubeArray->render_position[current_shadow_cube_index].x, y - m_RenderCubeArray->render_position[current_shadow_cube_index].y, z - m_RenderCubeArray->render_position[current_shadow_cube_index].z));


	for (i = 0; i < 6; i++) // 2 to 4 only does left to right
	{
		//if (i == 1)
		//	continue;
		//XMVector3Dot
		XMStoreFloat(&f_dist, XMVector3Dot(XMLoadFloat4(&mTransPointFrustumPlanes[_direction][i]), rel_pos));
		//plane_dist[i] = f_dist;

		if (f_dist >radius)
		{
			return false;
		}
	}
	return true;
}


void Lights::buildPointFrustumPlanes()
{
	int i;

	DirectX::XMFLOAT3 environmentMapTargets[NumberOfFaces] = {
		DirectX::XMFLOAT3(1, 0, 0),  // Point camera at negative x axis.
		DirectX::XMFLOAT3(-1, 0, 0),   // Point camera at positive x axis.
		DirectX::XMFLOAT3(0, 1, 0),   // Point camera at positive y axis.
		DirectX::XMFLOAT3(0, -1, 0),  // Point camera at negative y axis.
		DirectX::XMFLOAT3(0, 0, 1),   // Point camera at positive z axis.
		DirectX::XMFLOAT3(0, 0, -1)   // Point camera at negative z axis.
	};


	// Define the up vector for each Environment Map face.
	DirectX::XMFLOAT3 environmentMapUp[NumberOfFaces] = {
		DirectX::XMFLOAT3(0, 1, 0),
		DirectX::XMFLOAT3(0, 1, 0),
		DirectX::XMFLOAT3(0, 0, -1),
		DirectX::XMFLOAT3(0, 0, 1),
		DirectX::XMFLOAT3(0, 1, 0),
		DirectX::XMFLOAT3(0, 1, 0)
	};

	for (i = 0; i < 6; i++)
	{
		XMMATRIX cam_view = XMMatrixLookAtRH(
			XMLoadFloat3(&XMFLOAT3(0.0f,0.0f,0.01f)),
			XMLoadFloat3(&XMFLOAT3(environmentMapTargets[i].x, environmentMapTargets[i].y, environmentMapTargets[i].z)),
			XMLoadFloat3(&XMFLOAT3(environmentMapUp[i].x, environmentMapUp[i].y, environmentMapUp[i].z))
		);


		XMMATRIX identityMatrix = XMMatrixIdentity();
		XMMATRIX projectionMatrix = XMLoadFloat4x4(&spot_projection);
		XMMATRIX viewMatrix = cam_view;

		projectionMatrix = XMMatrixInverse(nullptr, projectionMatrix);

		viewMatrix = viewMatrix;// *XMLoadFloat4x4(&m_deviceResources->GetOrientationTransform3D());
		viewMatrix = XMMatrixInverse(nullptr, viewMatrix);

		XMMATRIX VP = XMMatrixMultiply(viewMatrix, projectionMatrix);// *XMMatrixTranspose(XMLoadFloat4x4(&m_deviceResources->GetOrientationTransform3D()));;

		VP = XMMatrixInverse(nullptr, VP);
		XMFLOAT4X4 VPvalues;
		XMStoreFloat4x4(&VPvalues, VP);

		XMVECTOR col0 = XMVectorSet(VPvalues(0, 0), VPvalues(0, 1), VPvalues(0, 2), VPvalues(0, 3));
		XMVECTOR col1 = XMVectorSet(VPvalues(1, 0), VPvalues(1, 1), VPvalues(1, 2), VPvalues(1, 3));
		XMVECTOR col2 = XMVectorSet(VPvalues(2, 0), VPvalues(2, 1), VPvalues(2, 2), VPvalues(2, 3));
		XMVECTOR col3 = XMVectorSet(VPvalues(3, 0), VPvalues(3, 1), VPvalues(3, 2), VPvalues(3, 3));

		// Planes face inward.
		XMStoreFloat4(&mPointFrustumPlanes[i][0], -(col2 + col1)); // near
		XMStoreFloat4(&mPointFrustumPlanes[i][1], -(col2 - col1)); // far
		XMStoreFloat4(&mPointFrustumPlanes[i][2], -(col3 + col0)); // left
		XMStoreFloat4(&mPointFrustumPlanes[i][3], -(col3 - col0)); // right
		XMStoreFloat4(&mPointFrustumPlanes[i][4], -(col3 + col1)); // top
		XMStoreFloat4(&mPointFrustumPlanes[i][5], -(col3 - col1)); // bottom

		for (int j = 0; j < 6; j++)
			XMStoreFloat4(&mPointFrustumPlanes[i][j], XMPlaneNormalize(XMLoadFloat4(&mPointFrustumPlanes[i][j])));
	}

}

void Lights::ClearSpotCubeDepth()
{

}

void Lights::RenderPointDepthCubSide(int _side, int _cube_index)
{
	m_Camera->m_constantBufferData.projection = spot_projection;

	//XMStoreFloat4x4(&m_Camera->m_constantBufferData.view, XMMatrixTranspose(m_RenderCube[_cube_index]->RenderCubeSide(_side)));

	XMStoreFloat4x4(&m_Camera->m_constantBufferData.view, XMMatrixTranspose(m_RenderCubeArray->RenderCubeSide(_cube_index,_side)));

	//UpdateConstantBuffer();


}




void Lights::CreateSpotProjection(
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
	spot_projection = DirectX::XMFLOAT4X4(
		xScale, 0.0f, 0.0f, 0.0f,
		0.0f, yScale, 0.0f, 0.0f,
		0.0f, 0.0f, -zScale, -nearPlane*zScale,
		0.0f, 0.0f, -1.0f, 0.0f
	);
}

void Lights::SetLights1()
{
	m_lightBufferData.ambientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_lightBufferData.diffuseColor = XMFLOAT4(1.5f, 1.5f, 1.5f, 1.0f);
	m_lightBufferData.lightDirection = XMFLOAT3(-0.4f, -1.0f, -0.2f);
	m_lightBufferData.specularColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_lightBufferData.lightning = 0.0f;
	m_lightBufferData.lightning_dir = XMFLOAT3(-0.2f, -1.0f, -0.4f);
	m_lightBufferData.specularPower = 50.0f;
}

void Lights::SetFog(DirectX::XMFLOAT4 col)
{
	m_lightBufferData.fogColor = col;
}


void Lights::SetSpecular(float level)
{
	m_lightBufferData.specularPower = level;
}


void Lights::SetNumTextures(int _texs)
{
	//m_lightBufferData.numTextures = _texs;

}











void Lights::BuildSpotFrustumPlanes()
{
	XMMATRIX identityMatrix = XMMatrixIdentity();





	XMMATRIX projectionMatrix = XMLoadFloat4x4(&spot_projection);
	XMMATRIX viewMatrix = XMLoadFloat4x4(&spot_projection);

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

bool Lights::CheckPlanes(float x, float y, float z, float radius)
{
	int i;
	XMVECTOR dist;
	XMVECTOR rel_pos = XMLoadFloat3(&XMFLOAT3(x - m_eye.x, y - m_eye.y, z - m_eye.z));
	float f_dist;

	for (i = 0; i < 6; i++) // 2 to 4 only does left to right
	{
		//if (i == 1)
		//	continue;
		//XMVector3Dot
		XMStoreFloat(&f_dist, XMVector3Dot(XMLoadFloat4(&mFrustumPlanes[i]), rel_pos));
		//plane_dist[i] = f_dist;

		if (f_dist >radius)
		{
			return false;
		}
	}
	return true;
}


float Lights::DistanceEst(float x, float y, float z, float x1, float y1, float z1)
{
	float dist1 = (x1 - x) * (x1 - x) +
		(y1 - y) * (y1 - y) +
		(z1 - z) * (z1 - z);

	XMStoreFloat(&dist1, XMVectorSqrtEst(XMLoadFloat(&dist1)));

	return dist1;
}


float Lights::CheckPoint(FLOAT x, FLOAT y, FLOAT z, FLOAT radius, float dist, float* full_dist)
{
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

float Lights::CheckPoint(FLOAT x, FLOAT y, FLOAT z, FLOAT radius, float* full_dist)
{
	float dist = DistanceEst(m_eye.x, m_eye.y, m_eye.z, x, y, z);

	if (full_dist != nullptr)
	{
		*full_dist = dist;
	}
	if (CheckPlanes(x, y, z, radius) == true && dist < LIGHT_RADIUS)
	{
		return dist;
	}
	else
	{
		return -1.0f;
	}
}

