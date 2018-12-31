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
Texture2D depthTexture : register(t4);

SamplerState SampleType : register(s0);

TextureCube environmentMapCube : register(t5);

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

StructuredBuffer<PointLight> lights : register(t50);

SamplerState samPointSam {
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
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

};

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

	float2 projectTexCoord;
	float4 projectionColor;
	float lightDepthValue;
	float depthValue;
	float4 lightmapValue;


	float bias = 0.001f;




	textureColor = shaderTexture.Sample(SampleType, input.tex);

	if (textureColor.a <0.3f) discard;


	outcolor = input.color;
	// Initialize the specular color.
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);



	// Calculate the amount of light on this pixel.
	lightIntensityDirectional = saturate(dot(n, lightDir));



	if (normal_height>0.0f)//normalLvl == 1.0f)
	{
		//discard;
		if (input.cam_dist < 25.0f)
		{
			normalMap = normalTexture.Sample(SampleType, input.tex);

			normalMap = ((normalMap * (2.0f*normal_height)) - normal_height);

			n = (normalMap.x * input.tangent) + (normalMap.y * input.binormal) + (normalMap.z * input.normal);

			n = normalize(n);

			if (input.cam_dist > 15.0f)
			{
				n = lerp(n, input.normal, (input.cam_dist - 15.0f)*0.1f);
			}
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




	for (int i = 0; i < numPointLights; i++)
	{
		l = (lights[i].pos - input.positionW.xyz);

		float distance = length(l);

		if (distance < lights[i].radius)
		{
			atten = (1.0f - (distance / lights[i].radius)) * (1.0 / (1.0 + 0.1*distance + 0.01*distance*distance));

			//atten = attenu(distance(input.positionW, lights[i].pos), 3.0f, a_a, a_b, a_c);
			l = normalize(l);
			//h = normalize(l + v);

			lightIntensity = saturate(dot(n, l));

			if (lightIntensity > 0.0f)
			{/*
				if (lights[i].spot > 0.0f)
				{
					projectTexCoord.x = input.viewPosition.x / input.viewPosition.w / 2.0f + 0.5f;
					projectTexCoord.y = -input.viewPosition.y / input.viewPosition.w / 2.0f + 0.5f;
						
					bool spot_lighted = false;

					if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
					{
						depthValue = depthTexture.Sample(SampleType, projectTexCoord).r;
						
						lightmapValue = lightmapArray[0].Sample(SampleType, float3(projectTexCoord, 1.0f));
						
						lightDepthValue = input.viewPosition.z / input.viewPosition.w;

						lightDepthValue = lightDepthValue - bias;

						if (lightDepthValue < depthValue)
						{
							spot_lighted = true;
						}
						else
						{
							spot_lighted = false;
						}
					}

					if(true)// (spot_lighted == true)
					{

						outcolor += float4(1.0f, 1.0f, 1.0f, 1.0f);// (lights[i].diffuse * lightIntensity * atten);// *(lightmapValue*5.0f);

						//float spotlight = (pow(max(dot(-l, lights[i].dir), 0.0f), lights[i].spot))*lightIntensity;
						//float4 spotlightness = (spotlight*2.0f)*atten*((lights[i].diffuse + lights[i].ambient)*1.0f);


						// Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
						//specular += spotlightness;// ((MaterialSpecular*lights[i].specular)*2.0f)*pow(saturate(dot(reflection, input.viewDirection)), MaterialSpecularPower*0.1f)*spotlight*2.0f;


					}

				} 
				else
				{

					if (lightIntensity > 0.0f)
					{
						bool bAddLight = true;
						if (lights[i].shadow_cube_index > 0)
						{

							float LightDepth = VectorToDepth(input.positionW.xyz - lights[i].pos);

							float Dist = LightDepth;

							float3 light_dir = normalize(l);
							
							const int indx = lights[i].shadow_cube_index;

							light_dir.x = -light_dir.x;

							float depthValue2 = 0.0f;


							[unroll(MAX_POINT_SHADOWS)]for (int j = 0; j < MAX_POINT_SHADOWS; j++)
							{
								if (lights[i].shadow_cube_index == j)
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

						if (bAddLight == true)
						{

							outcolor += lights[i].diffuse * lightIntensity * atten;

							//outcolor += atten * lights[i].ambient;

							// Calculate the reflection vector based on the light intensity, normal vector, and light direction.
							reflection = normalize(2 * lightIntensity * n - l);

							// Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
							specular += ((MaterialSpecular*lights[i].specular)*2.0f)*pow(saturate(dot(reflection, input.viewDirection)), MaterialSpecularPower)*atten;
						}
					}
				}
				*/
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


	if (true)//specular.w >0.0f)
	{
		outcolor = saturate(outcolor + specular);
	}

	if (true) // sky reflection , cool
	{
		float3 vec_tp = normalize(reflect(-normalize(input.viewDirection), normalize(-n)));

		vec_tp.x = -vec_tp.x;
		//outcolor = lerp(float4(environmentMapCube.Sample(SampleType, vec_tp).rgb, 1.0f), outcolor, 0.5f);

		outcolor = outcolor + float4(environmentMapCube.Sample(SampleType, vec_tp).rgb,1.0f) * (specular.w*0.3f);
		//float3 vec_tp = input.positionW - input.pointPosition;// float3(input.positionW.x - input.pointPosition.x, input.positionW.y - input.pointPosition.y, input.positionW.z - input.pointPosition.z);

		//float Dist = length(vec_tp);

		//vec_tp = normalize(vec_tp);

		//depthValue = pointshadowMapCube.Sample(SampleType, vec_tp).r;

		//outcolor.r = depthValue;
	}
	outcolor.a = textureColor.a;// *input.color.a;
	// Add the specular component last to the output color.
	return outcolor;// float4(1.0f, 0.0f, 1.0f, 1.0f);


}