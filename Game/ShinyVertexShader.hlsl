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
	float4 position : POSITION0;
	float3 normal : NORMAL0;
	float4 tangent : TANGENT0;
	float4 color : COLOR0;
	float2 tex : TEXCOORD0;
	//float3 binormal : BINORMAL;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float3 viewDirection : NORMAL1;
	float3 positionW : TEXCOORD2;
	float cam_dist : TEXCOORD3;

	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;


};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType main(VertexInputType input)
{
	PixelInputType output;

	float3 lightDir = -lightDirection;

	// Change the position vector to be 4 units for proper matrix calculations.

	input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	/*
	output.position = mul(input.position, worldMatrix);
	output.positionW = output.position.xyz;
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	*/
	output.positionW = mul(input.position, worldMatrix).xyz;

	output.position = mul(input.position, mvp);
	


	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	//output.tex = mul(float4(input.tex.x, input.tex.y, 0, 1), (float4x2)UVTransform4x4).xy;

	// Calculate the normal vector against the world matrix only.
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);

	if (normal_height > 0.0f)
	{
		output.tangent = mul(input.tangent, (float3x3)worldMatrix);
		//output.tangent = normalize(output.tangent);

		output.binormal = cross(input.normal, input.tangent);
		output.binormal = mul(output.binormal, (float3x3)worldMatrix);
		//output.binormal = normalize(output.binormal);
	}
	// Calculate the binormal vector against the world matrix only and then normalize the final value.
	//output.binormal = mul(input.binormal, (float3x3)worldMatrix);
	//output.binormal = normalize(output.binormal);

	//output.positionW = mul(float4(input.position.xyz, 1), worldMatrix).xyz;

	// Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
	output.viewDirection = eye_position - output.positionW;
	output.cam_dist = length(output.viewDirection);
	output.viewDirection = normalize(output.viewDirection);



	output.color = ((ambientColor * MaterialAmbient)*2.0f);


	float lightIntensity;

	float3 l = float3(0.0f, 0.0f, 0.0f);


	float atten = 0.0f;
	float nDotL = 0.0f;

	float power = 0.0f;

	float3 n = output.normal;
	float3 v = output.viewDirection;



	if (normal_height == 0)
	{
		lightIntensity = saturate(dot(output.normal, lightDir));

		if (lightIntensity > 0.0f)
		{
			// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
			output.color += ((diffuseColor * MaterialDiffuse)* lightIntensity);

			// Saturate the ambient and diffuse color.
			output.color = saturate(output.color);
		}
	}


	//output.color.a = input.color.a;




	return output;
}