////////////////////////////////////////////////////////////////////////////////
// Filename: depth.vs
////////////////////////////////////////////////////////////////////////////////

#define MAX_POINT_LIGHTS 10
/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
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
	float3 spare;
};



cbuffer VLightBuffer : register(b5)
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightDirection;
	float specularPower;
	float4 specularColor;
	float4 fogColor;
	int numLights;
	float3 pos;
	float lightning;
	float3 lightning_dir;
	float3 eye_position;
	float padding;
	matrix dirView; // directional light view and projection
	matrix dirProj;
	PointLight lights[MAX_POINT_LIGHTS];

};

struct VertexInputType
{
	float4 position : POSITION;
};

//For input into the pixel shader we will need the vertex position in homogeneous clip space stored in the position variable as usual.However since the SV_POSITION semantic offsets the position by 0.5 we will need a second set of coordinates called depthPosition that are not modified so that we can perform depth calculations.

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 depthPosition : TEXTURE0;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType main(VertexInputType input)
{
	PixelInputType output;


	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, dirView);
	output.position = mul(output.position, dirProj);

	//As described in the pixel shader input we store a second pair of position coordinates that will not be offset which will be used for depth calculations.

	// Store the position value in a second input value for depth value calculations.
	output.depthPosition = output.position;

	return output;
}
