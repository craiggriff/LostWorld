////////////////////////////////////////////////////////////////////////////////
// Filename: light.ps
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
#define MAX_POINT_LIGHTS 10
#define MAX_VERTEX_POINT_LIGHTS 20

Texture2D shaderTexture : register(t0);
Texture2D shaderTexture2 : register(t1);
SamplerState SampleType : register(s0);


struct PointLight
{
	float radius;
	float3 pos;
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 dir;
	float spot;
};


cbuffer LightBuffer : register(b0)
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightDirection;
	float specularPower;
	float4 specularColor;
	float4 fogColor;
	int numLights;
	float3 pos;

	PointLight lights[MAX_POINT_LIGHTS];

};


cbuffer MaterialBuffer : register(b4)
{
	float4 vertexCol; // for glowing
	float4 specularCol;
	float specularPow;
	float specularLvl;
	float diffuseLvl;
	float ambientLvl;
};




//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 viewDirection : TEXCOORD1;
	float3 positionW : TEXCOORD2;
	float cam_dist : TEXCOORD3;
	float4 color : COLOR;
};



float attenu(float distance, float range, float a, float b, float c)
{
	float atten = 1.0f / ((a * distance * distance) + (b * distance) + c);

	//step clamps to 0 if out of range
	return step(distance, range) * saturate(atten);
}

const float a_a = 0.0f;
const float a_b = 0.1f;
const float a_c = 1.0f;
////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{

	float4 textureColor;
	float3 lightDir;
	float lightIntensity;
	float4 outcolor;
	float3 reflection;
	float4 specular;

	float3 n = normalize(input.normal);
	float3 v = normalize(input.viewDirection);
	float3 l = float3(0.0f, 0.0f, 0.0f);
	float3 h = float3(0.0f, 0.0f, 0.0f);

	float atten = 0.0f;
	float nDotL = 0.0f;
	float nDotH = 0.0f;
	float power = 0.0f;



	textureColor = shaderTexture.Sample(SampleType, input.tex);



	if (input.color.a == 0.919f)
	{
		input.color.a = 1.0f;
		textureColor = lerp(textureColor, shaderTexture2.Sample(SampleType, input.tex), input.color.r*2.0f);

	}

	// Set the default output color to the ambient light value for all pixels.
	outcolor = ambientColor;

	// Initialize the specular color.
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Invert the light direction for calculations.
	lightDir = -lightDirection;

	// Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(input.normal, lightDir));

	if (lightIntensity > 0.0f)
	{
		// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
		outcolor += (diffuseColor * lightIntensity);

		// Saturate the ambient and diffuse color.
		outcolor = saturate(outcolor);

		if (false)//(specularPower != 0.0f)
		{
			// Calculate the reflection vector based on the light intensity, normal vector, and light direction.
			reflection = normalize(2 * lightIntensity * input.normal - lightDir);

			// Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
			specular = specularColor*pow(saturate(dot(reflection, input.viewDirection)), specularPower);
		}
	}




	float s = input.cam_dist;
	//s = saturate(100.0f - s);

	//color = lerp(color, float4(0.5f,0.5f,0.5f,0.0f), s/10.0f);

	// Multiply the texture pixel and the input color to get the textured result.
	outcolor = outcolor * (textureColor + input.color);


	if (false)//(specularPower != 0.0f)
	{
		outcolor = saturate(outcolor + specular);
	}

	//if (input.cam_dist > 50.0f)
	//{
	//color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (false)//(fogColor.a>0.0f)
	{
		float al = outcolor.a; // fog doesnt effect alpha
		outcolor = lerp(outcolor, fogColor, input.cam_dist / 100.0f);
		outcolor.a = al;
	}
	//}

	// Add the specular component last to the output color.
	return outcolor;


}