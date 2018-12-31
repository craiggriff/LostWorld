// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//----------------------------------------------------------------------

#include "DefShader.h"


cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix mvp;
};


cbuffer VLightBuffer : register(b5)
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightDirection;
	float specularPower;
	float4 specularColor;
	float4 fogColor;

	float3 eye_position;
	uint numPointLights;
	uint numSpotLights;
	float lightning;
	float3 lightning_dir;
	float3 pos;



};


/*
cbuffer MaterialVars : register (b0)
{
	float4 MaterialAmbient;
	float4 MaterialDiffuse;
	float4 MaterialSpecular;
	float4 MaterialEmissive;
	float MaterialSpecularPower;
};
*/
/*
cbuffer LightVars : register (b1)
{
	float4 AmbientLight;
	float4 LightColor[4];
	float4 LightAttenuation[4];
	float3 LightDirection[4];
	float LightSpecularIntensity[4];
	uint IsPointLight[4];
	uint ActiveLights;
}
*/
cbuffer ObjectVars : register(b1)
{
	float4x4 LocalToWorld4x4;
	float4x4 LocalToProjected4x4;
	float4x4 WorldToLocal4x4;
	float4x4 WorldToView4x4;
	float4x4 UVTransform4x4;
	float3 EyePosition;
};

cbuffer MaterialBuffer : register(b4)
{
	float4 MaterialAmbient;
	float4 MaterialDiffuse;
	float4 MaterialSpecular;
	float4 MaterialEmissive;
	float MaterialSpecularPower;
	float normal_height;
	float spare1;
	float spare2;

};

/*
cbuffer MiscVars : register(b3)
{
	float ViewportWidth;
	float ViewportHeight;
	float Time;
};
*/
cbuffer BoneVars : register(b2)
{
	float4x4 Bones[MAX_BONES];
};

struct A2V
{
	float4 pos : POSITION0;
	float3 normal : NORMAL0;
	float4 tangent : TANGENT0;
	float4 color : COLOR0;
	float2 uv : TEXCOORD0;
	uint4 boneIndices : BLENDINDICES0;
	float4 blendWeights : BLENDWEIGHT0;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 positionW : TEXCOORD2;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float cam_dist : TEXCOORD3;

	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;

};

PixelInputType main(A2V vertex)
{
	PixelInputType result;
	float3 l = float3(0.0f, 0.0f, 0.0f);
	float atten = 0.0f;
	float lightIntensity;
	float nDotL = 0.0f;
	float3 lightDir = -lightDirection;

	float4x4 skinTransform = Bones[vertex.boneIndices.x] * vertex.blendWeights.x;
	skinTransform += Bones[vertex.boneIndices.y] * vertex.blendWeights.y;
	skinTransform += Bones[vertex.boneIndices.z] * vertex.blendWeights.z;
	skinTransform += Bones[vertex.boneIndices.w] * vertex.blendWeights.w;

	//vertex.pos.y += 0.5f;

	float4 skinnedPos = mul(vertex.pos, skinTransform);
	float3 skinnedNorm = mul(vertex.normal, (float3x3)skinTransform);
	//float3 skinnedTan = mul((float3)vertex.tangent, (float3x3)skinTransform);

	//skinnedPos.y= skinnedPos+0.7f;

	// transform point into world space (for eye vector)
	//float3 wp = mul(skinnedPos, LocalToWorld4x4).xyz;

	// set output data
	result.position = mul(skinnedPos, mvp);

	result.positionW = mul(skinnedPos, worldMatrix).xyz;

	// Store the texture coordinates for the pixel shader.
	result.tex = vertex.uv;

	// Calculate the normal vector against the world matrix only.
	//result.normal = mul(skinnedNorm, (float3x3)worldMatrix);
	result.normal = normalize(mul(skinnedNorm, (float3x3)worldMatrix));
	// Normalize the normal vector.
	//result.normal = normalize(result.normal);

	result.cam_dist = length(eye_position - result.positionW);


	if (normal_height > 0.0f)
	{
		result.tangent = mul(vertex.tangent, (float3x3)worldMatrix);
		//output.tangent = normalize(output.tangent);

		result.binormal = cross(skinnedNorm, vertex.tangent);
		result.binormal = mul(result.binormal, (float3x3)worldMatrix);
		//output.binormal = normalize(output.binormal);
	}


	
	return result;
}