////////////////////////////////////////////////////////////////////////////////
// Filename: particle.ps
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture : register(t0);

Texture2D normalTexture : register(t3);

SamplerState SampleType : register(s0);


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



struct PixelOutputType
{
	float4 color : SV_Target0;
	float4 normal : SV_Target1;
	float4 position : SV_Target2;
	float4 specular : SV_Target3;
};


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 positionW : TEXCOORD2A;
	float2 tex : TEXCOORD0;
	float4 color : COLOR;
	float3 cam_dir : TEXCOORD3A;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
PixelOutputType main(PixelInputType input) : SV_TARGET
{
	PixelOutputType output;



	float4 tex_sam = shaderTexture.Sample(SampleType, input.tex);// *input.color;

	if (tex_sam.w < 0.1f)
	{
		discard;
	}

	output.color = tex_sam *input.color;
	output.normal = float4(input.cam_dir,0.7f);

	output.position = float4(input.positionW, 1.0f);

	output.specular = float4(MaterialSpecular.x*MaterialSpecular.w, MaterialSpecular.y*MaterialSpecular.w, MaterialSpecular.z*MaterialSpecular.w, MaterialSpecularPower);

	return output;

}