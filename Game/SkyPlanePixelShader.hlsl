////////////////////////////////////////////////////////////////////////////////
// Filename: skyplane.ps
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
Texture2D cloudTexture : register(t0);
Texture2D perturbTexture : register(t1);
SamplerState SampleType;

cbuffer SkyBuffer : register(b8)
{
	float translationx;
	float translationz;
	float scale;
	float brightness;
	float padding;
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

//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float2 alpha : TEXCOORD1;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
	float4 perturbValue;
	float4 cloudColor;


	// Translate the texture coordinate sampling location by the translation value.
	input.tex.x = input.tex.x + translationx;
	input.tex.y = input.tex.y + translationz;

	// Sample the texture value from the perturb texture using the translated texture coordinates.
	perturbValue = perturbTexture.Sample(SampleType, input.tex);

	// Multiply the perturb value by the perturb scale.
	perturbValue = perturbValue * scale;

	// Add the texture coordinates as well as the translation value to get the perturbed texture coordinate sampling location.
	perturbValue.xy = perturbValue.xy + input.tex.xy + translationx;

	//perturbValue.xy = perturbValue.xy + input.tex.y + translationz;

	// Now sample the color from the cloud texture using the perturbed sampling coordinates.
    cloudColor = cloudTexture.Sample(SampleType, perturbValue.xy);

	// Reduce the color cloud by the brightness value.
	cloudColor = cloudColor * brightness;

	cloudColor.w *= input.alpha.x;

	return cloudColor +((ambientColor + diffuseColor)*0.3f);
}