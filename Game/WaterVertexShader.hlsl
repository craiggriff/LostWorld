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
	float tex_blend : TEXCOORD4;
	float tex_blend2 : TEXCOORD5;

	float3 tangent : NORMAL2;
	float3 binormal : NORMAL3;


};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType main(VertexInputType input)
{
	PixelInputType output;

	float3 positionW;
	//float4 spec_colour

	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;
	// Calculate the position of the vertex against the world, view, and projection matrices.
	/*
	output.position = mul(input.position, worldMatrix);
	positionW = output.position.xyz;
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	*/
	output.positionW = mul(input.position, worldMatrix).xyz;
	output.position = mul(input.position, mvp);

	// Store the texture coordinates for the pixel shader.
	//input.tex.x = input.tex.x + (positionW.y*0.3f);
	output.tex = input.tex;

	// Calculate the normal vector against the world matrix only.
	//output.normal = mul(input.normal, (float3x3)worldMatrix);/*input.normal*/// 
	output.normal = input.normal;

	/*
	float3 tangent;
	float3 binormal;

	float3 c1 = cross(output.normal, float3(0.0, 0.0, 1.0));
	float3 c2 = cross(output.normal, float3(0.0, 1.0, 0.0));

	if (length(c1)>length(c2))
	{
		tangent = c1;
	}
	else
	{
		tangent = c2;
	}

	output.tangent = normalize(tangent);

	binormal = cross(output.normal, output.tangent);
	*/
	output.binormal = input.binormal;
	output.binormal = input.tangent;




	output.positionW = mul(float4(input.position.xyz, 1), worldMatrix).xyz;

	// Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
	output.viewDirection = eye_position -output.positionW;// -output.position;

	output.cam_dist = length(output.viewDirection);

	output.viewDirection = normalize(output.viewDirection);


	output.tex_blend = input.tex_blend;
	output.tex_blend2 = input.tex_blend2;


	output.color = ambientColor;


	float lightIntensity;

	float3 l = float3(0.0f, 0.0f, 0.0f);
	float3 h = float3(0.0f, 0.0f, 0.0f);

	float atten = 0.0f;
	float nDotL = 0.0f;
	float nDotH = 0.0f;
	float power = 0.0f;
	float3 reflection;

	float3 n = output.normal;
	float3 v = output.viewDirection;


	
	//lightIntensity = saturate(dot(output.normal, -lightDirection));
	/*
	if (lightIntensity > 0.0f)
	{
		// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
		output.color += ((diffuseColor)* lightIntensity);

		//power = (lightIntensity == 0.0f) ? 0.0f : pow(nDotH, specularPower);


		// Saturate the ambient and diffuse color.
		output.color = saturate(output.color);

	}
	*/
	/*
	if (lightIntensity > 0.0f)
	{
		// Calculate the reflection vector based on the light intensity, normal vector, and light direction.
		reflection = normalize(2 * lightIntensity * output.normal - lightDirection);

		// Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
		output.color += (specularColor)*pow(saturate(dot(reflection, output.viewDirection)), 50.0f)*100.0f;

		//specular = 0.0f;// specular * specularLvl;
	}
	*/

	/*
	for (int i = 0; i < numLights; ++i)
	{
		if (lights[i].spot==0.0f)
		{
			l = (lights[i].pos - output.positionW.xyz);
			float distance = length(l);

			if (distance < lights[i].radius)
			{
				atten = (distance / lights[i].radius);//    saturate(1.0f - dot(l, l));

				//atten = attenu(distance(input.positionW, lights[i].pos), 3.0f, a_a, a_b, a_c);
				l = normalize(l);
				h = normalize(l + v);

				lightIntensity = 1.0f;// saturate(dot(input.normal, l));

				nDotL = saturate(dot(n, l));
				nDotH = saturate(dot(n, h));
				power = (nDotL == 0.0f) ? 0.0f : pow(nDotH, specularPower);

				output.color += (lights[i].diffuse * nDotL * atten); // +(specularColor * lights[i].specular * power * atten);// *10.0f;// (ambientColor * ((atten * lights[i].ambient)));// +(diffuseColor * lights[i].diffuse * nDotL * atten);// +(specularColor * lights[i].specular * power * atten);
			}
		}
	}
	*/







	return output;
}