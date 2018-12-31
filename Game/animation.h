// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// This header shows you how to render animated meshes.
// If you don't need animations, remove this file and fix any build errors
// by removing all references to the SkinnedMeshedRenderer class.

#pragma once
#include "pch.h"
//#include "mesh.h"

#include "DefParticle.h"

namespace Game
{

	struct BoneConstants
	{
		BoneConstants()
		{
			static_assert((sizeof(BoneConstants) % 16) == 0, "CB must be 16-byte aligned");
		}

		DirectX::XMFLOAT4X4 Bones[MAX_BONES];
	};

	struct AnimationState
	{
		AnimationState()
		{
			m_animTime = 0;
			m_animTimeA = 0.0f;
			m_animTimeB = 0.0f;
			m_animTimeC = 0.0f;
			m_animTimeD = 0.0f;
		}

		std::vector<DirectX::XMFLOAT4X4> m_boneWorldTransforms;
		std::vector<DirectX::XMFLOAT4X4> m_boneWorldTransformsB; 
		std::vector<DirectX::XMFLOAT4X4> m_boneWorldTransformsC; 
		std::vector<DirectX::XMFLOAT4X4> m_boneWorldTransformsD;

		float m_animTime;
		float m_animTimeA;
		float m_animTimeB;
		float m_animTimeC;
		float m_animTimeD;
	};

	inline float flerp(float a, float b, float f)
	{
		return (a * (1.0f - f)) + (b * f);
	}


	class SkinnedMeshRenderer
	{
	public:
		SkinnedMeshRenderer() {}

		~SkinnedMeshRenderer() {}

		int lastKeyFrame;
		int lastKeyFrameA;
		int lastKeyFrameB;
		int lastKeyFrameC;
		int lastKeyFrameD;

		bool resetTimerA;
		bool resetTimerB;
		bool resetTimerC;
		bool resetTimerD;

		Microsoft::WRL::ComPtr<ID3D11Buffer> materialBuffer;

		// Initializes the skinned mesh renderer, loading the vertex shader asynchronously.
		void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, Microsoft::WRL::ComPtr<ID3D11Buffer> p_materialBuffer)
		{
			UNREFERENCED_PARAMETER(deviceContext);

			materialBuffer = p_materialBuffer;

			//m_skinningShader = nullptr;
			m_boneConstantBuffer = nullptr;
			//m_skinningVertexLayout = nullptr;

			// Create constant buffers.
			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;

			bufferDesc.ByteWidth = sizeof(BoneConstants);
			device->CreateBuffer(&bufferDesc, nullptr, &m_boneConstantBuffer);


			lastKeyFrame = 0;
			lastKeyFrameA = 0;
			lastKeyFrameB = 0;
			lastKeyFrameC = 0;
			lastKeyFrameD = 0;

			resetTimerA = false;
			resetTimerB = false;
			resetTimerC = false;
			resetTimerD = false;
			// Load skinning assets.
			/*
			return DX::ReadDataAsync(L"SkinningVertexShader.cso").then([this, device](const std::vector<byte>& vsBuffer)
			{
				if (vsBuffer.size() > 0)
				{
					device->CreateVertexShader(&vsBuffer[0], vsBuffer.size(), nullptr, &m_skinningShader);
					if (m_skinningShader == nullptr)
					{
						throw std::exception("Pixel Shader could not be created");
					}
				}

				// Create the vertex layout.
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
				device->CreateInputLayout(layout, ARRAYSIZE(layout), &vsBuffer[0], vsBuffer.size(), &m_skinningVertexLayout);
			});
			*/
		}

		// Combines the transforms by taking into account all parent bone transforms recursively
		void CombineTransforms(uint32 currentBoneIndex, std::vector<Game::Mesh::BoneInfo> const& skinningInfo, std::vector<DirectX::XMFLOAT4X4>& boneWorldTransforms)
		{
			auto bone = skinningInfo[currentBoneIndex];
			if (m_isTransformCombined[currentBoneIndex] || bone.ParentIndex < 0 || bone.ParentIndex == static_cast<int>(currentBoneIndex))
			{
				m_isTransformCombined[currentBoneIndex] = true;
				return;
			}

			CombineTransforms(bone.ParentIndex, skinningInfo, boneWorldTransforms);

			DirectX::XMMATRIX leftMat = XMLoadFloat4x4(&boneWorldTransforms[currentBoneIndex]);
			DirectX::XMMATRIX rightMat = XMLoadFloat4x4(&boneWorldTransforms[bone.ParentIndex]);

			DirectX::XMMATRIX ret = leftMat * rightMat;

			XMStoreFloat4x4(&boneWorldTransforms[currentBoneIndex], ret);

			m_isTransformCombined[currentBoneIndex] = true;
		}


		void ResetAnimTimer(int _ind)
		{
			switch (_ind)
			{
			case 0:resetTimerA = true; lastKeyFrameA = 0; break;
			case 1:resetTimerB = true; lastKeyFrameB = 0; break;
			case 2:resetTimerC = true; lastKeyFrameC = 0; break;
			case 3:resetTimerD = true; lastKeyFrameD = 60; break;
			}
		}


		void UpdateAnimationMix(float timeDelta, std::vector<Game::Mesh*>& meshes, float mix_val, float mix_valbb = 0.0f, float mix_valcc = 0.0f, std::wstring clipName1 = L"my_avatar|idle", std::wstring clipName2 = L"my_avatar|run", std::wstring clipName3 = L"my_avatar|walk", std::wstring clipName4 = L"my_avatar|attack")
		{
			
			for (Game::Mesh* mesh : meshes)
			{
				//float new_anim_time;

				//mix_val = mix_val*0.8f;
				//mix_val += 0.1f;
				AnimationState* animState = (AnimationState*)mesh->Tag;
				if (animState == nullptr || mix_val>1.0f || mix_val<0.0f)
				{
					continue;
				}

				if (resetTimerA == true)
				{
					animState->m_animTimeA = 0.0f;
					resetTimerA = false;
				}
				if (resetTimerB == true)
				{
					animState->m_animTimeB = 0.0f;
					resetTimerB = false;
				}
				if (resetTimerC == true)
				{
					animState->m_animTimeC = 0.0f;
					resetTimerC = false;
				}
				if (resetTimerD == true)
				{
					animState->m_animTimeD = 0.0f;
					resetTimerD = false;
				}

				animState->m_animTimeA += timeDelta;
				animState->m_animTimeB += timeDelta;
				animState->m_animTimeC += timeDelta;
				animState->m_animTimeD += timeDelta;
				// Update the bones.
				const std::vector<Game::Mesh::BoneInfo>& skinningInfo = mesh->BoneInfoCollection();
				/*
				for (UINT b = 0; b < skinningInfo.size(); b++)
				{
					//animState->m_boneWorldTransforms[b] = skinningInfo[b].BoneLocalTransform;
					//animState->m_boneWorldTransformsB[b] = skinningInfo[b].BoneLocalTransform;
					//animState->m_boneWorldTransformsC[b] = skinningInfo[b].BoneLocalTransform;
					//animState->m_boneWorldTransformsD[b] = skinningInfo[b].BoneLocalTransform;
				}
				*/

				// Get the keyframes.
				auto& animClips = mesh->AnimationClips();
				auto found1 = animClips.find(clipName1);
				auto found2 = animClips.find(clipName2);
				auto found3 = animClips.find(clipName3);
				auto found4 = animClips.find(clipName4);

				if (found1 != animClips.end() && found2 != animClips.end() && found3 != animClips.end() && found4 != animClips.end())
				{
					const auto& kf1 = found1->second.Keyframes;
					for (int i = lastKeyFrameA; i < kf1.size(); i++)
					{
						if (kf1[i].Time > animState->m_animTimeA)
						{
							lastKeyFrameA = i;
							break;
						}
						animState->m_boneWorldTransforms[kf1[i].BoneIndex] = kf1[i].Transform;
					}

					const auto& kf2 = found2->second.Keyframes;
					for (int i = lastKeyFrameB; i < kf2.size(); i++)
					{
						if (kf2[i].Time > animState->m_animTimeB)
						{
							lastKeyFrameB = i;
							break;
						}
						animState->m_boneWorldTransformsB[kf2[i].BoneIndex] = kf2[i].Transform;
					}

					const auto& kf3 = found3->second.Keyframes;
					for (int i = lastKeyFrameC; i < kf3.size(); i++)
					{
						if (kf3[i].Time > animState->m_animTimeC)
						{
							lastKeyFrameC = i;
							break;
						}
						animState->m_boneWorldTransformsC[kf3[i].BoneIndex] = kf3[i].Transform;
					}

					const auto& kf4 = found4->second.Keyframes;
					for (int i = lastKeyFrameD; i < kf4.size(); i++)
					{
						if (kf4[i].Time > animState->m_animTimeD)
						{
							lastKeyFrameD = i;
							break;
						}
						animState->m_boneWorldTransformsD[kf4[i].BoneIndex] = kf4[i].Transform;
					}

					if (animState->m_animTimeA > found1->second.EndTime)
					{
						lastKeyFrameA = 0;
						animState->m_animTimeA -= found1->second.EndTime;
					}
					if (animState->m_animTimeB > found2->second.EndTime)
					{
						lastKeyFrameB = 0;
						animState->m_animTimeB -= found2->second.EndTime;
					}
					if (animState->m_animTimeC > found3->second.EndTime)
					{
						lastKeyFrameC = 0;
						animState->m_animTimeC -= found3->second.EndTime;
					}
					if (animState->m_animTimeD > found4->second.EndTime)
					{
						//lastKeyFrameD = 0;
						//animState->m_animTimeD -= found4->second.EndTime;
					}


					m_isTransformCombined.assign(skinningInfo.size(), false);
					for (UINT b = 0; b < skinningInfo.size(); b++)
					{
						CombineTransforms(b, skinningInfo, animState->m_boneWorldTransforms);
					}

					m_isTransformCombined.assign(skinningInfo.size(), false);
					for (UINT b = 0; b < skinningInfo.size(); b++)
					{
						CombineTransforms(b, skinningInfo, animState->m_boneWorldTransformsB);
					}

					m_isTransformCombined.assign(skinningInfo.size(), false);
					for (UINT b = 0; b < skinningInfo.size(); b++)
					{
						CombineTransforms(b, skinningInfo, animState->m_boneWorldTransformsC);
					}

					m_isTransformCombined.assign(skinningInfo.size(), false);
					for (UINT b = 0; b < skinningInfo.size(); b++)
					{
						CombineTransforms(b, skinningInfo, animState->m_boneWorldTransformsD);
					}

					if (mix_val == 0.0f || mix_valbb == 1.0f)
					{

					}
					else
					{
						if (mix_val == 1.0f)
						{
							for (int i = 0; i < animState->m_boneWorldTransforms.size(); i++)
							{
								animState->m_boneWorldTransforms[i] = animState->m_boneWorldTransformsB[i];
							}
						}
						else
						{
							for (int i = 0; i < animState->m_boneWorldTransforms.size(); i++)
							{
								DirectX::XMMATRIX mat1 = DirectX::XMLoadFloat4x4(&animState->m_boneWorldTransforms[i]);
								DirectX::XMMATRIX mat2 = DirectX::XMLoadFloat4x4(&animState->m_boneWorldTransformsB[i]);
								DirectX::XMVECTOR vec1 = DirectX::XMQuaternionRotationMatrix(mat1);
								DirectX::XMVECTOR vec2 = DirectX::XMQuaternionRotationMatrix(mat2);
								DirectX::XMVECTOR res = DirectX::XMQuaternionSlerp(vec1, vec2, mix_val);
								DirectX::XMMATRIX matres = DirectX::XMMatrixRotationQuaternion(res);

								//matres = XMMatrixTranspose(matres);

								DirectX::XMFLOAT3X3 floatres;
								DirectX::XMStoreFloat3x3(&floatres, matres);

								animState->m_boneWorldTransforms[i]._11 = floatres._11;
								animState->m_boneWorldTransforms[i]._12 = floatres._12;
								animState->m_boneWorldTransforms[i]._13 = floatres._13;
								animState->m_boneWorldTransforms[i]._21 = floatres._21;
								animState->m_boneWorldTransforms[i]._22 = floatres._22;
								animState->m_boneWorldTransforms[i]._23 = floatres._23;
								animState->m_boneWorldTransforms[i]._31 = floatres._31;
								animState->m_boneWorldTransforms[i]._32 = floatres._32;
								animState->m_boneWorldTransforms[i]._33 = floatres._33;

								animState->m_boneWorldTransforms[i]._41 = flerp(animState->m_boneWorldTransforms[i]._41, animState->m_boneWorldTransformsB[i]._41, mix_val);
								animState->m_boneWorldTransforms[i]._42 = flerp(animState->m_boneWorldTransforms[i]._42, animState->m_boneWorldTransformsB[i]._42, mix_val);
								animState->m_boneWorldTransforms[i]._43 = flerp(animState->m_boneWorldTransforms[i]._43, animState->m_boneWorldTransformsB[i]._43, mix_val);
							}
						}
					}



					if (mix_valbb == 1.0f)
					{
						for (int i = 0; i < animState->m_boneWorldTransforms.size(); i++)
						{
							animState->m_boneWorldTransforms[i] = animState->m_boneWorldTransformsC[i];
						}
					}
					else
					{
						if (mix_valbb > 0.0f)
						{
							for (int i = 0; i < animState->m_boneWorldTransforms.size(); i++)
							{
								DirectX::XMMATRIX mat1 = DirectX::XMLoadFloat4x4(&animState->m_boneWorldTransforms[i]);
								DirectX::XMMATRIX mat2 = DirectX::XMLoadFloat4x4(&animState->m_boneWorldTransformsC[i]);
								DirectX::XMVECTOR vec1 = DirectX::XMQuaternionRotationMatrix(mat1);
								DirectX::XMVECTOR vec2 = DirectX::XMQuaternionRotationMatrix(mat2);
								DirectX::XMVECTOR res = DirectX::XMQuaternionSlerp(vec1, vec2, mix_valbb);
								DirectX::XMMATRIX matres = DirectX::XMMatrixRotationQuaternion(res);

								DirectX::XMFLOAT3X3 floatres;
								DirectX::XMStoreFloat3x3(&floatres, matres);

								animState->m_boneWorldTransforms[i]._11 = floatres._11;
								animState->m_boneWorldTransforms[i]._12 = floatres._12;
								animState->m_boneWorldTransforms[i]._13 = floatres._13;
								animState->m_boneWorldTransforms[i]._21 = floatres._21;
								animState->m_boneWorldTransforms[i]._22 = floatres._22;
								animState->m_boneWorldTransforms[i]._23 = floatres._23;
								animState->m_boneWorldTransforms[i]._31 = floatres._31;
								animState->m_boneWorldTransforms[i]._32 = floatres._32;
								animState->m_boneWorldTransforms[i]._33 = floatres._33;

								animState->m_boneWorldTransforms[i]._41 = flerp(animState->m_boneWorldTransforms[i]._41, animState->m_boneWorldTransformsC[i]._41, mix_valbb);
								animState->m_boneWorldTransforms[i]._42 = flerp(animState->m_boneWorldTransforms[i]._42, animState->m_boneWorldTransformsC[i]._42, mix_valbb);
								animState->m_boneWorldTransforms[i]._43 = flerp(animState->m_boneWorldTransforms[i]._43, animState->m_boneWorldTransformsC[i]._43, mix_valbb);
							}
						}
					}


					if (mix_valcc == 1.0f)
					{
						for (int i = 0; i < animState->m_boneWorldTransforms.size(); i++)
						{
							animState->m_boneWorldTransforms[i] = animState->m_boneWorldTransformsD[i];
						}
					}
					else
					{
						if (mix_valcc > 0.0f)
						{
							for (int i = 0; i < animState->m_boneWorldTransforms.size(); i++)
							{
								DirectX::XMMATRIX mat1 = DirectX::XMLoadFloat4x4(&animState->m_boneWorldTransforms[i]);
								DirectX::XMMATRIX mat2 = DirectX::XMLoadFloat4x4(&animState->m_boneWorldTransformsD[i]);
								DirectX::XMVECTOR vec1 = DirectX::XMQuaternionRotationMatrix(mat1);
								DirectX::XMVECTOR vec2 = DirectX::XMQuaternionRotationMatrix(mat2);
								DirectX::XMVECTOR res = DirectX::XMQuaternionSlerp(vec1, vec2, mix_valcc);
								DirectX::XMMATRIX matres = DirectX::XMMatrixRotationQuaternion(res);

								DirectX::XMFLOAT3X3 floatres;
								DirectX::XMStoreFloat3x3(&floatres, matres);

								animState->m_boneWorldTransforms[i]._11 = floatres._11;
								animState->m_boneWorldTransforms[i]._12 = floatres._12;
								animState->m_boneWorldTransforms[i]._13 = floatres._13;
								animState->m_boneWorldTransforms[i]._21 = floatres._21;
								animState->m_boneWorldTransforms[i]._22 = floatres._22;
								animState->m_boneWorldTransforms[i]._23 = floatres._23;
								animState->m_boneWorldTransforms[i]._31 = floatres._31;
								animState->m_boneWorldTransforms[i]._32 = floatres._32;
								animState->m_boneWorldTransforms[i]._33 = floatres._33;

								animState->m_boneWorldTransforms[i]._41 = flerp(animState->m_boneWorldTransforms[i]._41, animState->m_boneWorldTransformsD[i]._41, mix_valbb);
								animState->m_boneWorldTransforms[i]._42 = flerp(animState->m_boneWorldTransforms[i]._42, animState->m_boneWorldTransformsD[i]._42, mix_valbb);
								animState->m_boneWorldTransforms[i]._43 = flerp(animState->m_boneWorldTransforms[i]._43, animState->m_boneWorldTransformsD[i]._43, mix_valbb);
							}
						}
					}



					for (UINT b = 0; b < skinningInfo.size(); b++)
					{
						DirectX::XMMATRIX leftMat = XMLoadFloat4x4(&skinningInfo[b].InvBindPos);
						DirectX::XMMATRIX rightMat = XMLoadFloat4x4(&animState->m_boneWorldTransforms[b]);
						DirectX::XMMATRIX ret = leftMat * rightMat;
						XMStoreFloat4x4(&animState->m_boneWorldTransforms[b], ret);
					}


				}
			}
		}


		// Updates the animation state for a given time.
		void UpdateAnimation(float timeDelta, std::vector<Game::Mesh*>& meshes, std::wstring clipName = L"Take_002")
		{
			for (Game::Mesh* mesh : meshes)
			{
				AnimationState* animState = (AnimationState*)mesh->Tag;
				if (animState == nullptr)
				{
					continue;
				}

				animState->m_animTime += timeDelta;

				// Update the bones.
				const std::vector<Game::Mesh::BoneInfo>& skinningInfo = mesh->BoneInfoCollection();
				for (UINT b = 0; b < skinningInfo.size(); b++)
				{
					animState->m_boneWorldTransforms[b] = skinningInfo[b].BoneLocalTransform;
				}

				// Get the keyframes.
				auto& animClips = mesh->AnimationClips();
				auto found = animClips.find(clipName);
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
					// Transform to world.
					m_isTransformCombined.assign(skinningInfo.size(), false);

					for (UINT b = 0; b < skinningInfo.size(); b++)
					{
						CombineTransforms(b, skinningInfo, animState->m_boneWorldTransforms);
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
		void RenderSkinnedMesh(Game::Mesh* mesh, AllResources& graphics)
		{
			ID3D11DeviceContext* deviceContext = graphics.m_deviceResources->GetD3DDeviceContext();

			BOOL supportsShaderResources = graphics.m_deviceResources->GetDeviceFeatureLevel() >= D3D_FEATURE_LEVEL_10_0;
			/*
			DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&graphics.m_Camera->m_constantBufferData.view);
			DirectX::XMMATRIX projection = XMLoadFloat4x4(&graphics.m_Camera->m_constantBufferData.projection);// *DirectX::XMLoadFloat4x4(&graphics.m_deviceResources->GetOrientationTransform3D());

			//view = XMMatrixInverse(nullptr,view);
			world = XMMatrixTranspose(world);
			view = XMMatrixTranspose(view);// 
			projection = XMMatrixTranspose(projection);
			projection = projection *DirectX::XMLoadFloat4x4(&graphics.m_deviceResources->GetOrientationTransform3D());


			// Compute the object matrices.
			DirectX::XMMATRIX localToView = world * view;
			DirectX::XMMATRIX localToProj = world * view * projection * XMMatrixTranspose(XMLoadFloat4x4(&graphics.m_deviceResources->GetOrientationTransform3D()));;

			// Initialize object constants and update the constant buffer.
			Game::ObjectConstants objConstants;
			objConstants.LocalToWorld4x4 = DirectX::XMMatrixTranspose(world);
			objConstants.LocalToProjected4x4 = DirectX::XMMatrixTranspose(localToProj);
			objConstants.WorldToLocal4x4 = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, world));
			objConstants.WorldToView4x4 = DirectX::XMMatrixTranspose(view);
			objConstants.UvTransform4x4 = DirectX::XMMatrixIdentity();
			objConstants.EyePosition = graphics.m_Camera->GetEye();
			graphics.UpdateObjectConstants(objConstants);
			*/
			// Assign constant buffers to correct slots.
			//ID3D11Buffer* constantBuffer = graphics.GetLightConstants();
			//deviceContext->VSSetConstantBuffers(1, 1, &constantBuffer);
			//deviceContext->PSSetConstantBuffers(1, 1, &constantBuffer);

			//constantBuffer = graphics.GetMiscConstants();
			//deviceContext->VSSetConstantBuffers(3, 1, &constantBuffer);
			//deviceContext->PSSetConstantBuffers(3, 1, &constantBuffer);

			ID3D11Buffer* boneConsts = m_boneConstantBuffer.Get();
			deviceContext->VSSetConstantBuffers(2, 1, &boneConsts);

			ID3D11Buffer* constantBuffer = graphics.GetObjectConstants();
			deviceContext->VSSetConstantBuffers(1, 1, &constantBuffer);
			deviceContext->PSSetConstantBuffers(1, 1, &constantBuffer);

			// Prepare to draw.

			// NOTE: Set the skinning vertex layout.
			//deviceContext->IASetInputLayout(m_skinningVertexLayout.Get());
			//deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			BoneConstants boneConstants;

			// Update the bones.
			AnimationState* animState = (AnimationState*)mesh->Tag;
			if (animState != nullptr)
			{
				// Copy to constants.
				for (UINT b = 0; b < animState->m_boneWorldTransforms.size(); b++)
				{
					DirectX::XMMATRIX bones = DirectX::XMMatrixTranspose((XMLoadFloat4x4(&animState->m_boneWorldTransforms[b])));
					XMStoreFloat4x4(&boneConstants.Bones[b], bones);
				}
			}

			// Update the constants.
			deviceContext->UpdateSubresource(m_boneConstantBuffer.Get(), 0, nullptr, &boneConstants, 0, 0);

			// Loop over each submesh.
			for (const Game::Mesh::SubMesh& submesh : mesh->SubMeshes())
			{
				// Only draw submeshes that have valid materials.
				Game::MaterialConstants materialConstants;

				if (submesh.IndexBufferIndex < mesh->IndexBuffers().size() &&
					submesh.VertexBufferIndex < mesh->VertexBuffers().size())
				{
					ID3D11Buffer* vbs[2] =
					{
						mesh->VertexBuffers()[submesh.VertexBufferIndex],
						mesh->SkinningVertexBuffers()[submesh.VertexBufferIndex]
					};

					UINT stride[2] = { sizeof(Game::Vertex), sizeof(Game::SkinningVertexInput) };
					UINT offset[2] = { 0, 0 };
					deviceContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
					deviceContext->IASetIndexBuffer(mesh->IndexBuffers()[submesh.IndexBufferIndex], DXGI_FORMAT_R16_UINT, 0);
				}
				
				//deviceContext->UpdateSubresource(materialBuffer.Get(), 0, nullptr, &submesh.m_material, 0, 0);

				if (submesh.MaterialIndex < mesh->Materials().size())
				{
					const Game::Mesh::Material& material = mesh->Materials()[submesh.MaterialIndex];



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
					
					for (UINT tex = 0; tex < Game::Mesh::MaxTextures; tex++)
					{
						ID3D11ShaderResourceView* shaderResourceView = material.Textures[tex].Get();
						if (supportsShaderResources)
						{
							deviceContext->VSSetShaderResources(0 + tex, 1, &shaderResourceView);
							deviceContext->VSSetShaderResources(Game::Mesh::MaxTextures + tex, 1, &shaderResourceView);
						}

						deviceContext->PSSetShaderResources(0 + tex, 1, &shaderResourceView);
						deviceContext->PSSetShaderResources(Game::Mesh::MaxTextures + tex, 1, &shaderResourceView);
					}
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
					deviceContext->DrawIndexed(submesh.PrimCount * 3, submesh.StartIndex+3, 0);
				}
			}

			// Clear the extra vertex buffer.
			ID3D11Buffer* vbs[1] = { nullptr };
			UINT stride = 0;
			UINT offset = 0;
			deviceContext->IASetVertexBuffers(1, 1, vbs, &stride, &offset);
		}

	private:
		//Microsoft::WRL::ComPtr<ID3D11VertexShader> m_skinningShader;
		//Microsoft::WRL::ComPtr<ID3D11InputLayout> m_skinningVertexLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_boneConstantBuffer;
		std::vector<bool> m_isTransformCombined;
	};
}
