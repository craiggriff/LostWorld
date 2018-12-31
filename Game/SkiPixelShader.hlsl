#include "DefShader.h"

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

TextureCube environmentMapCube : register(t5);




struct PointLight
{
	float radius;
	float3 pos;
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 dir;
	float spot;
	float specular_power;
	int shadow_cube_index;
	float spare;
	int lightmap;
	matrix dirView; // directional light view and projection
	matrix dirProj;
};



cbuffer LightBuffer : register(b0)
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

struct PixelShaderInput
{
	float4 position : SV_POSITION;

	float2 tex : TEXCOORD0;
};


float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 textureColor;


// Sample the pixel color from the texture using the sampler at this texture coordinate location.
textureColor = shaderTexture.Sample(SampleType, input.tex);

if (false)//(fogColor.a>0.0f)
{
	//float al = outcolor.a; // fog doesnt effect alpha
	//outcolor = lerp(outcolor, fogColor, input.cam_dist / 100.0f);
	//outcolor.a = al;

	textureColor = lerp(textureColor, fogColor, 0.8f);  
}

return textureColor;
}


