// These are our deferred HLSL shaders.We use these to render the scene data into render to texture buffers.For the deferred vertex shader it still does the exact same thing as in previous tutorials.All the changes are actually in the pixel shader.

////////////////////////////////////////////////////////////////////////////////
// Filename: deferred.vs
////////////////////////////////////////////////////////////////////////////////
#include "DefShader.h"

//////////////////////
// CONSTANT BUFFERS //
//////////////////////
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

cbuffer MaterialBuffer : register(b4)
{
	float4 MaterialAmbient;
	float4 MaterialDiffuse;
	float4 MaterialSpecular;
	float4 MaterialEmissive;
	float MaterialSpecularPower;
	float normal_height;
	float spare1mat;
	float spare2mat;

};



//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
	float4 position : POSITION0;
	float3 normal : NORMAL0;
	float4 tangent : TANGENT0;
	float4 color : COLOR0;
	float2 tex : TEXCOORD0;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 positionW : TEXCOORD2;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float cam_dist : TEXCOORD3;
	
	float4 depthPosition : TEXTURE0B;

	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType main(VertexInputType input)
{
	PixelInputType output;


	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	output.positionW = mul(input.position, worldMatrix).xyz;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.depthPosition = output.position;
	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	// Calculate the normal vector against the world matrix only.
	output.normal = mul(input.normal, (float3x3)worldMatrix);

	// Normalize the normal vector.
	output.normal = normalize(output.normal);

	output.cam_dist = length(eye_position - output.positionW);


	if (normal_height > 0.0f)
	{
		output.tangent = mul(input.tangent, (float3x3)worldMatrix);
		//output.tangent = normalize(output.tangent);

		output.binormal = cross(input.normal, input.tangent);
		output.binormal = mul(output.binormal, (float3x3)worldMatrix);
		//output.binormal = normalize(output.binormal);
	}
	return output;
}
