#include "pch.h"
#include "ModelLoader.h"


FbxManager* g_pFbxSdkManager = nullptr;

using namespace Game;

using namespace std;
using namespace DirectX;
using namespace DX;
using namespace Microsoft::WRL;


int ModelSortCB(const VOID* arg1, const VOID* arg2)
{
	part_index* p1 = (part_index*)arg1;
	part_index* p2 = (part_index*)arg2;

	if (p1->dist == p2->dist)
	{
		return 0;
	}
	else
	{
		if (p1->dist<p2->dist)
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}
}



inline bool XM_CALLCONV Intersects(FXMVECTOR Origin, FXMVECTOR Direction, FXMVECTOR V0, GXMVECTOR V1, HXMVECTOR V2, float& Dist)
{
	assert(DirectX::Internal::XMVector3IsUnit(Direction));

	XMVECTOR Zero = XMVectorZero();

	XMVECTOR e1 = V1 - V0;
	XMVECTOR e2 = V2 - V0;

	// p = Direction ^ e2;
	XMVECTOR p = XMVector3Cross(Direction, e2);

	// det = e1 * p;
	XMVECTOR det = XMVector3Dot(e1, p);

	XMVECTOR u, v, t;

	if (XMVector3GreaterOrEqual(det, g_RayEpsilon))
	{
		// Determinate is positive (front side of the triangle).
		XMVECTOR s = Origin - V0;

		// u = s * p;
		u = XMVector3Dot(s, p);

		XMVECTOR NoIntersection = XMVectorLess(u, Zero);
		NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(u, det));

		// q = s ^ e1;
		XMVECTOR q = XMVector3Cross(s, e1);

		// v = Direction * q;
		v = XMVector3Dot(Direction, q);

		NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(v, Zero));
		NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(u + v, det));

		// t = e2 * q;
		t = XMVector3Dot(e2, q);

		NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(t, Zero));

		if (XMVector4EqualInt(NoIntersection, XMVectorTrueInt()))
		{
			Dist = 0.f;
			return false;
		}
	}
	else if (XMVector3LessOrEqual(det, g_RayNegEpsilon))
	{
		// Determinate is negative (back side of the triangle).
		XMVECTOR s = Origin - V0;

		// u = s * p;
		u = XMVector3Dot(s, p);

		XMVECTOR NoIntersection = XMVectorGreater(u, Zero);
		NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(u, det));

		// q = s ^ e1;
		XMVECTOR q = XMVector3Cross(s, e1);

		// v = Direction * q;
		v = XMVector3Dot(Direction, q);

		NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(v, Zero));
		NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(u + v, det));

		// t = e2 * q;
		t = XMVector3Dot(e2, q);

		NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(t, Zero));

		if (XMVector4EqualInt(NoIntersection, XMVectorTrueInt()))
		{
			Dist = 0.f;
			return false;
		}
	}
	else
	{
		// Parallel ray.
		Dist = 0.f;
		return false;
	}

	t = XMVectorDivide(t, det);

	// (u / det) and (v / dev) are the barycentric cooridinates of the intersection.

	// Store the x-component to *pDist
	XMStoreFloat(&Dist, t);

	return true;
}

float ModelLoader::RayHit(float x_point, float y_point, float z_point, float x_dir, float y_dir, float z_dir, DirectX::XMFLOAT4X4 _model_matrix)
{
	float min_dist = 999.0f;
	float dist = 0.0f;

	XMVECTOR rayfrom = XMLoadFloat3(&XMFLOAT3(x_point, y_point, z_point));
	XMVECTOR raydir = XMLoadFloat3(&XMFLOAT3(x_dir, y_dir, z_dir));

	btMatrix3x3 mat = btMatrix3x3(_model_matrix._11, _model_matrix._12, _model_matrix._13, _model_matrix._21, _model_matrix._22, _model_matrix._23, _model_matrix._31, _model_matrix._32, _model_matrix._33);

	btVector3 v1, v2, v3, v1b, v2b, v3b, n1, n2, n3;

	for (int i = 0; i < m_indicesCount; i += 3)
	{
		if (true)
		{

			if (object_scale > 1.0f || object_scale < -1.0f)
			{
				v1 = btVector3(m_vertices[m_indices[i]].pos.x / object_scale, m_vertices[m_indices[i]].pos.y / object_scale, m_vertices[m_indices[i]].pos.z / object_scale);
				v2 = btVector3(m_vertices[m_indices[i + 1]].pos.x / object_scale, m_vertices[m_indices[i + 1]].pos.y / object_scale, m_vertices[m_indices[i + 1]].pos.z / object_scale);
				v3 = btVector3(m_vertices[m_indices[i + 2]].pos.x / object_scale, m_vertices[m_indices[i + 2]].pos.y / object_scale, m_vertices[m_indices[i + 2]].pos.z / object_scale);
			}
			else
			{
				v1 = btVector3(m_vertices[m_indices[i]].pos.x * object_scale, m_vertices[m_indices[i]].pos.y * object_scale, m_vertices[m_indices[i]].pos.z * object_scale);
				v2 = btVector3(m_vertices[m_indices[i + 1]].pos.x * object_scale, m_vertices[m_indices[i + 1]].pos.y * object_scale, m_vertices[m_indices[i + 1]].pos.z * object_scale);
				v3 = btVector3(m_vertices[m_indices[i + 2]].pos.x * object_scale, m_vertices[m_indices[i + 2]].pos.y * object_scale, m_vertices[m_indices[i + 2]].pos.z * object_scale);
			}
		}
		else
		{
			v1 = btVector3(m_vertices[m_indices[i]].pos.x, m_vertices[m_indices[i]].pos.y, m_vertices[m_indices[i]].pos.z);
			v2 = btVector3(m_vertices[m_indices[i + 1]].pos.x, m_vertices[m_indices[i + 1]].pos.y, m_vertices[m_indices[i + 1]].pos.z);
			v3 = btVector3(m_vertices[m_indices[i + 2]].pos.x, m_vertices[m_indices[i + 2]].pos.y, m_vertices[m_indices[i + 2]].pos.z);

		}
		v1b = mat * v1;
		v2b = mat * v2;
		v3b = mat * v3;




		XMVECTOR triangleV0 = XMLoadFloat3(&XMFLOAT3(v1b.getX() + _model_matrix._14, v1b.getY() + _model_matrix._24, v1b.getZ() + _model_matrix._34));
		XMVECTOR triangleV1 = XMLoadFloat3(&XMFLOAT3(v2b.getX() + _model_matrix._14, v2b.getY() + _model_matrix._24, v2b.getZ() + _model_matrix._34));
		XMVECTOR triangleV2 = XMLoadFloat3(&XMFLOAT3(v3b.getX() + _model_matrix._14, v3b.getY() + _model_matrix._24, v3b.getZ() + _model_matrix._34));

		if (Intersects(rayfrom, raydir, triangleV0, triangleV1, triangleV2, dist) == true)
		{
			if (dist < min_dist)
			{
				min_dist = dist;

				n1 = btVector3(m_vertices[m_indices[i]].norm.x, m_vertices[m_indices[i]].norm.y, m_vertices[m_indices[i]].norm.z);
				n2 = btVector3(m_vertices[m_indices[i + 1]].norm.x, m_vertices[m_indices[i + 1]].norm.y, m_vertices[m_indices[i + 1]].norm.z);
				n3 = btVector3(m_vertices[m_indices[i + 2]].norm.x, m_vertices[m_indices[i + 2]].norm.y, m_vertices[m_indices[i + 2]].norm.z);

				ray_normal = n1;// (n1 + n2 + n3) / 3;

			}
		}

		/*
		triangleV2 = XMLoadFloat3(&XMFLOAT3(v1b.getX() + _model_matrix._14, v1b.getY() + _model_matrix._24, v1b.getZ() + _model_matrix._34));
		triangleV1 = XMLoadFloat3(&XMFLOAT3(v2b.getX() + _model_matrix._14, v2b.getY() + _model_matrix._24, v2b.getZ() + _model_matrix._34));
		triangleV0 = XMLoadFloat3(&XMFLOAT3(v3b.getX() + _model_matrix._14, v3b.getY() + _model_matrix._24, v3b.getZ() + _model_matrix._34));

		if (Intersects(rayfrom, raydir, triangleV0, triangleV1, triangleV2, dist) == true)
		{
		if (dist < min_dist)
		{
		min_dist = dist;

		n1 = btVector3(m_vertices[m_indices[i]].normal.x, m_vertices[m_indices[i]].normal.y, m_vertices[m_indices[i]].normal.z);
		n2 = btVector3(m_vertices[m_indices[i + 1]].normal.x, m_vertices[m_indices[i + 1]].normal.y, m_vertices[m_indices[i + 1]].normal.z);
		n3 = btVector3(m_vertices[m_indices[i + 2]].normal.x, m_vertices[m_indices[i + 2]].normal.y, m_vertices[m_indices[i + 2]].normal.z);

		ray_normal = n1;// (n1 + n2 + n3) / 3;

		}
		}
		*/
	}

	return min_dist;

}



int ModelLoader::copy_file(char *old_filename, char  *new_filename)
{
	FILE  *ptr_old, *ptr_new;
	errno_t err = 0, err1 = 0;
	int  a;

	err = fopen_s(&ptr_old, old_filename, "rb");
	err1 = fopen_s(&ptr_new, new_filename, "wb");

	if (err != 0)
		return  -1;

	if (err1 != 0)
	{
		fclose(ptr_old);
		return  -1;
	}

	while (1)
	{
		a = fgetc(ptr_old);

		if (!feof(ptr_old))
			fputc(a, ptr_new);
		else
			break;
	}

	fclose(ptr_new);
	fclose(ptr_old);
	return  0;
}


void ModelLoader::SetFBXAnimation(char* anim_str)
{
	FbxTakeInfo* lCurrentTakeInfo = pFbxScene->GetTakeInfo(anim_str);
	if (lCurrentTakeInfo)
	{
		mStart = lCurrentTakeInfo->mLocalTimeSpan.GetStart();
		mStop = lCurrentTakeInfo->mLocalTimeSpan.GetStop();
		anim_length = mStop.Get();
		bAnimated = true;

		FbxAnimStack * lCurrentAnimationStack = pFbxScene->FindMember<FbxAnimStack>(anim_str);
		pFbxScene->SetCurrentAnimationStack(lCurrentAnimationStack);
		//lCurrentTakeInfo->
	}
	/*
	lCurrentTakeInfo = pFbxScene->GetTakeInfo("Cube|CubeAction");
	if (lCurrentTakeInfo)
	{
		mStart = lCurrentTakeInfo->mLocalTimeSpan.GetStart();
		mStop = lCurrentTakeInfo->mLocalTimeSpan.GetStop();
		anim_length = mStop.Get();
		bAnimated = true;
		FbxAnimStack * lCurrentAnimationStack = pFbxScene->FindMember<FbxAnimStack>("Cube|CubeAction");
		pFbxScene->SetCurrentAnimationStack(lCurrentAnimationStack);
		//lCurrentTakeInfo->
	}
	*/
}


bool ModelLoader::LoadFBX(char *filename, float scale, int model_number)
{
	int vertexCount, textureCount, normalCount, faceCount;
	char garbage;
	bool result;
	char local_file_folder[140];
	char content_file_folder[140];

	m_verticesCount = 0;
	m_indicesCount = 0;

	if (g_pFbxSdkManager == nullptr)
	{
		g_pFbxSdkManager = FbxManager::Create();

		FbxIOSettings* pIOsettings = FbxIOSettings::Create(g_pFbxSdkManager, IOSROOT);
		g_pFbxSdkManager->SetIOSettings(pIOsettings);
	}

	FbxImporter* pImporter = FbxImporter::Create(g_pFbxSdkManager, "");
	pFbxScene = FbxScene::Create(g_pFbxSdkManager, "");
	
	//FbxStream fbx_stream;

	//fbx_stream.

	//FbxFile FBXFile;

	//FBXFile.Open("Assets/Models/cone.fbx");
	int lFileFormat = -1;
	//FbxStream* FBXStream = FbxStream::;

	/*x
	if (!g_pFbxSdkManager->GetIOPluginRegistry()->DetectReaderFileFormat("Assets/Models/cone.fbx", lFileFormat))
	{
		// Unrecognizable file format. Try to fall back to FbxImporter::eFBX_BINARY
		lFileFormat = g_pFbxSdkManager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");;
	}
	*/
	/*
	Windows::Storage::StorageFolder^ folder = Windows::Storage::ApplicationData::Current->LocalFolder;

	wcstombs(local_file_folder, folder->Path->Data(), sizeof(local_file_folder));
	
	sprintf(local_file_folder, "%s\\%s.fbx", local_file_folder, filename);


	//Windows::Storage::StorageFile

	copy_file(content_file_folder, local_file_folder);
	*/

	sprintf(content_file_folder, "Assets\\Models\\%s.fbx", filename);

	bool bSuccess = pImporter->Initialize(content_file_folder, lFileFormat, g_pFbxSdkManager->GetIOSettings());// ("Assets/Models/cone.fbx", -1, g_pFbxSdkManager->GetIOSettings());
	if (!bSuccess)
	{
		const char* error_string = pImporter->GetStatus().GetErrorString();

		return E_FAIL;
	}

	bSuccess = pImporter->Import(pFbxScene);
	if (!bSuccess) return E_FAIL;

	int anims = pImporter->GetAnimStackCount();



	for (int i = 0; i < anims; i++)
	{
		FbxTakeInfo* lTakeInfo = pImporter->GetTakeInfo(i);

	}

	pImporter->Destroy();

	bAnimated = false;

	FbxNode* pFbxRootNode = pFbxScene->GetRootNode();
	//pFbxScene->SetCurrentAnimationStack();

	FbxAnimStack* stack =  pFbxScene->GetCurrentAnimationStack();
	//stack->
	//pFbxScene->GetCharacterPose()
	int current_mesh = -1;
	if (pFbxRootNode)
	{
		for (int i = 0; i < pFbxRootNode->GetChildCount(); i++)
		{
			pFbxModelNode = pFbxRootNode->GetChild(i);
			
			current_mesh++;

			if (pFbxModelNode->GetNodeAttribute() == NULL)
				continue;

			FbxNodeAttribute::EType AttributeType = pFbxModelNode->GetNodeAttribute()->GetAttributeType();

			if (AttributeType != FbxNodeAttribute::eMesh)
			{
				continue;
			}

			if (current_mesh != model_number)
			{
				continue;
			}

			FbxMesh* pMesh = (FbxMesh*)pFbxModelNode->GetNodeAttribute();

			FbxVector4* pVertices = pMesh->GetControlPoints();
			
			//pMesh->GetPolygonIn
			int p_count = pMesh->GetPolygonCount();
			unique_ptr<VertexData> vd(new VertexData());

			for (int j = 0; j < pMesh->GetPolygonCount(); j++)
			{
				int iNumVertices = pMesh->GetPolygonSize(j);
				assert(iNumVertices == 3);

				for (int k = iNumVertices-1; k >-1 ; k--)
				{
					int iControlPointIndex = pMesh->GetPolygonVertex(j, k);


					FbxVector4 vec_norm;
					FbxVector2 vec_uv;

					pMesh->GetPolygonVertexNormal(j, k, vec_norm);
					
					bool lUnmappedUV;

					FbxStringList lUVSetNameList;
					pMesh->GetUVSetNames(lUVSetNameList);


					//get lUVSetIndex-th uv set
					const char* lUVSetName = lUVSetNameList.GetStringAt(0);
					const FbxGeometryElementUV* lUVElement = pMesh->GetElementUV(lUVSetName);
					
					pMesh->GetPolygonVertexUV(j, k, lUVSetName, vec_uv, lUnmappedUV);


					vd->SetPosition((float)pVertices[iControlPointIndex].mData[0], (float)pVertices[iControlPointIndex].mData[1], (float)pVertices[iControlPointIndex].mData[2]);
					vd->SetTex(vec_uv.mData[0], vec_uv.mData[1]);
					vd->SetNormal(vec_norm.mData[0], vec_norm.mData[1], vec_norm.mData[2]);
					vd->SetColor(DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
					//vd->SetBlend(0.0f);
					//vd->SetSpecular(0.0f);

					m_vertices.push_back(*vd);

					m_indices.push_back(m_indicesCount);

					m_verticesCount++;
					m_indicesCount++;

					//MyVertex vertex;
					//vertex.pos[0] = (float)pVertices[iControlPointIndex].mData[0];
					//vertex.pos[1] = (float)pVertices[iControlPointIndex].mData[1];
					//vertex.pos[2] = (float)pVertices[iControlPointIndex].mData[2];
					//pOutVertexVector->push_back(vertex);
				}
			}

		}

	}



	m_phy_vertices = new VertexType[m_verticesCount];
	no_phy_verticies = m_verticesCount;

	for (int i = 0; i<m_verticesCount; i++)
	{
		m_phy_vertices[i].x = m_vertices[i].pos.x;
		m_phy_vertices[i].y = m_vertices[i].pos.y;
		m_phy_vertices[i].z = m_vertices[i].pos.z;
	}

	if (bUpdateable == true)
	{
		CreateUpdatebleBuffers();
	}
	else
	{
		SetIndexBuffer();
		SetVertexBuffer();
	}
	bLoadingComplete = true;
}








void ModelLoader::SetModelTextureDensity(float _d)
{
	for (int i = 0; i < m_verticesCount; i++)
	{
		m_vertices[i].tex.x *= _d;
		m_vertices[i].tex.y *= _d;
	}

	if (bUpdateable == true)
	{
		UpdateVerticies();
	}
	else
	{
		SetIndexBuffer();
		SetVertexBuffer();
	}

}



void ModelLoader::Optimize()
{
	int i, j;

	std::vector<VertexData> opt_vertices;
	std::vector<unsigned short> opt_ind;

	opt_vertices.clear();
	opt_ind.clear();
	//int m_verticesCount;
	//int m_indicesCount;

	for (i = 0; i < m_verticesCount; i++)
	{
		bool bFound = false;
		for (j = 0; j < opt_vertices.size(); j++)
		{
			if (opt_vertices[j].pos.x == m_vertices[i].pos.x &&
				opt_vertices[j].pos.y == m_vertices[i].pos.y &&
				opt_vertices[j].pos.z == m_vertices[i].pos.z &&
				opt_vertices[j].tex.x == m_vertices[i].tex.x &&
				opt_vertices[j].tex.y == m_vertices[i].tex.y)
			{
				bFound = true;
				break;
			}
		}
		if (bFound == false)
		{
			opt_vertices.push_back(m_vertices[i]);
		}
	}

	for (i = 0; i < m_indicesCount; i++)
	{
		bool bFound = false;
		for (j = 0; j < opt_vertices.size(); j++)
		{
			if (opt_vertices[j].pos.x == m_vertices[m_indices[i]].pos.x &&
				opt_vertices[j].pos.y == m_vertices[m_indices[i]].pos.y &&
				opt_vertices[j].pos.z == m_vertices[m_indices[i]].pos.z &&
				opt_vertices[j].tex.x == m_vertices[m_indices[i]].tex.x &&
				opt_vertices[j].tex.y == m_vertices[m_indices[i]].tex.y)
			{
				bFound = true;
				opt_ind.push_back(j);
				break;
			}
		}
	}

	m_indices = opt_ind;
	m_vertices = opt_vertices;

	m_verticesCount = m_vertices.size();
	m_indicesCount = m_indices.size();

	opt_vertices.clear();
	opt_ind.clear();

}

void ModelLoader::RotateY(float angle)
{
	for (int i = 0; i < m_verticesCount; i += 1)
	{
		m_vertices[i].pos.x = (cos(angle)*m_vertices[i].pos.x) + (sin(angle)*m_vertices[i].pos.z);

		m_vertices[i].pos.z = (-sin(angle)*m_vertices[i].pos.x) + (cos(angle)*m_vertices[i].pos.z);

		m_phy_vertices[i].x = (cos(angle)*m_phy_vertices[i].x) + (sin(angle)*m_phy_vertices[i].z);

		m_phy_vertices[i].z = (-sin(angle)*m_phy_vertices[i].x) + (cos(angle)*m_phy_vertices[i].z);

	}

	if (bUpdateable == true)
	{
		UpdateVerticies();
	}
	else
	{
		SetIndexBuffer();
		SetVertexBuffer();
	}
	//newX = centerX + (cosX * (point2X - centerX) + sinX * (point2Y - centerY))
	//newY = centerY + (-sinX * (point2X - centerX) + cosX * (point2Y - centerY))

}

void ModelLoader::SetColourWeights(DirectX::XMFLOAT4* _cols)
{
	int i;
	//return;
	for (i = 0; i < m_verticesCount; i++)
	{
		//m_vertices[i].col = _cols[i];
	}

	if (bUpdateable == true)
	{
		UpdateVerticies();
	}
	else
	{
		SetIndexBuffer();
		SetVertexBuffer();
	}
}

void ModelLoader::SaveBinary()
{



}


ModelLoader::ModelLoader(std::shared_ptr<DX::DeviceResources> pm_deviceResources, bool _bUpdateable, int _updateLevel)
{
	bLoadingComplete = false;
	//bCleared = false;

	// get local file folder
	Windows::Storage::StorageFolder^ folder = Windows::Storage::ApplicationData::Current->LocalFolder;
	// use folder->Path->Data() as the path base
	wcstombs(local_file_folder, folder->Path->Data(), sizeof(local_file_folder));

	bAnimated = false;

	bUpdateable = _bUpdateable;

	iUpdateLevel = _updateLevel;

	m_deviceResources = pm_deviceResources;

	XMFLOAT4X4 matr;
	// create a flat model matrix
	auto rotationMatrix = XMMatrixRotationRollPitchYaw(0.0f,0.0f,0.0f);
	auto translationMatrix = XMMatrixTranslation(0.0f,0.0f,0.0f);

	XMMATRIX scaleMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX scaleRotationMatrix = XMMatrixMultiply(scaleMatrix, rotationMatrix);
	XMMATRIX modelMatrix = XMMatrixMultiplyTranspose(scaleRotationMatrix, translationMatrix);

	XMStoreFloat4x4(&m_ObMatrix, XMMatrixTranspose(rotationMatrix * translationMatrix));

	// set default object matrix
	


}

void ModelLoader::SetPosition(float x, float y, float z, float yaw, float pitch, float roll,float scale)
{
	auto rotationMatrix = XMMatrixRotationRollPitchYaw(yaw, pitch, roll);
	auto translationMatrix = XMMatrixTranslation(x, y, z);



	XMMATRIX scaleMatrix = XMMatrixScaling(scale, scale, scale);
	XMMATRIX scaleRotationMatrix = XMMatrixMultiply(scaleMatrix, rotationMatrix);
	XMMATRIX modelMatrix = XMMatrixMultiplyTranspose(scaleRotationMatrix, translationMatrix);

	XMStoreFloat4x4(&m_ObMatrix, XMMatrixTranspose(rotationMatrix * translationMatrix));

}

void ModelLoader::SetPosition(DirectX::XMFLOAT4X4* p_posmatrix)
{
	m_ObMatrix = *p_posmatrix;

}

void ModelLoader::SetRenderBuffers()
{
	if (bLoadingComplete == false)
		return;
	UINT stride = sizeof(VertexData);
	UINT offset = 0;

	m_deviceResources->GetD3DDeviceContext()->IASetVertexBuffers(0, 1, GetVertexBuffer().GetAddressOf(), &stride, &offset);
	m_deviceResources->GetD3DDeviceContext()->IASetIndexBuffer(GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);
}

void ModelLoader::DrawIndexed()
{
	if (bLoadingComplete == false)
		return;
	m_deviceResources->GetD3DDeviceContext()->DrawIndexed(m_indicesCount, 0, 0);

}


bool ModelLoader::CreateUpdatebleBuffers()
{
	unsigned long* indices;
	int i;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	DirectX::XMFLOAT3 pos[3]; // creating central points of triangles for sorting
	DirectX::XMFLOAT3 cent;

	if (iUpdateLevel == 0 || iUpdateLevel == 2)
	{
		// Set up the description of the dynamic vertex buffer.
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.ByteWidth = sizeof(VertexData) * m_verticesCount;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the vertex data.
		vertexData.pSysMem = m_vertices.data();
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		// Now finally create the vertex buffer.
		result = m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
		if (FAILED(result))
		{
			return false;
		}
	}
	else
	{
		SetVertexBuffer();
	}


	if (iUpdateLevel == 1 || iUpdateLevel == 2)
	{
		// Set up the description of the static index buffer.
		indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indicesCount;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the index data.
		indexData.pSysMem = m_indices.data();
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		// Create the index buffer.
		result = m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
		if (FAILED(result))
		{
			return false;
		}

		// create middle point array
		for (i = 0; i < m_indicesCount; i += 3)
		{
			pos[0] = m_vertices[m_indices[i]].pos;
			pos[1] = m_vertices[m_indices[i + 1]].pos;
			pos[2] = m_vertices[m_indices[i + 2]].pos;

			cent.x = pos[0].x + pos[1].x + pos[2].x;
			cent.y = pos[0].y + pos[1].y + pos[2].y;
			cent.z = pos[0].z + pos[1].z + pos[2].z;
			cent.x = cent.x / 3;
			cent.y = cent.y / 3;
			cent.z = cent.z / 3;

			m_central.push_back(btVector3(cent.x, cent.y, cent.z));
		}
		p_index = new part_index[m_central.size()];
		m_indices_back = m_indices;
	}
	else
	{
		SetIndexBuffer();
	}

	return true;
}

void ModelLoader::Sort(float x, float y, float z) // sort by camera position
{



	if (iUpdateLevel == 1 || iUpdateLevel == 2)
	{
		int i;

		btVector3 cam_pos = btVector3(x, y, z);
		//m_indices = m_indices_back;

		for (i = 0; i < m_central.size(); i++)
		{
			p_index[i].part_no = i;
			p_index[i].dist = cam_pos.distance(m_central[i]);
		}

		qsort(p_index, m_central.size(), sizeof(part_index), ModelSortCB);

		for (i = 0; i < m_central.size(); i++)
		{
			m_indices[(i * 3)] = m_indices_back[(i * 3)];
			m_indices[(i * 3) + 1] = m_indices_back[(i * 3) + 1];
			m_indices[(i * 3) + 2] = m_indices_back[(i * 3) + 2];
		}

	}
}

bool ModelLoader::UpdateVerticies()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	D3D11_MAPPED_SUBRESOURCE mappedResource2;

	HRESULT result;
	VertexData* verticesPtr;
	unsigned long* indicesPtr;


	if (iUpdateLevel == 0 || iUpdateLevel == 2)
	{
		// Lock the vertex buffer.
		result = m_deviceResources->GetD3DDeviceContext()->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
		{
			return false;
		}

		// Get a pointer to the data in the vertex buffer.
		verticesPtr = (VertexData*)mappedResource.pData;

		// Copy the data into the vertex buffer.
		memcpy(verticesPtr, (void*)m_vertices.data(), (sizeof(VertexData) * (m_verticesCount)));

		// Unlock the vertex buffer.
		m_deviceResources->GetD3DDeviceContext()->Unmap(m_vertexBuffer.Get(), 0);
	}
	else
	{
		SetVertexBuffer();
	}



	if (iUpdateLevel == 1 || iUpdateLevel == 2)
	{

		// Lock the index buffer.
		result = m_deviceResources->GetD3DDeviceContext()->Map(m_indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource2);
		if (FAILED(result))
		{
			return false;
		}

		// Get a pointer to the data in the vertex buffer.
		indicesPtr = (unsigned long*)mappedResource2.pData;

		// Copy the data into the vertex buffer.
		memcpy(indicesPtr, (void*)m_indices.data(), (sizeof(unsigned long) * (m_indicesCount)));

		// Unlock the vertex buffer.
		m_deviceResources->GetD3DDeviceContext()->Unmap(m_indexBuffer.Get(), 0);

	}
	else
	{
		SetIndexBuffer();
	}


}

void ModelLoader::SetVertexBuffer()
{
	D3D11_SUBRESOURCE_DATA bufferData = { m_vertices.data(), 0, 0 };
	UINT bytes = sizeof(VertexData) * m_verticesCount;
	CD3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(bytes, D3D11_BIND_VERTEX_BUFFER);
	ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bufferDesc, &bufferData, &m_vertexBuffer));
}

void ModelLoader::SetIndexBuffer()
{
	D3D11_SUBRESOURCE_DATA bufferData = { m_indices.data(), 0, 0 };
	UINT bytes = sizeof(unsigned short) * m_indicesCount;
	CD3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(bytes, D3D11_BIND_INDEX_BUFFER);
	ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bufferDesc, &bufferData, &m_indexBuffer));
}


ModelLoader::~ModelLoader()
{
	//m_vertices->Release();


	//delete[] m_phy_vertices;
	//m_vertexBuffer->Release();
	//m_indexBuffer->Release();

}

void ModelLoader::Clearmemory()
{
	/*
	if (bCleared == false)
	{
	bCleared = true;
	delete[] m_phy_vertices;
	m_vertexBuffer->Release();
	m_indexBuffer->Release();
	}
	*/
}

void ModelLoader::TransOrigin(float x, float y, float z)
{
	for (int i = 0; i < m_verticesCount; i += 1)
	{
		m_phy_vertices[i].x += x;
		m_phy_vertices[i].y += y;
		m_phy_vertices[i].z += z;

		m_vertices[i].pos.x += x;
		m_vertices[i].pos.y += y;
		m_vertices[i].pos.z += z;
	}

	if (bUpdateable == true)
	{
		UpdateVerticies();
	}
	else
	{
		SetIndexBuffer();
		SetVertexBuffer();
	}
}

void ModelLoader::FlipNorms()
{
	for (int i = 0; i < m_verticesCount; i += 1)
	{
		//m_phy_vertices[i].y = -m_phy_vertices[i].y;

		//m_vertices[i].position.y = -m_vertices[i].position.y;
		m_vertices[i].norm.x = -m_vertices[i].norm.x;
		m_vertices[i].norm.y = -m_vertices[i].norm.y;
		m_vertices[i].norm.z = -m_vertices[i].norm.z;

	}

	if (bUpdateable == true)
	{
		UpdateVerticies();
	}
	else
	{
		SetIndexBuffer();
		SetVertexBuffer();
	}
}

void ModelLoader::UpsideDown()
{
	for (int i = 0; i < m_verticesCount; i += 1)
	{
		m_phy_vertices[i].y = -m_phy_vertices[i].y;

		m_vertices[i].pos.y = -m_vertices[i].pos.y;
		m_vertices[i].norm.y = -m_vertices[i].norm.y;

	}

	if (bUpdateable == true)
	{
		UpdateVerticies();
	}
	else
	{
		SetIndexBuffer();
		SetVertexBuffer();
	}
}

void ModelLoader::SetScale(float x, float y, float z)
{
	for (int i = 0; i<m_verticesCount; i++)
	{

		m_phy_vertices[i].x = m_phy_vertices[i].x * x;
		m_phy_vertices[i].y = m_phy_vertices[i].y * y;
		m_phy_vertices[i].z = m_phy_vertices[i].z * z;


		m_vertices[i].pos.x = m_vertices[i].pos.x * x;
		m_vertices[i].pos.y = m_vertices[i].pos.y * y;
		m_vertices[i].pos.z = m_vertices[i].pos.z * z;

		//m_phy_vertices[i].y = -m_phy_vertices[i].y;
		//m_vertices[i].position.y = -m_vertices[i].position.y;
	}

	if (bUpdateable == true)
	{
		UpdateVerticies();
	}
	else
	{
		SetIndexBuffer();
		SetVertexBuffer();
	}
}

void ModelLoader::InsideOut()
{
	FlipVertDir();
	FlipNorms();
}

void ModelLoader::FlipVertDir()
{
	float x, y, z;
	unsigned short ind;

	//exit(1);
	for (int i = 0; i < m_indicesCount; i += 3)
	{
		ind = m_indices[i];
		m_indices[i] = m_indices[i + 1];
		m_indices[i + 1] = ind;
	}

	for (int i = 0; i<no_phy_verticies; i += 3)
	{
		//exit(0);
		x = m_phy_vertices[i].x;
		y = m_phy_vertices[i].y;
		z = m_phy_vertices[i].z;

		m_phy_vertices[i].x = m_phy_vertices[i + 1].x;
		m_phy_vertices[i].y = m_phy_vertices[i + 1].y;
		m_phy_vertices[i].z = m_phy_vertices[i + 1].z;

		m_phy_vertices[i + 1].x = x;
		m_phy_vertices[i + 1].y = y;
		m_phy_vertices[i + 1].z = z;

		/*
		x = m_vertices[i].position.x;
		y = m_vertices[i].position.y;
		z = m_vertices[i].position.z;

		m_vertices[i].position.x = m_vertices[i + 1].position.x;
		m_vertices[i].position.y = m_vertices[i + 1].position.y;
		m_vertices[i].position.z = m_vertices[i + 1].position.z;

		m_vertices[i + 1].position.x = x;
		m_vertices[i + 1].position.y = y;
		m_vertices[i + 1].position.z = z;

		m_vertices[i].normal.x = -m_vertices[i].normal.x;
		m_vertices[i].normal.y = -m_vertices[i].normal.y;
		m_vertices[i].normal.z = -m_vertices[i].normal.z;

		m_vertices[i + 1].normal.x = -m_vertices[i + 1].normal.x;
		m_vertices[i + 1].normal.y = -m_vertices[i + 1].normal.y;
		m_vertices[i + 1].normal.z = -m_vertices[i + 1].normal.z;

		m_vertices[i + 2].normal.x = -m_vertices[i + 2].normal.x;
		m_vertices[i + 2].normal.y = -m_vertices[i + 2].normal.y;
		m_vertices[i + 2].normal.z = -m_vertices[i + 2].normal.z;


		x = m_vertices[i].tex.x;
		y = m_vertices[i].tex.y;


		m_vertices[i].tex.x = m_vertices[i + 1].tex.x;
		m_vertices[i].tex.y = m_vertices[i + 1].tex.y;

		m_vertices[i + 1].tex.x = x;
		m_vertices[i + 1].tex.y = y;

		*/
		//m_phy_vertices[i].y = -m_phy_vertices[i].y;
		//m_vertices[i].position.y = -m_vertices[i].position.y;
	}


	if (bUpdateable == true)
	{
		UpdateVerticies();
	}
	else
	{
		SetIndexBuffer();
		SetVertexBuffer();
	}

}

// optimize 1 for optimize
bool ModelLoader::Load(char *filename, float scale, int optimize)
{
	int vertexCount, textureCount, normalCount, faceCount;
	char garbage;
	bool result;



	GetModelFilename(filename);
	result = ReadFileCounts(filename, vertexCount, textureCount, normalCount, faceCount);
	if (!result)
	{
		return false;
		// exit(1);
	}
	result = LoadDataStructures(filename, vertexCount, textureCount, normalCount, faceCount, scale);
	if (!result)
	{
		return false;
		// exit(1);
	}

	if (optimize == 1)
		Optimize();

	if (bUpdateable == true)
	{
		CreateUpdatebleBuffers();
	}
	else
	{
		SetIndexBuffer();
		SetVertexBuffer();
	}
	bLoadingComplete = true;
}



void ModelLoader::GetModelFilename(char* filename)
{
	bool done;
	ifstream fin;

	/*
	auto appInstallDirectory = Windows::ApplicationModel::Package::Current->InstalledLocation->Path;

	//auto Windows::ApplicationModel::Package::Current->InstalledLocation;

	std::wstring wpath = Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data();
	wpath += L"\\dice.obj";

	FILE* f = nullptr;
	errno_t e = _wfopen_s(&f, wpath.c_str(), L"rb");
	*/


	done = false;
	while (!done)
	{
		fin.open(filename);

		if (fin.good())
		{
			done = true;
		}
		else
		{
			fin.clear();
		}
	}

	return;
}

bool ModelLoader::ReadFileCounts(char* filename, int& vertexCount, int& textureCount, int& normalCount, int& faceCount)
{
	ifstream fin;
	char input;


	// Initialize the counts.
	vertexCount = 0;
	textureCount = 0;
	normalCount = 0;
	faceCount = 0;

	// Open the file.
	fin.open(filename);

	// Check if it was successful in opening the file.
	if (fin.fail() == true)
	{
		return false;
	}

	// Read from the file and continue to read until the end of the file is reached.
	fin.get(input);
	while (!fin.eof())
	{
		// If the line starts with 'v' then count either the vertex, the texture coordinates, or the normal vector.
		if (input == 'v')
		{
			fin.get(input);
			if (input == ' ') { vertexCount++; }
			if (input == 't') { textureCount++; }
			if (input == 'n') { normalCount++; }
		}

		// If the line starts with 'f' then increment the face count.
		if (input == 'f')
		{
			fin.get(input);
			if (input == ' ') { faceCount++; }
		}

		// Otherwise read in the remainder of the line.
		while (input != '\n')
		{
			fin.get(input);
		}

		// Start reading the beginning of the next line.
		fin.get(input);
	}

	// Close the file.
	fin.close();

	return true;
}

//bool ModelLoader::SetColour()

void ModelLoader::SetModelSpecular(float _s)
{
	int num = m_vertices.size();

	for (int i = 0; i < num; i++)
	{
		//m_vertices[i].SetSpecular(_s);
	}

	if (bUpdateable == true)
	{
		UpdateVerticies();
	}
	else
	{
		SetIndexBuffer();
		SetVertexBuffer();
	}
}

void ModelLoader::SetModelColourWeight(float _r, float _g, float _b, float _a)
{
	int num = m_vertices.size();

	for (int i = 0; i < num; i++)
	{
		//m_vertices[i].col = DirectX::XMFLOAT4(_r, _g, _b, _a);
	}

	if (bUpdateable == true)
	{
		UpdateVerticies();
	}
	else
	{
		SetIndexBuffer();
		SetVertexBuffer();
	}
}

void ModelLoader::SetModelNormals(float _x, float _y, float _z)
{
	int num = m_vertices.size();

	for (int i = 0; i < num; i++)
	{
		m_vertices[i].norm = DirectX::XMFLOAT3(_x, _y, _z);
	}

	if (bUpdateable == true)
	{
		UpdateVerticies();
	}
	else
	{
		SetIndexBuffer();
		SetVertexBuffer();
	}
}


DirectX::XMFLOAT4X4* ModelLoader::GetAnimationObjectMatrix(float _time)
{
	FbxAMatrix f_matrix;
	FbxTime f_time;
	f_time.Set(_time);

	f_matrix = pFbxModelNode->EvaluateGlobalTransform(f_time);
	//pFbxModelNode->GetGeometricTranslation()
	for (int r = 0; r < 4; r++)
		for (int c = 0; c < 4; c++)
		{
			m_AnimMatrix.m[c][r] = (float)f_matrix.mData[r][c];
		}

	XMMATRIX m_1 = XMLoadFloat4x4(&m_ObMatrix);
	XMMATRIX m_2 = XMLoadFloat4x4(&m_AnimMatrix); // 
	XMMATRIX m_3 = XMMatrixMultiply(m_1, m_2);
	
	XMStoreFloat4x4(&m_AnimObMatrix, m_3);

	return &m_AnimObMatrix;
}

DirectX::XMFLOAT4X4* ModelLoader::GetObjectMatrix()
{
	return &m_ObMatrix;
}


DirectX::XMFLOAT4X4* ModelLoader::GetAnimationMatrix(float _time)
{
	FbxAMatrix f_matrix;
	FbxTime f_time;
	f_time.Set(_time);

	if (bAnimated == false)
	{
		return &m_ObMatrix;
	}

	f_matrix = pFbxModelNode->EvaluateGlobalTransform(f_time);
	//pFbxModelNode->GetGeometricTranslation()
	for (int r = 0; r < 4; r++)
		for (int c = 0; c < 4; c++)
		{
			m_AnimMatrix.m[c][r] = (float)f_matrix.mData[r][c];
		}

	return &m_AnimMatrix;
}

bool ModelLoader::LoadDataStructures(char* filename, int vertexCount, int textureCount, int normalCount, int faceCount, float m_scale)
{
	VertexType *vertices, *texcoords, *normals;
	FaceType *faces;
	FaceType *new_faces;
	ifstream fin;
	int vertexIndex, texcoordIndex, normalIndex, faceIndex, vIndex, tIndex, nIndex;
	char input, input2;
	ofstream fout;

	object_scale = m_scale;

	float high_x = 0.0f, high_y = 0.0f, high_z = 0.0f;

	height_highest = 0.0f;
	height_lowest = 0.0f;
	total_height = 0.0f;

	vertices = new VertexType[vertexCount];
	if (!vertices)
	{
		return false;
	}

	texcoords = new VertexType[textureCount];
	if (!texcoords)
	{
		return false;
	}

	normals = new VertexType[normalCount];
	if (!normals)
	{
		return false;
	}

	faces = new FaceType[faceCount];
	if (!faces)
	{
		return false;
	}

	new_faces = new FaceType[faceCount];
	if (!new_faces)
	{
		return false;
	}

	vertexIndex = 0;
	texcoordIndex = 0;
	normalIndex = 0;
	faceIndex = 0;

	// Open the file.
	fin.open(filename);

	// Check if it was successful in opening the file.
	if (fin.fail() == true)
	{
		return false;
	}

	fin.get(input);
	while (!fin.eof())
	{
		if (input == 'v')
		{
			fin.get(input);

			if (input == ' ')
			{
				fin >> vertices[vertexIndex].x >> vertices[vertexIndex].y >> vertices[vertexIndex].z;
				vertices[vertexIndex].z = vertices[vertexIndex].z * -1.0f;

				vertices[vertexIndex].x = vertices[vertexIndex].x*m_scale;
				vertices[vertexIndex].y = vertices[vertexIndex].y*m_scale;
				vertices[vertexIndex].z = vertices[vertexIndex].z*m_scale;

				if (vertices[vertexIndex].y>0.0f && vertices[vertexIndex].y>height_highest)
				{
					height_highest = vertices[vertexIndex].y;
				}

				if (vertices[vertexIndex].y<0.0f && vertices[vertexIndex].y<height_lowest)
				{
					height_lowest = vertices[vertexIndex].y;
				}

				if (abs(vertices[vertexIndex].x) > high_x)
					high_x = abs(vertices[vertexIndex].x);
				if (abs(vertices[vertexIndex].y) > high_y)
					high_y = abs(vertices[vertexIndex].y);
				if (abs(vertices[vertexIndex].z) > high_z)
					high_z = abs(vertices[vertexIndex].z);

				vertexIndex++;
			}

			if (input == 't')
			{
				fin >> texcoords[texcoordIndex].x >> texcoords[texcoordIndex].y;
				texcoords[texcoordIndex].y = 1.0f - texcoords[texcoordIndex].y;
				texcoordIndex++;
			}

			if (input == 'n')
			{
				fin >> normals[normalIndex].x >> normals[normalIndex].y >> normals[normalIndex].z;
				/*normals[normalIndex].x=normals[normalIndex].x;
				normals[normalIndex].y=normals[normalIndex].y;
				normals[normalIndex].z=normals[normalIndex].z;*/
				normals[normalIndex].z = normals[normalIndex].z * -1.0f;

				if (true)//( m_scale<0.0f )
				{
					normals[normalIndex].x = -normals[normalIndex].x;
					normals[normalIndex].y = -normals[normalIndex].y;
					normals[normalIndex].z = -normals[normalIndex].z;
				}

				normalIndex++;
			}
		}

		if (input == 'f')
		{
			fin.get(input);
			if (input == ' ')
			{
				// Read the face data in backwards to convert it to a left hand system from right hand system.
				fin >> faces[faceIndex].vIndex3 >> input2 >> faces[faceIndex].tIndex3 >> input2 >> faces[faceIndex].nIndex3
					>> faces[faceIndex].vIndex2 >> input2 >> faces[faceIndex].tIndex2 >> input2 >> faces[faceIndex].nIndex2
					>> faces[faceIndex].vIndex1 >> input2 >> faces[faceIndex].tIndex1 >> input2 >> faces[faceIndex].nIndex1;
				faceIndex++;
			}
		}

		while (input != '\n')
		{
			fin.get(input);
		}

		fin.get(input);
	}

	fin.close();

	total_height = abs(height_highest) + abs(height_lowest);

	btVector3 vecdist = btVector3(high_x, high_y, high_z);
	furthest_point = vecdist.length();

	int current_face = 0;

	for (int i = 0; i<faceIndex; i++)
	{
		unique_ptr<VertexData> vd(new VertexData());

		vIndex = faces[i].vIndex1 - 1;
		tIndex = faces[i].tIndex1 - 1;
		nIndex = faces[i].nIndex1 - 1;

		new_faces[i].vIndex1 = current_face++;

		vd->SetPosition(vertices[vIndex].x, vertices[vIndex].y, vertices[vIndex].z);
		vd->SetTex(texcoords[tIndex].x, texcoords[tIndex].y);
		vd->SetNormal(normals[nIndex].x, normals[nIndex].y, normals[nIndex].z);
		vd->SetColor(DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
		//vd->SetBlend(0.0f);
		//vd->SetSpecular(0.0f);
		m_vertices.push_back(*vd);

		vIndex = faces[i].vIndex2 - 1;
		tIndex = faces[i].tIndex2 - 1;
		nIndex = faces[i].nIndex2 - 1;

		new_faces[i].vIndex2 = current_face++;

		vd->SetPosition(vertices[vIndex].x, vertices[vIndex].y, vertices[vIndex].z);
		vd->SetTex(texcoords[tIndex].x, texcoords[tIndex].y);
		vd->SetNormal(normals[nIndex].x, normals[nIndex].y, normals[nIndex].z);
		vd->SetColor(DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
		//vd->SetBlend(0.0f);
		//vd->SetSpecular(0.0f);
		m_vertices.push_back(*vd);

		vIndex = faces[i].vIndex3 - 1;
		tIndex = faces[i].tIndex3 - 1;
		nIndex = faces[i].nIndex3 - 1;

		new_faces[i].vIndex3 = current_face++;

		vd->SetPosition(vertices[vIndex].x, vertices[vIndex].y, vertices[vIndex].z);
		vd->SetTex(texcoords[tIndex].x, texcoords[tIndex].y);
		vd->SetNormal(normals[nIndex].x, normals[nIndex].y, normals[nIndex].z);
		vd->SetColor(DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
		//vd->SetBlend(0.0f);
		//vd->SetSpecular(0.0f);
		m_vertices.push_back(*vd);

	}


	for (int i = 0; i<faceCount; i++)
	{
		m_indices.push_back(new_faces[i].vIndex1);
		m_indices.push_back(new_faces[i].vIndex2);
		m_indices.push_back(new_faces[i].vIndex3);
		/*
		m_indices.push_back(new_faces[i].vIndex1);
		m_indices.push_back(new_faces[i].vIndex2);
		m_indices.push_back(new_faces[i].vIndex3);*/
	}

	m_verticesCount = m_vertices.size();
	m_indicesCount = m_indices.size();

	m_phy_vertices = new VertexType[m_verticesCount];
	no_phy_verticies = m_verticesCount;

	for (int i = 0; i<m_verticesCount; i++)
	{
		m_phy_vertices[i].x = m_vertices[i].pos.x;
		m_phy_vertices[i].y = m_vertices[i].pos.y;
		m_phy_vertices[i].z = m_vertices[i].pos.z;
	}

	/*
	if(vertices)
	{
	delete [] vertices;
	vertices = 0;
	}*/
	if (texcoords)
	{
		delete[] texcoords;
		texcoords = 0;
	}
	if (normals)
	{
		delete[] normals;
		normals = 0;
	}
	if (faces)
	{
		delete[] faces;
		faces = 0;
	}
	return true;
}