////////////////////////////////////////////////////////////////////////////////
// Filename: light.ps
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
#include "DefShader.h"

Texture2D shaderTexture : register(t0);

SamplerState SampleType : register(s0);


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
	float4 textureColor;


	textureColor = shaderTexture.Sample(SampleType, input.tex);

	//if (false)//(textureColor.a < 0.2f)
	//	discard;

	return textureColor;
}

