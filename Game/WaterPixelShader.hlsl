////////////////////////////////////////////////////////////////////////////////
// Filename: light.ps
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
#include "DefShader.h"


Texture2D shaderTexture : register(t0);
Texture2D shaderTexture2 : register(t1);
Texture2D shaderTexture3 : register(t2);
Texture2D normalTexture : register(t3);
Texture2D normalTexture2 : register(t4);
//Texture2D depthTexture : register(t4);

SamplerState SampleType : register(s0);

TextureCube environmentMapCube : register(t5);

Texture2DArray spotshadowMapArray[MAX_SPOT_SHADOWS] : register(t100);

TextureCubeArray pointshadowMapCubeArray[MAX_POINT_SHADOWS] : register(t60);

Texture2DArray lightmapArray[8] : register(t40);

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

StructuredBuffer<PointLight> pointLights : register(t50);
StructuredBuffer<PointLight> spotLights : register(t51);

SamplerState samTriLinearSam {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};


SamplerState samPointSam {
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};

cbuffer SkyBuffer : register(b8)
{
	float translationx;
	float translationz;
	float scale;
	float brightness;
	float _padding;
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

//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float3 viewDirection : TEXCOORD1;
	float3 positionW : TEXCOORD2;
	float cam_dist : TEXCOORD3;

	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;

	float4 viewPosition : TEXCOORD4;
};

float VectorToDepth(float3 Vec)
{
	float3 AbsVec = abs(Vec);
	float LocalZcomp = max(AbsVec.x, max(AbsVec.y, AbsVec.z));

	// Replace f and n with the far and near plane values you used when
	//   you drew your cube map.
	const float f = LIGHT_RADIUS;
	const float n = 0.1f;

	float NormZComp = (f + n) / (f - n) - (2 * f*n) / (f - n) / LocalZcomp;
	return (NormZComp + 1.0) * 0.5;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{

	float4 textureColor;

float lightIntensity;
float lightIntensityDirectional;

float4 outcolor;
float3 reflection;
float4 specular;
float4 specular_point;

float3 lightDir = -lightDirection;

float3 n = input.normal;
float3 v = input.viewDirection;
float3 l = float3(0.0f, 0.0f, 0.0f);
float3 h = float3(0.0f, 0.0f, 0.0f);

float atten = 0.0f;
float nDotL = 0.0f;
float nDotH = 0.0f;
float power = 0.0f;
float4 normalMap;
float4 normalMap2;

float2 projectTexCoord;
float4 projectionColor;
float lightDepthValue;

float depthValue;
float bias = 0.001f;

float4 outputColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
float4 viewPosition;
float4 lightmapValue;


//textureColor = shaderTexture.Sample(SampleType, input.tex);


textureColor = shaderTexture.Sample(SampleType, float2(input.tex.x + (translationx*10.0f), input.tex.y + (translationz *10.0f)));
textureColor = lerp(textureColor, shaderTexture2.Sample(SampleType, float2(input.tex.x + (translationx*4.0f), input.tex.y + (translationz *4.0f))),0.5f);


if (textureColor.a <0.3f) discard;


outcolor = input.color;
// Initialize the specular color.
specular = float4(0.0f, 0.0f, 0.0f, 0.0f);



// Calculate the amount of light on this pixel.
lightIntensityDirectional = saturate(dot(n, lightDir));



if (normal_height>0.0f)
{

	if (input.cam_dist < 45.0f)
	{
		normalMap = normalTexture.Sample(SampleType, float2(input.tex.x + (translationx*10.0f), input.tex.y + (translationz *10.0f)));
		normalMap2 = normalTexture.Sample(SampleType, float2(input.tex.x + (translationx*4.0f), input.tex.y + (translationz *4.0f)));

		normalMap = normalize(lerp(normalMap, normalMap2, 0.5f));

		normalMap = ((normalMap * (2.0f*normal_height)) - normal_height);

		n = (normalMap.x * input.tangent) + (normalMap.y * input.binormal) + (normalMap.z * input.normal);

		n = normalize(n);

		if (input.cam_dist > 35.0f)
		{
			n = lerp(n, input.normal, (input.cam_dist - 35.0f)*0.1f);
		}
		else
		{
			//n = n;;// +input.normal;// lerp(n, input.normal, 0.8f);
		}
		n = normalize(n);
	}
	else
	{
		n = input.normal;
	}

	if (lightIntensityDirectional > 0.0f)
	{
		// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
		outcolor += ((diffuseColor * MaterialDiffuse)* lightIntensityDirectional);

		// Saturate the ambient and diffuse color.
		outcolor = saturate(outcolor);

	}
}


if (MaterialSpecular.w > 0.0f)
{
	if (lightIntensityDirectional > 0.0f)
	{
		reflection = normalize(2 * lightIntensityDirectional * n - lightDir);

		specular = ((MaterialSpecular*specularColor)*2.0f)*pow(saturate(dot(reflection, input.viewDirection)), MaterialSpecularPower*0.2f);

	}
}




for (int i = 0; i < numSpotLights; i++)
{
	l = (spotLights[i].pos - input.positionW.xyz);

	float distance = length(l);

	if (distance < spotLights[i].radius)
	{
		l = normalize(l);

		if (dot(-spotLights[i].dir, l) > 0.0f)
		{
			//lightIntensity = saturate(dot(n, l));

			lightIntensity = 0.3f + (saturate(dot(n, l)));

			if (lightIntensity > 0.0f)
			{
				atten = (1.0f - (distance / spotLights[i].radius)) * (1.0 / (1.0 + 0.1*distance + 0.01*distance*distance));

				viewPosition = mul(float4(input.positionW.xyz, 1.0f), spotLights[i].dirView);
				viewPosition = mul(viewPosition, spotLights[i].dirProj);

				projectTexCoord.x = viewPosition.x / viewPosition.w / 2.0f + 0.5f;
				projectTexCoord.y = -viewPosition.y / viewPosition.w / 2.0f + 0.5f;

				bool spot_lighted = false;

				if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
				{

					[unroll(8)]for (int j = 0; j < 8; j++)
					{
						if (spotLights[i].lightmap == j)
						{
							lightmapValue = lightmapArray[j].Sample(SampleType, float3(projectTexCoord, 1.0f));
						}
					}


					if (lightmapValue.r > 0.0f)
					{
						[unroll(MAX_POINT_SHADOWS)]for (int j = 0; j < 8; j++)
						{
							if (spotLights[i].shadow_cube_index == j)
							{
								depthValue = spotshadowMapArray[j].Sample(samPointSam, float3(projectTexCoord, 1.0f)).r;
							}
						}

						lightDepthValue = viewPosition.z / viewPosition.w;

						lightDepthValue = lightDepthValue - bias;

						if (true)//(lightDepthValue < depthValue)
						{
							spot_lighted = true;
						}
						else
						{
							spot_lighted = false;
						}
					}
				}

				if (spot_lighted == true)
				{

					outputColor = max(outputColor, (spotLights[i].ambient + ((spotLights[i].diffuse * (lightIntensity)) * lightmapValue)) * LIGHT_SPOT_MULTIPLIER * atten);

					//float spotlight = (pow(max(dot(-l, lights[i].dir), 0.0f), lights[i].spot))*lightIntensity;
					//float4 spotlightness = (spotlight*2.0f)*atten*((lights[i].diffuse + lights[i].ambient)*1.0f);
					//if (spotLights[i].specular_power > 0.0f)// && normals.w == 0)
					//{
					//	reflection = normalize(2 * lightIntensity * n - l);

					//	specular += ((float4(speculars.xyz, 1.0f) *spotLights[i].specular)*2.0f)*pow(saturate(dot(reflection, viewDir)), spotLights[i].specular_power);
					//}
				}

			}
		}
	}

}




for (int i = 0; i < numPointLights; i++)
{
	l = (pointLights[i].pos - input.positionW.xyz);

	float distance = length(l);

	if (distance < pointLights[i].radius)
	{
		atten = (1.0f - (distance / pointLights[i].radius)) * (1.0 / (1.0 + 0.1*distance + 0.01*distance*distance));

		//atten = attenu(distance(input.positionW, lights[i].pos), 3.0f, a_a, a_b, a_c);
		l = normalize(l);
		//h = normalize(l + v);

		lightIntensity = 0.3f + (saturate(dot(n, l)));
		if (lightIntensity > 0.0f)
		{
			bool bAddLight = true;

			if (pointLights[i].shadow_cube_index > 0)
			{
				float LightDepth = VectorToDepth(input.positionW.xyz - pointLights[i].pos);

				float Dist = LightDepth;

				float3 light_dir = normalize(l);

				const int indx = pointLights[i].shadow_cube_index;

				light_dir.x = -light_dir.x;

				float depthValue2 = 0.0f;


				[unroll(MAX_POINT_SHADOWS)]for (int j = 0; j < MAX_POINT_SHADOWS; j++)
				{
					if (pointLights[i].shadow_cube_index == j)
					{
						depthValue2 = pointshadowMapCubeArray[j].Sample(samPointSam, float4(-light_dir, 1.0f)).r; break;
					}
				}

				if (Dist - bias < depthValue2)
				{
					bAddLight = true;
				}
				else
				{
					bAddLight = false;
				}
			}
			else
			{
				bAddLight = true;
			}

			if (bAddLight == true)
			{

				outputColor = max(outputColor, (pointLights[i].ambient + ((pointLights[i].diffuse * (lightIntensity)))) * LIGHT_POINT_MULTIPLIER * atten);


				if (pointLights[i].specular_power > 0.0f)
				{
					reflection = normalize(2 * lightIntensity * n - l);

					specular += ((MaterialSpecular *pointLights[i].specular)*2.0f)*pow(saturate(dot(reflection, input.viewDirection)), pointLights[i].specular_power);
				}
				//outputColor += atten * lights[i].ambient;

				// Calculate the reflection vector based on the light intensity, normal vector, and light direction.
				//reflection = normalize(2 * lightIntensity * n - l);

				// Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
				//specular += ((MaterialSpecular*lights[i].specular)*2.0f)*pow(saturate(dot(reflection, input.viewDirection)), MaterialSpecularPower)*atten;
			}
		}
	}
}





//float s = input.cam_dist;
//s = saturate(100.0f - s);

//color = lerp(color, float4(0.5f,0.5f,0.5f,0.0f), s/10.0f);

// Multiply the texture pixel and the input color to get the textured result.
outcolor = (outcolor*textureColor)*2.0f;  // 2.0f is gamma
										  // textureColor.w;// *vertexCol.w;// textureColor.w; // retain alpha
										  //outcolor.w = input.color.w;


if (true)//(specular.w >0.0f)
{
	outcolor = saturate(outcolor + specular);
}

if (true) // sky reflection , cool
{
	float3 vec_tp = normalize(reflect(-normalize(input.viewDirection), normalize(-n)));

	vec_tp.x = -vec_tp.x;
	outcolor = lerp(float4(environmentMapCube.Sample(SampleType, vec_tp).rgb, 1.0f), outcolor, 0.3f);

	//outcolor = float4(environmentMapCube.Sample(SampleType, vec_tp).rgb, 1.0f);
	//float3 vec_tp = input.positionW - input.pointPosition;// float3(input.positionW.x - input.pointPosition.x, input.positionW.y - input.pointPosition.y, input.positionW.z - input.pointPosition.z);

	//float Dist = length(vec_tp);

	//vec_tp = normalize(vec_tp);

	//depthValue = pointshadowMapCube.Sample(SampleType, vec_tp).r;

	//outcolor.r = depthValue;
}
outcolor.a = 0.8f;// textureColor.a;// *input.color.a;
							// Add the specular component last to the output color.
return max(outcolor, (outcolor + outputColor)*0.1f);// float4(1.0f, 0.0f, 1.0f, 1.0f);


}