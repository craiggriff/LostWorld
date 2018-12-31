#include "pch.h"
#include "..\Common\DirectXHelper.h"
#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"

#include "camera.h"
#include "Lights.h"
#include "Graphics.h"
#include "Mesh.h"
#include "Animation.h"


using namespace _3dtest;
using namespace DirectX;


void SkinnedMeshRenderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	UNREFERENCED_PARAMETER(deviceContext);

	m_skinningShader = nullptr;
	m_boneConstantBuffer = nullptr;
	m_skinningVertexLayout = nullptr;

	//
	// create constant buffers
	//
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	bufferDesc.ByteWidth = sizeof(BoneConstants);
	device->CreateBuffer(&bufferDesc, nullptr, &m_boneConstantBuffer);

	//
	// skinning assets
	//
	FILE* fp = NULL;

	fopen_s(&fp, "SkinningVertexShader.cso", "rb");
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::vector<BYTE> buffer(size);
	if (size > 0)
	{
		fread(&buffer[0], 1, size, fp);

		device->CreateVertexShader(&buffer[0], size, NULL, &m_skinningShader);
	}

	fclose(fp);

	//
	// create the vertex layout
	//
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	device->CreateInputLayout(layout, ARRAYSIZE(layout), &buffer[0], buffer.size(), &m_skinningVertexLayout);
}


void SkinnedMeshRenderer::UpdateAnimation(float timeDelta, std::vector<Mesh*>& meshes)
{
	for (Mesh* mesh : meshes)
	{
		AnimationState* animState = (AnimationState*)mesh->Tag;
		if (animState == NULL)
		{
			continue;
		}

		animState->m_animTime += timeDelta;

		//
		// Update bones
		//
		const std::vector<Mesh::BoneInfo>& skinningInfo = mesh->BoneInfoCollection();
		for (UINT b = 0; b < skinningInfo.size(); b++)
		{
			animState->m_boneWorldTransforms[b] = skinningInfo[b].BoneLocalTransform;
		}

		// get keyframes
		auto& animClips = mesh->AnimationClips();
		auto found = animClips.find(L"my_avatar|run");
		if (found != animClips.end())
		{
			const auto& kf = found->second.Keyframes;
			for (const auto& frame : kf)
			{
				if (frame.Time > animState->m_animTime)
				{
					break;
				}

				animState->m_boneWorldTransforms[frame.BoneIndex] = frame.Transform;
			}
			// transform to world
			for (UINT b = 1; b < skinningInfo.size(); b++)
			{
				const Mesh::BoneInfo& skinning = skinningInfo[b];

				if (skinning.ParentIndex < 0)
					continue;

				DirectX::XMMATRIX leftMat = XMLoadFloat4x4(&animState->m_boneWorldTransforms[b]);
				DirectX::XMMATRIX rightMat = XMLoadFloat4x4(&animState->m_boneWorldTransforms[skinning.ParentIndex]);

				DirectX::XMMATRIX ret = leftMat * rightMat;

				XMStoreFloat4x4(&animState->m_boneWorldTransforms[b], ret);
			}

			for (UINT b = 0; b < skinningInfo.size(); b++)
			{
				DirectX::XMMATRIX leftMat = XMLoadFloat4x4(&skinningInfo[b].InvBindPos);
				DirectX::XMMATRIX rightMat = XMLoadFloat4x4(&animState->m_boneWorldTransforms[b]);

				DirectX::XMMATRIX ret = leftMat * rightMat;

				XMStoreFloat4x4(&animState->m_boneWorldTransforms[b], ret);
			}

			if (animState->m_animTime > found->second.EndTime)
			{
				animState->m_animTime = found->second.StartTime + (animState->m_animTime - found->second.EndTime);
			}
		}
	}
}

// Renders the mesh using the skinned mesh renderer.
void SkinnedMeshRenderer::RenderSkinnedMesh(Mesh* mesh, const Graphics& graphics)
{
	ID3D11DeviceContext* deviceContext = graphics.GetDeviceContext();

	BOOL supportsShaderResources = false;// graphics.m_deviceResources->GetDeviceFeatureLevel() >= D3D_FEATURE_LEVEL_10_0;

										 // Assign constant buffers to correct slots.
										 //ID3D11Buffer* constantBuffer = graphics.GetLightConstants();
										 //deviceContext->VSSetConstantBuffers(1, 1, &constantBuffer);
										 //deviceContext->PSSetConstantBuffers(1, 1, &constantBuffer);

										 //constantBuffer = graphics.GetMiscConstants();
										 //deviceContext->VSSetConstantBuffers(3, 1, &constantBuffer);
										 //deviceContext->PSSetConstantBuffers(3, 1, &constantBuffer);

	ID3D11Buffer* boneConsts = m_boneConstantBuffer.Get();
	deviceContext->VSSetConstantBuffers(2, 1, &boneConsts);

	//ID3D11Buffer* constantBuffer = graphics.GetObjectConstants();
	//deviceContext->VSSetConstantBuffers(1, 1, &constantBuffer);
	//deviceContext->PSSetConstantBuffers(1, 1, &constantBuffer);

	// Prepare to draw.

	// NOTE: Set the skinning vertex layout.
	//deviceContext->IASetInputLayout(m_skinningVertexLayout.Get());
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	BoneConstants boneConstants;

	// Update the bones.
	AnimationState* animState = (AnimationState*)mesh->Tag;
	if (animState != nullptr)
	{
		// Copy to constants.
		for (UINT b = 0; b < animState->m_boneWorldTransforms.size(); b++)
		{
			boneConstants.Bones[b] = DirectX::XMMatrixTranspose((XMLoadFloat4x4(&animState->m_boneWorldTransforms[b])));
		}
	}

	// Update the constants.
	deviceContext->UpdateSubresource(m_boneConstantBuffer.Get(), 0, nullptr, &boneConstants, 0, 0);

	// Loop over each submesh.
	for (const Mesh::SubMesh& submesh : mesh->SubMeshes())
	{
		// Only draw submeshes that have valid materials.
		MaterialConstants materialConstants;

		if (submesh.IndexBufferIndex < mesh->IndexBuffers().size() &&
			submesh.VertexBufferIndex < mesh->VertexBuffers().size())
		{
			ID3D11Buffer* vbs[2] =
			{
				mesh->VertexBuffers()[submesh.VertexBufferIndex],
				mesh->SkinningVertexBuffers()[submesh.VertexBufferIndex]
			};

			UINT stride[2] = { sizeof(Vertex), sizeof(SkinningVertexInput) };
			UINT offset[2] = { 0, 0 };
			deviceContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
			deviceContext->IASetIndexBuffer(mesh->IndexBuffers()[submesh.IndexBufferIndex], DXGI_FORMAT_R16_UINT, 0);
		}

		//deviceContext->UpdateSubresource(materialBuffer.Get(), 0, nullptr, &submesh.m_material, 0, 0);

		if (submesh.MaterialIndex < mesh->Materials().size())
		{
			const Mesh::Material& material = mesh->Materials()[submesh.MaterialIndex];



			MaterialBufferType mat_buffer;

			// Update the material constant buffer.
			memcpy(&mat_buffer.Ambient, material.Ambient, sizeof(material.Ambient));
			memcpy(&mat_buffer.Diffuse, material.Diffuse, sizeof(material.Diffuse));
			memcpy(&mat_buffer.Specular, material.Specular, sizeof(material.Specular));
			memcpy(&mat_buffer.Emissive, material.Emissive, sizeof(material.Emissive));
			mat_buffer.SpecularPower = material.SpecularPower;
			mat_buffer.normal_height = material.normal_height;

			graphics.UpdateMaterialBuffer(&mat_buffer);



			/*
			// Update the material constant buffer.
			memcpy(&materialConstants.Ambient, material.Ambient, sizeof(material.Ambient));
			memcpy(&materialConstants.Diffuse, material.Diffuse, sizeof(material.Diffuse));
			memcpy(&materialConstants.Specular, material.Specular, sizeof(material.Specular));
			memcpy(&materialConstants.Emissive, material.Emissive, sizeof(material.Emissive));
			materialConstants.SpecularPower = material.SpecularPower;

			graphics.UpdateMaterialConstants(materialConstants);

			// Assign the material buffer to the correct slots.
			constantBuffer = graphics.GetMaterialConstants();
			deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
			deviceContext->PSSetConstantBuffers(0, 1, &constantBuffer);

			// Update the UV transform.
			memcpy(&objConstants.UvTransform4x4, &material.UVTransform, sizeof(objConstants.UvTransform4x4));
			graphics.UpdateObjectConstants(objConstants);
			*/


			// Assign shaders, samplers and texture resources.

			/*
			// NOTE: Set the skinning shader here.
			deviceContext->VSSetShader(m_skinningShader.Get(), nullptr, 0);

			ID3D11SamplerState* samplerState = material.SamplerState.Get();
			if (supportsShaderResources)
			{
			deviceContext->VSSetSamplers(0, 1, &samplerState);
			}

			deviceContext->PSSetShader(material.PixelShader.Get(), nullptr, 0);
			deviceContext->PSSetSamplers(0, 1, &samplerState);
			*/
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

	// Clear the extra vertex buffer.
	ID3D11Buffer* vbs[1] = { nullptr };
	UINT stride = 0;
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(1, 1, vbs, &stride, &offset);
}