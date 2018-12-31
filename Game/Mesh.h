///////////////////////////////////////////////////////////////////////////////////////////
//
// Mesh is a class used to display meshes in 3d which are converted
// during build-time from fbx and dgsl files.
//
#pragma once
#include "pch.h"
#include "ppltasks_extra.h"


namespace Game
{

#define NUM_BONE_INFLUENCES 4
	struct SkinningVertex
	{
		UINT boneIndex[NUM_BONE_INFLUENCES];
		float boneWeight[NUM_BONE_INFLUENCES];
	};
	struct SkinningVertexInput
	{
		byte boneIndex[NUM_BONE_INFLUENCES];
		float boneWeight[NUM_BONE_INFLUENCES];
	};

	///////////////////////////////////////////////////////////////////////////////////////////
	//
	// Simple COM helper template functions for safe AddRef() and Release() of IUnknown objects.
	//
	template <class T> inline LONG SafeAddRef(T* pUnk) { ULONG lr = 0; if (pUnk != nullptr) { lr = pUnk->AddRef(); } return lr; }
	template <class T> inline LONG SafeRelease(T*& pUnk) { ULONG lr = 0; if (pUnk != nullptr) { lr = pUnk->Release(); pUnk = nullptr; } return lr; }
	//
	//
	///////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////
	//
	// Constant buffer structures
	//
	// These structs use padding and different data types in places to adhere
	// to the shader constant's alignment.
	//
	/*
	struct SubMesh
	{
		SubMesh() : MaterialIndex(0), IndexBufferIndex(0), VertexBufferIndex(0), StartIndex(0), PrimCount(0) {}

		UINT MaterialIndex;
		UINT IndexBufferIndex;
		UINT VertexBufferIndex;
		UINT StartIndex;
		UINT PrimCount;
	};
	*/

class Mesh
{
public:
	static const UINT MaxTextures = 8;  // 8 unique textures are supported.

	struct SubMesh
	{
		SubMesh() : MaterialIndex(0), IndexBufferIndex(0), VertexBufferIndex(0), StartIndex(0), PrimCount(0) {}

		MaterialBufferType m_material;

		UINT MaterialIndex;
		UINT IndexBufferIndex;
		UINT VertexBufferIndex;
		UINT StartIndex;
		UINT PrimCount;
	};

	struct Material
	{
		Material() { ZeroMemory(this, sizeof(Material)); }
		~Material() {}

		std::wstring Name;

		DirectX::XMFLOAT4X4 UVTransform;

		float Ambient[4];
		float Diffuse[4];
		float Specular[4];
		float Emissive[4];
		float SpecularPower;

		float normal_height;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Textures[MaxTextures];
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Normal;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerState;
	};

	struct MeshExtents
	{
		float CenterX, CenterY, CenterZ;
		float Radius;

		float MinX, MinY, MinZ;
		float MaxX, MaxY, MaxZ;
	};

	struct Triangle
	{
		DirectX::XMFLOAT3 points[3];
	};

	struct MeshPoint
	{
		DirectX::XMFLOAT3 point;
	};

	typedef std::vector<Triangle> TriangleCollection;
	typedef std::vector<MeshPoint> PointCollection;


	struct BoneInfo
	{
		std::wstring Name;
		INT ParentIndex;
		DirectX::XMFLOAT4X4 InvBindPos;
		DirectX::XMFLOAT4X4 BindPose;
		DirectX::XMFLOAT4X4 BoneLocalTransform;
	};

	struct Keyframe
	{
		Keyframe() : BoneIndex(0), Time(0.0f) {}

		UINT BoneIndex;
		float Time;
		DirectX::XMFLOAT4X4 Transform;
	};

	typedef std::vector<Keyframe> KeyframeArray;

	struct AnimClip
	{
		float StartTime;
		float EndTime;
		KeyframeArray Keyframes;
	};

	typedef std::map<const std::wstring, AnimClip> AnimationClipMap;

	int alpha_mode;

	// Access to mesh data.
	std::vector<SubMesh>& SubMeshes() { return m_submeshes; }
	std::vector<Material>& Materials() { return m_materials; }
	std::vector<ID3D11Buffer*>& VertexBuffers() { return m_vertexBuffers; }
	std::vector<ID3D11Buffer*>& SkinningVertexBuffers() { return m_skinningVertexBuffers; }
	std::vector<ID3D11Buffer*>& IndexBuffers() { return m_indexBuffers; }
	MeshExtents& Extents() { return m_meshExtents; }
	AnimationClipMap& AnimationClips() { return m_animationClips; }
	std::vector<BoneInfo>& BoneInfoCollection() { return m_boneInfo; }
	TriangleCollection& Triangles() { return m_triangles; }
	PointCollection& Points() { return m_points; }
	const wchar_t* Name() const { return m_name.c_str(); }

	void* Tag;

	// Destructor.
	~Mesh()
	{
		for (ID3D11Buffer *ib : m_indexBuffers)
		{
			SafeRelease(ib);
		}

		for (ID3D11Buffer *vb : m_vertexBuffers)
		{
			SafeRelease(vb);
		}

		for (ID3D11Buffer *svb : m_skinningVertexBuffers)
		{
			SafeRelease(svb);
		}

		m_submeshes.clear();
		m_materials.clear();
		m_indexBuffers.clear();
		m_vertexBuffers.clear();
		m_skinningVertexBuffers.clear();
	}
	
	void SetMaterialTexture(std::wstring _name, int tex_slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> p_Tex,int _alpha_mode)
	{
		alpha_mode = _alpha_mode;
		for (int i = 0; i < m_materials.size(); i++)
		{
			if(_name.length()==0)
			{
				m_materials[i].Textures[tex_slot] = p_Tex;
			}
			else
			{
				if(!wcscmp(_name.c_str(), m_materials[i].Name.c_str()))
				{
					m_materials[i].Textures[tex_slot] = p_Tex;
				}
			}


		}
	}

	void SetMaterialNormal(std::wstring _name, int tex_slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> p_Tex,float _intensity)
	{

		for (int i = 0; i < m_materials.size(); i++)
		{
			if (_name.length() == 0)
			{
				m_materials[i].Normal = p_Tex;
				m_materials[i].normal_height = _intensity;
			}
			else
			{
				if(!wcscmp(_name.c_str(), m_materials[i].Name.c_str()))
				{
					m_materials[i].Normal = p_Tex;
					m_materials[i].normal_height = _intensity;
				}
			}


		}
	}

	// Render the mesh to the current render target.
	void Render(AllResources& graphics)
	{
		ID3D11DeviceContext* deviceContext = graphics.m_deviceResources->GetD3DDeviceContext();

		//BOOL supportsShaderResources = graphics.GetDeviceFeatureLevel() >= D3D_FEATURE_LEVEL_10_0;

		//const DirectX::XMMATRIX& view = DirectX::XMLoadFloat4x4(&graphics.GetCamera().GetView());
		//const DirectX::XMMATRIX& projection = DirectX::XMLoadFloat4x4(&graphics.GetCamera().GetProjection()) * DirectX::XMLoadFloat4x4(&graphics.GetCamera().GetOrientationMatrix());

		// Compute the object matrices.
		//DirectX::XMMATRIX localToView = world * view;
		//DirectX::XMMATRIX localToProj = world * view * projection;

		// Initialize object constants and update the constant buffer.
		//ObjectConstants objConstants;
		//objConstants.LocalToWorld4x4 = DirectX::XMMatrixTranspose(world);
		//objConstants.LocalToProjected4x4 = DirectX::XMMatrixTranspose(localToProj);
		//objConstants.WorldToLocal4x4 = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, world));
		//objConstants.WorldToView4x4 = DirectX::XMMatrixTranspose(view);
		//objConstants.UvTransform4x4 = DirectX::XMMatrixIdentity();
		//objConstants.EyePosition = graphics.GetCamera().GetPosition();
		//graphics.UpdateObjectConstants(objConstants);

		// Assign the constant buffers to correct slots.
		//ID3D11Buffer* constantBuffer = graphics.GetLightConstants();
		//deviceContext->VSSetConstantBuffers(1, 1, &constantBuffer);
		//deviceContext->PSSetConstantBuffers(1, 1, &constantBuffer);

		//constantBuffer = graphics.GetMiscConstants();
		//deviceContext->VSSetConstantBuffers(3, 1, &constantBuffer);
		//deviceContext->PSSetConstantBuffers(3, 1, &constantBuffer);

		// Prepare to draw.
		//deviceContext->IASetInputLayout(graphics.GetVertexInputLayout());
		//deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Loop over each submesh.
		for (SubMesh& submesh : m_submeshes)
		{
			// Only draw submeshes that have valid materials.
			MaterialConstants materialConstants;

			if (submesh.IndexBufferIndex < m_indexBuffers.size() &&
				submesh.VertexBufferIndex < m_vertexBuffers.size())
			{
				UINT stride = sizeof(Vertex);
				UINT offset = 0;
				deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffers[submesh.VertexBufferIndex], &stride, &offset);
				deviceContext->IASetIndexBuffer(m_indexBuffers[submesh.IndexBufferIndex], DXGI_FORMAT_R16_UINT, 0);
			}

			if (submesh.MaterialIndex < m_materials.size())
			{
				Material& material = m_materials[submesh.MaterialIndex];


				MaterialBufferType mat_buffer;

				// Update the material constant buffer.
				memcpy(&mat_buffer.Ambient, material.Ambient, sizeof(material.Ambient));
				memcpy(&mat_buffer.Diffuse, material.Diffuse, sizeof(material.Diffuse));
				memcpy(&mat_buffer.Specular, material.Specular, sizeof(material.Specular));
				memcpy(&mat_buffer.Emissive, material.Emissive, sizeof(material.Emissive));
				mat_buffer.SpecularPower = material.SpecularPower;
				mat_buffer.normal_height = material.normal_height;

				graphics.UpdateMaterialBuffer(&mat_buffer);
				//graphics.UpdateMaterialConstants(materialConstants);






				/*
				
				// Assign the material buffer to correct slots.
				constantBuffer = graphics.GetMaterialConstants();
				deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
				deviceContext->PSSetConstantBuffers(0, 1, &constantBuffer);

				// Update the UV transform.
				memcpy(&objConstants.UvTransform4x4, &material.UVTransform, sizeof(objConstants.UvTransform4x4));
				graphics.UpdateObjectConstants(objConstants);

				constantBuffer = graphics.GetObjectConstants();
				deviceContext->VSSetConstantBuffers(2, 1, &constantBuffer);
				deviceContext->PSSetConstantBuffers(2, 1, &constantBuffer);

				// Assign the shaders, samplers and texture resources.
				ID3D11SamplerState* sampler = material.SamplerState.Get();
				*/
				//deviceContext->VSSetShader(material.VertexShader.Get(), nullptr, 0);
				//if (supportsShaderResources)
				//{
				//	deviceContext->VSSetSamplers(0, 1, &sampler);
				//}

				//deviceContext->PSSetShader(material.PixelShader.Get(), nullptr, 0);
				//deviceContext->PSSetSamplers(0, 1, &sampler);

				for (UINT tex = 0; tex < 1; tex++)
				{
					//ID3D11ShaderResourceView* texture = material.Textures[tex].GetAddressOf();

					if (true)//supportsShaderResources)
					{
						//deviceContext->VSSetShaderResources(0 + tex, 1, &texture);
						//deviceContext->VSSetShaderResources(MaxTextures + tex, 1, &texture);
					}

					deviceContext->PSSetShaderResources(0, 1, material.Textures[tex].GetAddressOf());

					if (material.normal_height > 0.0f)
					{
						deviceContext->PSSetShaderResources(3, 1, material.Normal.GetAddressOf());
					}
					//deviceContext->PSSetShaderResources(MaxTextures + tex, 1, &texture);
				}

				// Draw the submesh.
				deviceContext->DrawIndexed(submesh.PrimCount * 3, submesh.StartIndex, 0);
			
			}
		}
	}



	static void LoadFromFile(
		AllResources& graphics,
		const std::wstring& meshFilename,
		const std::wstring& shaderPathLocation,
		const std::wstring& texturePathLocation,
		std::vector<Mesh*>& loadedMeshes,
		float _scale
	)
	{
		//
		// clear output vector
		//
		if (true)
		{
			loadedMeshes.clear();
		}

		//
		// open the mesh file
		//
		FILE* fp = nullptr;
		_wfopen_s(&fp, meshFilename.c_str(), L"rb");
		if (fp == nullptr)
		{
			//std::wstring error = L"Mesh file could not be opened " + meshFilename + L"\n";
			//OutputDebugString(error.c_str());
		}
		else
		{
			//
			// read how many meshes are part of the scene
			//
			UINT meshCount = 0;
			fread(&meshCount, sizeof(meshCount), 1, fp);

			//
			// for each mesh in the scene, load it from the file
			//
			for (UINT i = 0; i < meshCount; i++)
			{
				Mesh* mesh = nullptr;
				Mesh::Load(fp, graphics, shaderPathLocation, texturePathLocation, mesh, _scale);
				if (mesh != nullptr)
				{
					loadedMeshes.push_back(mesh);
				}
			}
		}
	}


	static void Load(FILE* fp, AllResources& graphics, const std::wstring& shaderPathLocation, const std::wstring& texturePathLocation, Mesh*& outMesh, float _scale)
	{
		UNREFERENCED_PARAMETER(texturePathLocation);

		//
		// initialize output mesh
		//
		outMesh = nullptr;
		if (fp != nullptr)
		{
			Mesh* mesh = new Mesh();

			UINT nameLen = 0;
			fread(&nameLen, sizeof(nameLen), 1, fp);
			if (nameLen > 0)
			{
				std::vector<wchar_t> objName(nameLen);
				fread(&objName[0], sizeof(wchar_t), nameLen, fp);
				mesh->m_name = &objName[0];
			}

			//
			// read material count
			//
			UINT numMaterials = 0;
			fread(&numMaterials, sizeof(UINT), 1, fp);
			mesh->m_materials.resize(numMaterials);

			//
			// load each material
			//
			for (UINT i = 0; i < numMaterials; i++)
			{
				Material& material = mesh->m_materials[i];

				//
				// read material name
				//
				UINT stringLen = 0;
				fread(&stringLen, sizeof(stringLen), 1, fp);
				if (stringLen > 0)
				{
					std::vector<wchar_t> matName(stringLen);
					fread(&matName[0], sizeof(wchar_t), stringLen, fp);
					material.Name = &matName[0];
				}

				//
				// read ambient and diffuse properties of material
				//
				fread(material.Ambient, sizeof(material.Ambient), 1, fp);
				fread(material.Diffuse, sizeof(material.Diffuse), 1, fp);
				fread(material.Specular, sizeof(material.Specular), 1, fp);
				fread(&material.SpecularPower, sizeof(material.SpecularPower), 1, fp);
				fread(material.Emissive, sizeof(material.Emissive), 1, fp);
				fread(&material.UVTransform, sizeof(material.UVTransform), 1, fp);

				//
				// assign vertex shader and sampler state
				//
				//material.VertexShader = graphics.GetVertexShader();

				//material.SamplerState = graphics.GetSamplerState();

				//
				// read name of the pixel shader
				//
				stringLen = 0;
				fread(&stringLen, sizeof(stringLen), 1, fp);
				if (stringLen > 0)
				{
					//
					// read the pixel shader name
					//
					std::vector<wchar_t> pixelShaderName(stringLen);
					fread(&pixelShaderName[0], sizeof(wchar_t), stringLen, fp);
					std::wstring sourceFile = &pixelShaderName[0];

					//
					// continue loading pixel shader if name is not empty 
					//
					if (!sourceFile.empty())
					{
						// 
						// create well-formed file name for the pixel shader
						//
						Mesh::StripPath(sourceFile);

						//
						// use fallback shader if Pixel Shader Model 4.0 is not supported
						//
						/*
						if (graphics.GetDeviceFeatureLevel() < D3D_FEATURE_LEVEL_10_0)
						{
						//
						// this device is not compatible with Pixel Shader Model 4.0
						// try to fall back to a shader with the same name but compiled from HLSL
						//
						size_t lastUnderline = sourceFile.find_last_of('_');
						size_t firstDotAfterLastUnderline = sourceFile.find_first_of('.', lastUnderline);
						sourceFile = sourceFile.substr(lastUnderline + 1, firstDotAfterLastUnderline - lastUnderline) + L"cso";
						}

						//
						// append path
						//
						sourceFile = shaderPathLocation + sourceFile;


						//
						// get or create pixel shader
						//
						ID3D11PixelShader* materialPixelShader = graphics.GetOrCreatePixelShader(sourceFile);
						material.PixelShader = materialPixelShader;
						*/
					}
				}

				//
				// load textures
				//
				for (int t = 0; t < MaxTextures; t++)
				{
					//
					// read name of texture
					//
					stringLen = 0;
					fread(&stringLen, sizeof(stringLen), 1, fp);
					if (stringLen > 0)
					{
						//
						// read the texture name
						//
						std::vector<wchar_t> textureFilename(stringLen);
						fread(&textureFilename[0], sizeof(wchar_t), stringLen, fp);
						std::wstring sourceFile = &textureFilename[0];

						//
						// get or create texture
						//
						//ID3D11ShaderResourceView* textureResource = graphics.GetOrCreateTexture(sourceFile);
						//material.Textures[t] = textureResource;
					}
				}
			}

			//
			// does this object contain skeletal animation?
			//
			BYTE isSkeletalDataPresent = FALSE;
			fread(&isSkeletalDataPresent, sizeof(BYTE), 1, fp);

			//
			// read submesh info
			//
			UINT numSubmeshes = 0;
			fread(&numSubmeshes, sizeof(UINT), 1, fp);
			mesh->m_submeshes.resize(numSubmeshes);
			for (UINT i = 0; i < numSubmeshes; i++)
			{

				SubMesh& submesh = mesh->m_submeshes[i];


				//fread(&(mesh->m_submeshes[i]), sizeof(SubMesh), 1, fp);


				fread(&(submesh.MaterialIndex), sizeof(UINT), 1, fp);
				fread(&(submesh.IndexBufferIndex), sizeof(UINT), 1, fp);
				fread(&(submesh.VertexBufferIndex), sizeof(UINT), 1, fp);
				fread(&(submesh.StartIndex), sizeof(UINT), 1, fp);
				fread(&(submesh.PrimCount), sizeof(UINT), 1, fp);


			}


			//
			// read index buffers
			//
			UINT numIndexBuffers = 0;
			fread(&numIndexBuffers, sizeof(UINT), 1, fp);
			mesh->m_indexBuffers.resize(numIndexBuffers);

			std::vector<std::vector<USHORT>> indexBuffers(numIndexBuffers);

			for (UINT i = 0; i < numIndexBuffers; i++)
			{
				UINT ibCount = 0;
				fread(&ibCount, sizeof(UINT), 1, fp);
				if (ibCount > 0)
				{
					indexBuffers[i].resize(ibCount);

					//
					// read in the index data
					//
					fread(&indexBuffers[i][0], sizeof(USHORT), ibCount, fp);

					//
					// create an index buffer for this data
					//
					D3D11_BUFFER_DESC bd;
					ZeroMemory(&bd, sizeof(bd));
					bd.Usage = D3D11_USAGE_DEFAULT;
					bd.ByteWidth = sizeof(USHORT) * ibCount;
					bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
					bd.CPUAccessFlags = 0;

					D3D11_SUBRESOURCE_DATA initData;
					ZeroMemory(&initData, sizeof(initData));
					initData.pSysMem = &indexBuffers[i][0];

					graphics.m_deviceResources->GetD3DDevice()->CreateBuffer(&bd, &initData, &mesh->m_indexBuffers[i]);
				}
			}

			//
			// read vertex buffers
			//
			UINT numVertexBuffers = 0;
			fread(&numVertexBuffers, sizeof(UINT), 1, fp);
			mesh->m_vertexBuffers.resize(numVertexBuffers);

			std::vector<std::vector<Vertex>> vertexBuffers(numVertexBuffers);

			for (UINT i = 0; i < numVertexBuffers; i++)
			{
				UINT vbCount = 0;
				fread(&vbCount, sizeof(UINT), 1, fp);
				if (vbCount > 0)
				{
					vertexBuffers[i].resize(vbCount);

					//
					// read in the vertex data
					//
					fread(&vertexBuffers[i][0], sizeof(Vertex), vbCount, fp);


					for (int z = 0; z < vbCount; z++)
					{
						vertexBuffers[i][z].v = 1.0f - vertexBuffers[i][z].v;

					}

					for (int z = 0; z < vbCount; z++)
					{
						//awwwwvertexBuffers[i][z].v = 0.0f - vertexBuffers[i][z].v;
						MeshPoint point;
						if (true)// (isSkeletalDataPresent == false)
						{
							//vertexBuffers[i][z].u *= 0.01f;
							//vertexBuffers[i][z].v *= 0.01f;
							vertexBuffers[i][z].x *= _scale;
							vertexBuffers[i][z].y *= _scale;
							vertexBuffers[i][z].z *= _scale;

							point.point.x = vertexBuffers[i][z].x;
							point.point.y = vertexBuffers[i][z].y;
							point.point.z = vertexBuffers[i][z].z;
						}
						
						mesh->m_points.push_back(point);
					}

					//
					// create a vertex buffer for this data
					//
					D3D11_BUFFER_DESC bd;
					ZeroMemory(&bd, sizeof(bd));
					bd.Usage = D3D11_USAGE_DEFAULT;
					bd.ByteWidth = sizeof(Vertex) * vbCount;
					bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
					bd.CPUAccessFlags = 0;

					D3D11_SUBRESOURCE_DATA initData;
					ZeroMemory(&initData, sizeof(initData));
					initData.pSysMem = &vertexBuffers[i][0];

					graphics.m_deviceResources->GetD3DDevice()->CreateBuffer(&bd, &initData, &mesh->m_vertexBuffers[i]);
				}
			}

			for (SubMesh& subMesh : mesh->m_submeshes)
			{
				std::vector<USHORT>& ib = indexBuffers[subMesh.IndexBufferIndex];
				std::vector<Vertex>& vb = vertexBuffers[subMesh.VertexBufferIndex];

				for (UINT j = 0; j < ib.size(); j += 3)
				{
					Vertex& v0 = vb[ib[j]];
					Vertex& v1 = vb[ib[j + 1]];
					Vertex& v2 = vb[ib[j + 2]];

					Triangle tri;
					tri.points[0].x = v0.x;
					tri.points[0].y = v0.y;
					tri.points[0].z = v0.z;

					tri.points[1].x = v1.x;
					tri.points[1].y = v1.y;
					tri.points[1].z = v1.z;

					tri.points[2].x = v2.x;
					tri.points[2].y = v2.y;
					tri.points[2].z = v2.z;

					mesh->m_triangles.push_back(tri);
				}
			}

			// done with temp buffers
			vertexBuffers.clear();
			indexBuffers.clear();

			//
			// read skinning vertex buffers
			//
			UINT numSkinningVertexBuffers = 0;
			fread(&numSkinningVertexBuffers, sizeof(UINT), 1, fp);
			mesh->m_skinningVertexBuffers.resize(numSkinningVertexBuffers);
			for (UINT i = 0; i < numSkinningVertexBuffers; i++)
			{
				UINT vbCount = 0;
				fread(&vbCount, sizeof(UINT), 1, fp);
				if (vbCount > 0)
				{
					std::vector<SkinningVertex> verts(vbCount);
					std::vector<SkinningVertexInput> input(vbCount);

					//
					// read in the vertex data
					//
					fread(&verts[0], sizeof(SkinningVertex), vbCount, fp);

					//
					// convert indices to byte (to support D3D Feature Level 9)
					//
					for (UINT j = 0; j < vbCount; j++)
					{
						for (int k = 0; k < NUM_BONE_INFLUENCES; k++)
						{
							input[j].boneIndex[k] = (byte)verts[j].boneIndex[k];
							input[j].boneWeight[k] = verts[j].boneWeight[k];
						}
					}

					//
					// create a vertex buffer for this data
					//
					D3D11_BUFFER_DESC bd;
					ZeroMemory(&bd, sizeof(bd));
					bd.Usage = D3D11_USAGE_DEFAULT;
					bd.ByteWidth = sizeof(SkinningVertexInput) * vbCount;
					bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
					bd.CPUAccessFlags = 0;

					D3D11_SUBRESOURCE_DATA initData;
					ZeroMemory(&initData, sizeof(initData));
					initData.pSysMem = &input[0];

					graphics.m_deviceResources->GetD3DDevice()->CreateBuffer(&bd, &initData, &mesh->m_skinningVertexBuffers[i]);
				}
			}

			//
			// read extents
			//
			fread(&mesh->m_meshExtents, sizeof(MeshExtents), 1, fp);


			mesh->m_meshExtents.CenterX *= _scale;
			mesh->m_meshExtents.CenterY *= _scale;
			mesh->m_meshExtents.CenterZ *= _scale;
			mesh->m_meshExtents.MaxX *= _scale;
			mesh->m_meshExtents.MaxY *= _scale;
			mesh->m_meshExtents.MaxZ *= _scale;
			mesh->m_meshExtents.MinX *= _scale;
			mesh->m_meshExtents.MinY *= _scale;
			mesh->m_meshExtents.MinZ *= _scale;

			//
			// do we need to read bones and animation?
			//
			if (isSkeletalDataPresent)
			{
				//
				// read bones
				//
				UINT boneCount = 0;
				fread(&boneCount, sizeof(UINT), 1, fp);

				mesh->m_boneInfo.resize(boneCount);

				for (UINT b = 0; b < boneCount; b++)
				{
					// read the bone name (length, then chars)
					UINT nameLength = 0;
					fread(&nameLength, sizeof(UINT), 1, fp);

					if (nameLength > 0)
					{
						std::vector<wchar_t> nameVec(nameLength);
						fread(&nameVec[0], sizeof(wchar_t), nameLength, fp);

						mesh->m_boneInfo[b].Name = &nameVec[0];
					}

					// read the transforms
					fread(&mesh->m_boneInfo[b].ParentIndex, sizeof(INT), 1, fp);
					fread(&mesh->m_boneInfo[b].InvBindPos, sizeof(DirectX::XMFLOAT4X4), 1, fp);
					fread(&mesh->m_boneInfo[b].BindPose, sizeof(DirectX::XMFLOAT4X4), 1, fp);
					fread(&mesh->m_boneInfo[b].BoneLocalTransform, sizeof(DirectX::XMFLOAT4X4), 1, fp);
				}

				//
				// read animation clips
				//
				UINT clipCount = 0;
				fread(&clipCount, sizeof(UINT), 1, fp);

				for (UINT j = 0; j < clipCount; j++)
				{
					// read clip name
					UINT len = 0;
					fread(&len, sizeof(UINT), 1, fp);

					std::wstring clipName;
					if (len > 0)
					{
						std::vector<wchar_t> clipNameVec(len);
						fread(&clipNameVec[0], sizeof(wchar_t), len, fp);

						clipName = &clipNameVec[0];
					}

					fread(&mesh->m_animationClips[clipName].StartTime, sizeof(float), 1, fp);
					fread(&mesh->m_animationClips[clipName].EndTime, sizeof(float), 1, fp);

					KeyframeArray& keyframes = mesh->m_animationClips[clipName].Keyframes;

					// read keyframecount
					UINT kfCount = 0;
					fread(&kfCount, sizeof(UINT), 1, fp);

					// preallocate the memory
					keyframes.reserve(kfCount);

					// read each keyframe
					for (UINT k = 0; k < kfCount; k++)
					{
						Keyframe kf;

						// read the bone
						fread(&kf.BoneIndex, sizeof(UINT), 1, fp);

						// read the time
						fread(&kf.Time, sizeof(UINT), 1, fp);

						// read the transform
						fread(&kf.Transform, sizeof(DirectX::XMFLOAT4X4), 1, fp);

						// add to collection
						keyframes.push_back(kf);
					}
				}
			}

			//
			// set the output mesh
			//
			outMesh = mesh;
		}
	}

	// Loads a scene from the specified file, returning a vector of mesh objects.
	static concurrency::task<void> LoadFromFileAsync(
		AllResources& graphics,
		const std::wstring& meshFilename,
		const std::wstring& shaderPathLocation,
		const std::wstring& texturePathLocation,
		std::vector<Mesh*>& loadedMeshes,
		float _scale = 1.0f
		)
	{
		// Clear the output vector.
		if (true)
		{
			loadedMeshes.clear();
		}

		auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;

		// Start by loading the file.
		return concurrency::create_task(folder->GetFileAsync(Platform::StringReference(meshFilename.c_str())))
			// Then open the file.
			.then([](Windows::Storage::StorageFile^ file)
		{
			return Windows::Storage::FileIO::ReadBufferAsync(file);
		})
			// Then read the meshes.
			.then([&graphics, _scale , shaderPathLocation, texturePathLocation, &loadedMeshes](Windows::Storage::Streams::IBuffer^ buffer)
		{
			auto reader = Windows::Storage::Streams::DataReader::FromBuffer(buffer);
			reader->ByteOrder = Windows::Storage::Streams::ByteOrder::LittleEndian;
			reader->UnicodeEncoding = Windows::Storage::Streams::UnicodeEncoding::Utf8;

			// Read how many meshes are part of the scene.
			std::shared_ptr<UINT> remainingMeshesToRead = std::make_shared<UINT>(reader->ReadUInt32());

			if (*remainingMeshesToRead == 0)
			{
				return concurrency::create_task([]() {});
			}

			// For each mesh in the scene, load it from the file.

			return concurrency::extras::create_iterative_task([reader, _scale, &graphics, shaderPathLocation, texturePathLocation, &loadedMeshes, remainingMeshesToRead]()
			{
				return Mesh::ReadAsync(reader, graphics, shaderPathLocation, texturePathLocation, _scale)
					.then([&loadedMeshes, remainingMeshesToRead](Mesh* mesh) -> bool
				{
					if (mesh != nullptr)
					{
						loadedMeshes.push_back(mesh);
					}

					// Return true to continue iterating, false to stop.

					*remainingMeshesToRead = *remainingMeshesToRead - 1;

					return *remainingMeshesToRead > 0;
				});
			});
		});
	}



private:
	Mesh()
	{
		Tag = nullptr;
	}

	static void StripPath(std::wstring& path)
	{
		size_t p = path.rfind(L"\\");
		if (p != std::wstring::npos)
		{
			path = path.substr(p + 1);
		}
	}

	template <typename T>
	static void ReadStruct(Windows::Storage::Streams::DataReader^ reader, T* output, size_t size = sizeof(T))
	{
		reader->ReadBytes(Platform::ArrayReference<BYTE>((BYTE*)output, size));
	}

	// Reads a string converted from an array of wchar_t of given size using a DataReader.
	static void ReadString(Windows::Storage::Streams::DataReader^ reader, std::wstring* output, unsigned int count)
	{
		if (count == 0)
		{
			return;
		}

		std::vector<wchar_t> characters(count);
		ReadStruct(reader, &characters[0], count * sizeof(wchar_t));
		*output = &characters[0];
	}

	// Reads a string converted from an array of wchar_t using a DataReader.
	static void ReadString(Windows::Storage::Streams::DataReader^ reader, std::wstring* output)
	{
		UINT count = reader->ReadUInt32();
		ReadString(reader, output, count);
	}



	static concurrency::task<Mesh*> ReadAsync(Windows::Storage::Streams::DataReader^ reader, AllResources& graphics, const std::wstring& shaderPathLocation, const std::wstring& texturePathLocation,float _scale=1.0f)
	{
		std::vector<concurrency::task<void>> innerTasks;

		// Initialize output mesh.
		if (reader == nullptr)
		{
			return concurrency::extras::create_value_task<Mesh*>(nullptr);
		}

		Mesh* mesh = new Mesh();

		ReadString(reader, &mesh->m_name);

		// Read material count.
		UINT numMaterials = reader->ReadUInt32();
		mesh->m_materials.resize(numMaterials);

		//if (!mesh->m_name.compare(L"my_avatar"))
		//{
		//	mesh->m_materials.resize(numMaterials);
	//	}
		// Load each material.
		for (UINT i = 0; i < numMaterials; i++)
		{
			Material& material = mesh->m_materials[i];

			// Read the material name.
			ReadString(reader, &material.Name);

			// Read the ambient and diffuse properties of material.

			ReadStruct(reader, &material.Ambient, sizeof(material.Ambient));
			ReadStruct(reader, &material.Diffuse, sizeof(material.Diffuse));
			ReadStruct(reader, &material.Specular, sizeof(material.Specular));
			ReadStruct(reader, &material.SpecularPower);
			ReadStruct(reader, &material.Emissive, sizeof(material.Emissive));
			ReadStruct(reader, &material.UVTransform);
			
			material.SpecularPower *= 2.0f;
			//if (material.Name == L"lamp_glass")
			//{
				material.Ambient[0] = 0.5f;
				material.Ambient[1] = 0.5f;
				material.Ambient[2] = 0.5f;
				material.Ambient[3] = 1.0f;
			//}

			if (material.Specular[0] == 0 || material.Specular[1] == 0 || material.Specular[2] == 0)
			{
				material.Specular[3] = 0.0f;
			}

			// Assign the vertex shader and sampler state.
			//material.VertexShader = graphics.GetVertexShader();

			//material.SamplerState = graphics.GetSamplerState();

			// Read the size of the name of the pixel shader.
			UINT stringLen = reader->ReadUInt32();
			if (stringLen > 0)
			{
				// Read the pixel shader name.
				std::wstring sourceFile;
				ReadString(reader, &sourceFile, stringLen);

				// Continue loading pixel shader if name is not empty.
				if (!sourceFile.empty())
				{
					// Create well-formed file name for the pixel shader.
					Mesh::StripPath(sourceFile);

					// Use fallback shader if Pixel Shader Model 4.0 is not supported.
					if (true) //(graphics.GetDeviceFeatureLevel() < D3D_FEATURE_LEVEL_10_0)
					{
						// This device is not compatible with Pixel Shader Model 4.0.
						// Try to fall back to a shader with the same name but compiled from HLSL.
						size_t lastUnderline = sourceFile.find_last_of('_');
						size_t firstDotAfterLastUnderline = sourceFile.find_first_of('.', lastUnderline);
						sourceFile = sourceFile.substr(lastUnderline + 1, firstDotAfterLastUnderline - lastUnderline) + L"cso";
					}

					// Append the base path.
					sourceFile = shaderPathLocation + sourceFile;

					// Get or create the pixel shader.
					//innerTasks.push_back(graphics.GetOrCreatePixelShaderAsync(sourceFile).then([&material](ID3D11PixelShader* materialPixelShader)
					//{
					//	material.PixelShader = materialPixelShader;
					//}));
				}
			}

			// Load the textures.
			
			for (int t = 0; t < MaxTextures; t++)
			{
				// Read the size of the name of the texture.
				stringLen = reader->ReadUInt32();
				if (stringLen > 0)
				{
					// Read the texture name.
					std::wstring sourceFile;
					ReadString(reader, &sourceFile, stringLen);

					// Append the base path.
					sourceFile = texturePathLocation + sourceFile;

					// Get or create the texture.
					//innerTasks.push_back(graphics.GetOrCreateTextureAsync(sourceFile).then([&material, t](ID3D11ShaderResourceView* textureResource)
					//{
					//	material.Textures[t] = textureResource;
					//}));
				}
			}
			
		}

		// Does this object contain skeletal animation?
		BYTE isSkeletalDataPresent = reader->ReadByte();

		// Read the submesh information.
		UINT numSubmeshes = reader->ReadUInt32();
		mesh->m_submeshes.resize(numSubmeshes);
		for (UINT i = 0; i < numSubmeshes; i++)
		{
			SubMesh& submesh = mesh->m_submeshes[i];
			submesh.MaterialIndex = reader->ReadUInt32();
			submesh.IndexBufferIndex = reader->ReadUInt32();
			submesh.VertexBufferIndex = reader->ReadUInt32();
			submesh.StartIndex = reader->ReadUInt32();
			submesh.PrimCount = reader->ReadUInt32();
		}

		// Read the index buffers.
		UINT numIndexBuffers = reader->ReadUInt32();
		mesh->m_indexBuffers.resize(numIndexBuffers);

		std::vector<std::vector<USHORT>> indexBuffers(numIndexBuffers);

		for (UINT i = 0; i < numIndexBuffers; i++)
		{
			UINT ibCount = reader->ReadUInt32();
			if (ibCount > 0)
			{
				indexBuffers[i].resize(ibCount);

				// Read in the index data.
				for (USHORT& component : indexBuffers[i])
				{
					component = reader->ReadUInt16();
				}

				// Create an index buffer for this data.
				D3D11_BUFFER_DESC bd = { 0 };
				bd.Usage = D3D11_USAGE_DEFAULT;
				bd.ByteWidth = sizeof(USHORT)* ibCount;
				bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
				bd.CPUAccessFlags = 0;

				D3D11_SUBRESOURCE_DATA initData = { 0 };
				initData.pSysMem = &indexBuffers[i][0];

				graphics.m_deviceResources->GetD3DDevice()->CreateBuffer(&bd, &initData, &mesh->m_indexBuffers[i]);
			}
		}

		// Read the vertex buffers.
		UINT numVertexBuffers = reader->ReadUInt32();
		mesh->m_vertexBuffers.resize(numVertexBuffers);

		std::vector<std::vector<Vertex>> vertexBuffers(numVertexBuffers);

		for (UINT i = 0; i < numVertexBuffers; i++)
		{
			UINT vbCount = reader->ReadUInt32();
			if (vbCount > 0)
			{
				vertexBuffers[i].resize(vbCount);

				// Read in the vertex data.
				ReadStruct(reader, &vertexBuffers[i][0], vbCount * sizeof(Vertex));

				//&vertexBuffers[i][0].
				// Flip the u
				for (int z = 0; z < vbCount; z++)
				{
					vertexBuffers[i][z].v = 0.0f -vertexBuffers[i][z].v;

					MeshPoint point;
					if (true)// (isSkeletalDataPresent == false)
					{
						//vertexBuffers[i][z].u *= 0.01f;
						//vertexBuffers[i][z].v *= 0.01f;
						point.point.x = vertexBuffers[i][z].x *= _scale;
						point.point.y = vertexBuffers[i][z].y *= _scale;
						point.point.z = vertexBuffers[i][z].z *= _scale;

						

						mesh->m_points.push_back(point);
					}

				}

				// Create a vertex buffer for this data.
				D3D11_BUFFER_DESC bd = { 0 };
				bd.Usage = D3D11_USAGE_DEFAULT;
				bd.ByteWidth = sizeof(Vertex)* vbCount;
				bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bd.CPUAccessFlags = 0;

				D3D11_SUBRESOURCE_DATA initData = { 0 };
				initData.pSysMem = &vertexBuffers[i][0];

				graphics.m_deviceResources->GetD3DDevice()->CreateBuffer(&bd, &initData, &mesh->m_vertexBuffers[i]);
			}
		}

		// Create the triangle array for each submesh.
		for (SubMesh& subMesh : mesh->m_submeshes)
		{
			std::vector<USHORT>& ib = indexBuffers[subMesh.IndexBufferIndex];
			std::vector<Vertex>& vb = vertexBuffers[subMesh.VertexBufferIndex];



			for (UINT j = 0; j < ib.size(); j += 3)
			{
				Vertex& v0 = vb[ib[j]];
				Vertex& v1 = vb[ib[j + 1]];
				Vertex& v2 = vb[ib[j + 2]];



				Triangle tri;
				tri.points[0].x = v0.x;
				tri.points[0].y = v0.y;
				tri.points[0].z = v0.z;

				tri.points[1].x = v1.x;
				tri.points[1].y = v1.y;
				tri.points[1].z = v1.z;

				tri.points[2].x = v2.x;
				tri.points[2].y = v2.y;
				tri.points[2].z = v2.z;

				mesh->m_triangles.push_back(tri);
			}
		}

		// Clear the temporary buffers.
		vertexBuffers.clear();
		indexBuffers.clear();

		// Read the skinning vertex buffers.
		UINT numSkinningVertexBuffers = reader->ReadUInt32();
		mesh->m_skinningVertexBuffers.resize(numSkinningVertexBuffers);
		for (UINT i = 0; i < numSkinningVertexBuffers; i++)
		{
			UINT vbCount = reader->ReadUInt32();
			if (vbCount > 0)
			{
				std::vector<SkinningVertexInput> verts(vbCount);

				// Read in the vertex data.
				for (SkinningVertexInput& vertex : verts)
				{
					for (size_t j = 0; j < NUM_BONE_INFLUENCES; j++)
					{
						// Convert indices to byte (to support D3D Feature Level 9).
						vertex.boneIndex[j] = (byte)reader->ReadUInt32();
					}
					for (size_t j = 0; j < NUM_BONE_INFLUENCES; j++)
					{
						vertex.boneWeight[j] = reader->ReadSingle();
					}
				}

				// Create a vertex buffer for this data.
				D3D11_BUFFER_DESC bd = { 0 };
				bd.Usage = D3D11_USAGE_DEFAULT;
				bd.ByteWidth = sizeof(SkinningVertexInput)* vbCount;
				bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bd.CPUAccessFlags = 0;

				D3D11_SUBRESOURCE_DATA initData = { 0 };
				initData.pSysMem = &verts[0];

				graphics.m_deviceResources->GetD3DDevice()->CreateBuffer(&bd, &initData, &mesh->m_skinningVertexBuffers[i]);
			}
		}

		// Read the mesh extents.
		ReadStruct(reader, &mesh->m_meshExtents);

		mesh->m_meshExtents.CenterX *= _scale;
		mesh->m_meshExtents.CenterY *= _scale;
		mesh->m_meshExtents.CenterZ *= _scale;
		mesh->m_meshExtents.MaxX *= _scale;
		mesh->m_meshExtents.MaxY *= _scale;
		mesh->m_meshExtents.MaxZ *= _scale;
		mesh->m_meshExtents.MinX *= _scale;
		mesh->m_meshExtents.MinY *= _scale;
		mesh->m_meshExtents.MinZ *= _scale;

		// Read bones and animation information if needed.
		if (isSkeletalDataPresent)
		{
			// Read bone information.
			UINT boneCount = reader->ReadUInt32();

			mesh->m_boneInfo.resize(boneCount);

			for (BoneInfo& bone : mesh->m_boneInfo)
			{
				// Read the bone name.
				ReadString(reader, &bone.Name);

				// Read the transforms.
				bone.ParentIndex = reader->ReadInt32();
				ReadStruct(reader, &bone.InvBindPos);
				ReadStruct(reader, &bone.BindPose);
				ReadStruct(reader, &bone.BoneLocalTransform);

				/*
				bone.BoneLocalTransform._14 += 0.01f;
				bone.BoneLocalTransform._24 += 0.01f;
				bone.BoneLocalTransform._34 += 0.01f;
				bone.InvBindPos._14 += 0.01f;
				bone.InvBindPos._24 += 0.01f;
				bone.InvBindPos._34 += 0.01f;
				*/
				//bone.
			}

			// Read animation clips.
			UINT clipCount = reader->ReadUInt32();

			for (UINT j = 0; j < clipCount; j++)
			{
				// Read clip name.
				std::wstring clipName;
				ReadString(reader, &clipName);

				mesh->m_animationClips[clipName].StartTime = reader->ReadSingle();
				mesh->m_animationClips[clipName].EndTime = reader->ReadSingle();

				KeyframeArray& keyframes = mesh->m_animationClips[clipName].Keyframes;

				// Read keyframe count.
				UINT kfCount = reader->ReadUInt32();

				// Preallocate the memory.
				keyframes.reserve(kfCount);

				// Read each keyframe.
				for (UINT k = 0; k < kfCount; k++)
				{
					Keyframe kf;

					// Read the bone.
					kf.BoneIndex = reader->ReadUInt32();

					// Read the time.
					kf.Time = reader->ReadSingle();

					// Read the transform.
					ReadStruct(reader, &kf.Transform);

					// Add to the keyframe collection.
					keyframes.push_back(kf);
				}
			}
		}

		return concurrency::when_all(innerTasks.begin(), innerTasks.end()).then([mesh]()
		{
			return mesh;
		});
	}

	std::vector<SubMesh> m_submeshes;
	std::vector<Material> m_materials;
	std::vector<ID3D11Buffer*> m_vertexBuffers;
	std::vector<ID3D11Buffer*> m_skinningVertexBuffers;
	std::vector<ID3D11Buffer*> m_indexBuffers;
	TriangleCollection m_triangles;
	PointCollection m_points;

	MeshExtents m_meshExtents;

	AnimationClipMap m_animationClips;
	std::vector<BoneInfo> m_boneInfo;

	std::wstring m_name;
};
//
//
///////////////////////////////////////////////////////////////////////////////////////////

}