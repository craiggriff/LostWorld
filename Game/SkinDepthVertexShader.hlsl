////////////////////////////////////////////////////////////////////////////////
// Filename: depth.vs
////////////////////////////////////////////////////////////////////////////////

#include "DefShader.h"




/////////////
// GLOBALS //
/////////////
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


cbuffer BoneVars : register(b2)
{
	float4x4 Bones[MAX_BONES];
};



struct VertexInputType
{
	float4 position : POSITION;
	float3 normal : NORMAL0;
	float4 tangent : TANGENT0;
	float4 color : COLOR0;
	float2 uv : TEXCOORD0;
	uint4 boneIndices : BLENDINDICES0;
	float4 blendWeights : BLENDWEIGHT0;
};

//For input into the pixel shader we will need the vertex position in homogeneous clip space stored in the position variable as usual.However since the SV_POSITION semantic offsets the position by 0.5 we will need a second set of coordinates called depthPosition that are not modified so that we can perform depth calculations.

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 depthPosition : TEXTURE0;

};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType main(VertexInputType input)
{
	PixelInputType output;


	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;


	float4x4 skinTransform = Bones[input.boneIndices.x] * input.blendWeights.x;
	skinTransform += Bones[input.boneIndices.y] * input.blendWeights.y;
	skinTransform += Bones[input.boneIndices.z] * input.blendWeights.z;
	skinTransform += Bones[input.boneIndices.w] * input.blendWeights.w;

	float4 skinnedPos = mul(input.position, skinTransform);



	// Calculate the position of the vertex against the world, view, and projection matrices.
	//output.position = mul(skinnedPos, worldMatrix);
	//output.position = mul(output.position, dirView);
	//output.position = mul(output.position, dirProj);

	output.position = mul(skinnedPos, mvp);


	//As described in the pixel shader input we store a second pair of position coordinates that will not be offset which will be used for depth calculations.

	// Store the position value in a second input value for depth value calculations.
	output.depthPosition = output.position;

	return output;
}
