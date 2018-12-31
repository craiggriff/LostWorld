////////////////////////////////////////////////////////////////////////////////
// Filename: depth.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix mvp;
};


struct VertexInputType
{
	float4 position : POSITION;
	float3 normal : NORMAL0;
	float4 tangent : TANGENT0;
	float4 color : COLOR0;
	float2 tex : TEXCOORD0;
};

//For input into the pixel shader we will need the vertex position in homogeneous clip space stored in the position variable as usual.However since the SV_POSITION semantic offsets the position by 0.5 we will need a second set of coordinates called depthPosition that are not modified so that we can perform depth calculations.

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 depthPosition : TEXTURE0;
	float2 tex : TEXCOORD0;
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
	//output.position = mul(input.position, worldMatrix);
	//output.position = mul(output.position, dirView);
	//output.position = mul(output.position, dirProj);

	output.position = mul(input.position, mvp);

	//As described in the pixel shader input we store a second pair of position coordinates that will not be offset which will be used for depth calculations.

	// Store the position value in a second input value for depth value calculations.
	output.depthPosition = output.position;

	output.tex = input.tex;

	return output;
}
