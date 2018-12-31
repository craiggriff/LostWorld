////////////////////////////////////////////////////////////////////////////////
// Filename: particle.vs
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



//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
	float4 pos : POSITION;
	float4 col : COLOR;
	float2 tex : TEXCOORD;
	float2 padding : PADDING;
	float3 position : POSITIONI;
	float4 color : COLORI;
	float2 paddingb : PADDINGI;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType main(VertexInputType input)
{
	PixelInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
	input.pos.w = 1.0f;


	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.pos, worldMatrix);

	output.position.x += input.position.x;
	output.position.y += input.position.y;
	output.position.z += input.position.z;

	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);



	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	//input.color.w = 1.0f;
	output.color = input.color;// *ambientColor*(diffuseColor*0.5);
	//output.color.w = 1.0f;










	// Store the particle color for the pixel shader. 
	

	return output;
}