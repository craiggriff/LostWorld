////////////////////////////////////////////////////////////////////////////////
// Filename: light.ps
////////////////////////////////////////////////////////////////////////////////
#include "DefShader.h"

// As inputs it takes the two render textures that contain the color information and the normals from the deferred shader.

/////////////
// GLOBALS //
/////////////
Texture2D colorTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D positionTexture : register(t2);
Texture2D specularTexture : register(t3);

TextureCube environmentMapCube : register(t5);

Texture2D testTexture : register(t6);


SamplerState SampleType : register(s0);




TextureCubeArray pointshadowMapCubeArray[MAX_POINT_SHADOWS] : register(t60);

Texture2DArray spotshadowMapArray[MAX_SPOT_SHADOWS] : register(t100);

Texture2DArray lightmapArray[8] : register(t40);

// We require a point sampler since we will be sampling out exact per - pixel data values from the render textures.


///////////////////
// SAMPLE STATES //
///////////////////
//SamplerState SampleTypePoint : register(s0);

SamplerState samPointSam {
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};


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
	uint shadow_index;
	float spare;
	int lightmap;
	matrix dirView; // directional light view and projection
	matrix dirProj;
};

StructuredBuffer<PointLight> pointLights : register(t50);
StructuredBuffer<PointLight> spotLights : register(t51);

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

float manhattandist(float x, float y, float z, float x1, float y1, float z1, float distance)
{
	float dx = abs(x - x1);
	if (dx > distance) return -1.0f; // too far in x direction

	float dz = abs(z - z1);
	if (dz > distance) return -1.0f; // too far in z direction

	float dy = abs(y - y1);
	if (dy > distance) return -1.0f; // too far in y direction

	return 1.0f; // we're within the cube
}


float4 mix(float4 col1, float4 col2)
{
	float4 add = (col1 + col2)*0.04f;
	float4 m = max(col1, col2);

	return m;// add + m;
}

float VectorToDepth(float3 Vec)
{
	//return 0.5f;
	float3 AbsVec = abs(Vec);
	float LocalZcomp = max(AbsVec.x, max(AbsVec.y, AbsVec.z));

	// Replace f and n with the far and near plane values you used when
	//   you drew your cube map.
	const float f = LIGHT_RADIUS;
	const float n = 0.1f;

	float NormZComp = (f + n) / (f - n) - (2 * f*n) / (f - n) / LocalZcomp;
	return (NormZComp + 1.0) * 0.5;
}

//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float2 pad : PAD0;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
	float4 colors;
	float4 normals;
	float4 positions;
	float4 speculars;
	float4 specular =  float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightDir;
	float lightIntensity;
	float4 outputColor;
	float spot_atten;
	float3 l,n;
	float3 reflection;
	float bias = 0.001f;

	float2 projectTexCoord;
	float4 projectionColor;
	float lightDepthValue;
	float depthValue;
	float depthValue2;
	float4 lightmapValue;

	float4 ambDiffColor;

	float3 viewDir;

	float LightDepth;
	float Dist;

	float3 point_light_dir;

	float point_distance;
	float point_atten;

	float pixel_distance;
	float spot_distance;

	int i;
	int k;


	// In the light pixel shader we start by retrieving the color data and normals for this pixel using the point sampler.

	// Sample the colors from the color render texture using the point sampler at this texture coordinate location.
	colors = colorTexture.Sample(samPointSam, input.tex);
	if (colors.w == 0)
		discard;
	// Sample the normals from the normal render texture using the point sampler at this texture coordinate location.
	normals = normalTexture.Sample(samPointSam, input.tex);

	positions = positionTexture.Sample(samPointSam, input.tex);

	speculars = specularTexture.Sample(samPointSam, input.tex);
	//We can then perform our directional lighting equation using this sampled information.

	viewDir = eye_position - positions.xyz;

	pixel_distance = length(viewDir);

	viewDir = normalize(viewDir);

	// Invert the light direction for calculations.
	lightDir = -lightDirection;

	lightDir = normalize(lightDir);

	// Calculate the amount of light on this pixel.
	if (normals.w > 0.0f) // ignore normals
	{
		lightIntensity = normals.w;
	}
	else
	{
		lightIntensity = saturate(dot(normals.xyz, lightDir));
	}


	// Determine the final amount of diffuse color based on the color of the pixel combined with the light intensity.
	//outputColor = float4(normals.x, normals.y, normals.z, 1.0f);// colors;// saturate(colors * lightIntensity);
	//outputColor = colors;// saturate(colors * lightIntensity);
	//outputColor = colors;
	outputColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	ambDiffColor = max(diffuseColor* lightIntensity,ambientColor);

	n = normals.xyz;

	if (speculars.a > 0.0f)
	{
		if (lightIntensity > 0.0f)
		{
			reflection = normalize(2 * lightIntensity * n - lightDir);

			specular = ((float4(speculars.xyz, 1.0f) *specularColor)*2.0f)*pow(saturate(dot(reflection, viewDir)), speculars.w*0.2f);
		}
	}

	float4 viewPosition;

	for (k = 0; k < numSpotLights; k++)
	{
		l = (spotLights[k].pos - positions.xyz);

		spot_distance = length(l);

		if (spot_distance < spotLights[k].radius)
		{
			l = normalize(l);

			if (dot(-spotLights[k].dir, l) > 0.0f)
			{
				//lightIntensity = saturate(dot(n, l));

				if (normals.w > 0.0f) // ignore normals
				{
					lightIntensity = normals.w;// 
				}
				else
				{
					lightIntensity = saturate(dot(n, l));
				}


				spot_atten = (1.0f - (spot_distance / spotLights[k].radius)) * (1.0 / (1.0 + 0.1*spot_distance + 0.01*spot_distance*spot_distance));

				viewPosition = mul(float4(positions.xyz, 1.0f), spotLights[k].dirView);
				viewPosition = mul(viewPosition, spotLights[k].dirProj);

				projectTexCoord.x = viewPosition.x / viewPosition.w / 2.0f + 0.5f;
				projectTexCoord.y = -viewPosition.y / viewPosition.w / 2.0f + 0.5f;

				bool spot_lighted = true;

				if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
				{
					[unroll(8)]for (int j = 0; j < 8; j++)
					{
						if (spotLights[k].lightmap == j)
						{
							lightmapValue = lightmapArray[j].Sample(SampleType, float3(projectTexCoord, 1.0f));
						}
					}

					if (lightmapValue.r > 0.0f)
					{
						depthValue=testTexture.Sample(SampleType, projectTexCoord).r*2.0f;

						//depthValue = 50.0f;
						
						[unroll(MAX_SPOT_SHADOWS)]for (int j = 0; j <MAX_SPOT_SHADOWS; j++)
						{
							if (spotLights[i].shadow_index == j)
							{
								depthValue = spotshadowMapArray[j].Sample(samPointSam, float3(projectTexCoord, 1.0f)).r;
							}
						}
						
						lightDepthValue = viewPosition.z / viewPosition.w;

						if (false)//(lightDepthValue - bias > depthValue)
						{
							spot_lighted = false;
						}
					}
				}

				if (spot_lighted == true && lightmapValue.r > 0.0f)
				{
					float4 ambient = (spotLights[k].diffuse*0.3f);
					outputColor = mix(outputColor, (max((spotLights[k].diffuse * lightIntensity) , ambient)) * lightmapValue * LIGHT_SPOT_MULTIPLIER * spot_atten);


					if (lightIntensity > 0.0f)
					{

						if (spotLights[k].specular_power > 0.0f && normals.w == 0)
						{
							reflection = normalize(2 * lightIntensity * n - l);

							specular += ((float4(speculars.xyz, 1.0f) *spotLights[k].specular)*2.0f)*pow(saturate(dot(reflection, viewDir)), spotLights[k].specular_power);
						}

						// Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
						//specular += spotlightness;// ((MaterialSpecular*lights[i].specular)*2.0f)*pow(saturate(dot(reflection, input.viewDirection)), MaterialSpecularPower*0.1f)*spotlight*2.0f;
					}
				}
			}
		}
	}




	for (i = 0; i < numPointLights; i++)
	{
		if (manhattandist(pointLights[i].pos.x, pointLights[i].pos.y, pointLights[i].pos.z,
			positions.xyz.x, positions.xyz.y, positions.xyz.z, pointLights[i].radius) < 0.0f)
			continue;


		l = (pointLights[i].pos - positions.xyz);

		point_distance = length(l);

		if (point_distance > pointLights[i].radius)
			continue;

		point_atten = (1.0f - (point_distance / pointLights[i].radius)) * (1.0 / (1.0 + 0.1*point_distance + 0.01*point_distance*point_distance));

		if (point_atten < 0.1f)
			continue;

		l = normalize(l);

		lightIntensity = max(normals.w, saturate(dot(n, l)));
		bool bAddLight = true;
		const int indx = pointLights[i].shadow_index;

		depthValue2 = 1.0f;

		if (indx > -1)
		{
			point_light_dir = normalize(l);

			point_light_dir.x = -point_light_dir.x;

			
			
			[unroll(MAX_POINT_SHADOWS)]for (int j =0; j < MAX_POINT_SHADOWS-1; j++)
			{
				if (indx == j)
				{
					depthValue2 = pointshadowMapCubeArray[j].Sample(samPointSam, float4(-point_light_dir, 1.0f)).r; break;

					//break;
				}
			}

			//if(bFound==true)
			if (VectorToDepth(positions.xyz - pointLights[i].pos) - bias > depthValue2)
				continue;

			//depthValue2 = pointshadowMapCubeArray[2].Sample(samPointSam, float4(-point_light_dir, 1.0f)).r; break;

		}

		float4 ambient = (pointLights[i].diffuse*0.3f);
		outputColor = mix(outputColor, (max((pointLights[i].diffuse * lightIntensity), ambient)) * LIGHT_POINT_MULTIPLIER * point_atten);


		if (lightIntensity <= 0.0f || pointLights[i].specular_power <=0.0f || normals.w>0.0f)
			continue;

		reflection = normalize(2 * lightIntensity * n - l);

		specular += ((float4(speculars.xyz, 1.0f) *pointLights[i].specular)*2.0f)*pow(saturate(dot(reflection, viewDir)), pointLights[i].specular_power);
	}

	if (false)
	{
		float3 vec_tp = normalize(reflect(-normalize(viewDir), normalize(n)));

		vec_tp.x = -vec_tp.x;

		outputColor = float4(environmentMapCube.Sample(SampleType, vec_tp).rgb, 1.0f);
	}
	float fade = 1.0f-((1.0f / MAX_LIGHT_DISTANCE) * pixel_distance);

	float fade_atten = (1.0f - (pixel_distance / MAX_LIGHT_DISTANCE)) * (1.0 / (1.0 + 0.1*pixel_distance + 0.01*pixel_distance*pixel_distance));
	fade_atten = 0.3f + fade_atten * 0.7f;
	return (((outputColor  * fade_atten + ambDiffColor + specular) * (colors * 0.9f))+ ((outputColor  * fade_atten + ambDiffColor + specular)*0.04f));
}

