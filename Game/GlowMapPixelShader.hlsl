////////////////////////////////////////////////////////////////////////////////
// Filename: glowmap.ps
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture : register(t0);
Texture2D glowMapTexture : register(t1);
Texture2D glowMapOriginalTexture : register(t2);
SamplerState SampleType;


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 mix(float4 col1, float4 col2)
{
	float4 add = col2*0.5f;
	float4 m = max(col1, col2);

	return add + m;
}



////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
	float4 glowMap;
	float4 glowMapOriginal;
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	// Sample the glow map.
	glowMap = glowMapTexture.Sample(SampleType, input.tex);

	glowMapOriginal = glowMapOriginalTexture.Sample(SampleType, input.tex);

	if (glowMap.a > 0.0f)
	{
		textureColor.r = mix(textureColor.r, glowMap.r);
		textureColor.g = mix(textureColor.g, glowMap.g);
		textureColor.b = mix(textureColor.b, glowMap.b);
	}

	return textureColor;
}
