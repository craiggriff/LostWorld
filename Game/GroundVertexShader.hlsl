////////////////////////////////////////////////////////////////////////////////
// Filename: light.vs
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



cbuffer ObjectVars : register(b1)
{
	float4x4 LocalToWorld4x4;
	float4x4 LocalToProjected4x4;
	float4x4 WorldToLocal4x4;
	float4x4 WorldToView4x4;
	float4x4 UVTransform4x4;
	float3 EyePosition;

}


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


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
	float4 position : POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD;
	float tex_blend : TEXBLEND;
	float tex_blend2 : TEXBLENDB;
	float tex_blend3 : TEXBLENDC;
	float tex_blend4 : TEXBLENDD;
	float tex_blend5 : TEXBLENDE;
	float tex_blend6 : TEXBLENDF;
	float tex_blend7 : TEXBLENDG;
	float tex_blend8 : TEXBLENDH;
	float4 color : COLOR;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;

};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float3 viewDirection : TEXCOORD1;
	float3 positionW : TEXCOORD2;
	float cam_dist : TEXCOORD3;
	float tex_blend : TEXBLEND1;
	float tex_blend2 : TEXBLEND2;
	float tex_blend3 : TEXBLEND3;
	float tex_blend4 : TEXBLEND4;
	float tex_blend5 : TEXBLEND5;
	float tex_blend6 : TEXBLEND6;
	float tex_blend7 : TEXBLEND7;
	float tex_blend8 : TEXBLEND8;

	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;



};

float VectorToDepth(float3 Vec)
{
	float3 AbsVec = abs(Vec);
	float LocalZcomp = max(AbsVec.x, max(AbsVec.y, AbsVec.z));

	// Replace f and n with the far and near plane values you used when
	//   you drew your cube map.
	const float f = 17.0f;
	const float n = 0.1f;

	float NormZComp = (f + n) / (f - n) - (2 * f*n) / (f - n) / LocalZcomp;
	return (NormZComp + 1.0) * 0.5;
}

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType main(VertexInputType input)
{
	PixelInputType output;


	input.position.w = 1.0f;
	// Change the position vector to be 4 units for proper matrix calculations.
	
	// Calculate the position of the vertex against the world, view, and projection matrices.

	/*
	output.position = mul(input.position, worldMatrix);
	output.positionW = output.position.xyz;

	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	*/
	output.positionW = mul(input.position, worldMatrix).xyz;
	output.position = mul(input.position, mvp);



	//output.positionW = mul(float4(input.position.xyz, 1), worldMatrix).xyz;
	

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;
	
	// Calculate the normal vector against the world matrix only.
	//output.normal =  mul(input.normal, (float3x3)worldMatrix);

	output.normal = input.normal;
	output.tangent = input.tangent;
	output.binormal = input.binormal;

	// Calculate the tangent vector against the world matrix only and then normalize the final value.
	//output.tangent = mul(input.tangent, (float3x3)worldMatrix);
	//output.tangent = normalize(output.tangent);

	// Calculate the binormal vector against the world matrix only and then normalize the final value.
	//output.binormal = mul(input.binormal, (float3x3)worldMatrix);
	//output.binormal = normalize(output.binormal);
	
	//output.normal = normalize(output.normal);

	//output.positionW = mul(float4(input.position.xyz, 1), worldMatrix).xyz;

	// Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
	//output.viewDirection = transpose(viewMatrix)[3].xyz - output.position;
	output.viewDirection = eye_position - output.positionW.xyz;

	output.cam_dist = length(output.viewDirection);

	output.viewDirection = normalize(output.viewDirection);

	output.tex_blend = input.tex_blend;
	output.tex_blend2 = input.tex_blend2;
	output.tex_blend3 = input.tex_blend3;
	output.tex_blend4 = input.tex_blend4;
	output.tex_blend5 = input.tex_blend5;
	output.tex_blend6 = input.tex_blend6;
	output.tex_blend7 = input.tex_blend7;
	output.tex_blend8 = input.tex_blend8;


	output.color = ambientColor;// input.color;// ambientColor;// +output.lightness;


	float lightIntensity;

	float3 l = float3(0.0f, 0.0f, 0.0f);

	float atten = 0.0f;
	float nDotL = 0.0f;
	float nDotH = 0.0f;


	float3 n = output.normal;
	float3 v = output.viewDirection;



	output.color = saturate(output.color);



	return output;
}