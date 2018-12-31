#pragma once
#include "../Bullet/src/btBulletDynamicsCommon.h"

//#include "DefParticle.h"
#include "DefShader.h"

namespace Game
{

	typedef struct
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 dir; // camera point direction
	} PathPoint;

	typedef struct
	{
		int spot_index;
		DirectX::XMFLOAT4X4 dirView; // Directional Light View
		DirectX::XMFLOAT4X4 dirProj; // Directional Light Projection
	} SpotShadow;

	typedef struct
	{
		int path_type;
		std::vector<PathPoint> path_points;
	} TerrainPath;

	struct ScreenSizeBufferType
	{
		float width_size;
		float height_size;
		DirectX::XMFLOAT2 padding;
	};

	struct SkyBufferType
	{
		float translationx;
		float translationz;
		float scale;
		float brightness;
	};


	struct CG_POINT_LIGHT
	{
		DirectX::XMFLOAT4 ambient;
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT4 specular;
		DirectX::XMFLOAT3 pos;

		float _specular_power;
		bool bCastShadows;
	};
	struct CG_SPOT_LIGHT
	{
		DirectX::XMFLOAT4 ambient;
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT4 specular;
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 dir;
		DirectX::XMFLOAT3 up;
		int lightmap;
		float spot;

		float _specular_power;
		bool bCastShadows;
	};

	struct PointLightType
	{
		float radius;
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 ambient;
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT4 specular;
		DirectX::XMFLOAT3 dir;
		float spot;
		float specular_power;
		unsigned int shadow_index;
		float spare;
		int lightmap;
		DirectX::XMFLOAT4X4 dirView; // Directional Light View
		DirectX::XMFLOAT4X4 dirProj; // Directional Light Projection
	};


	struct LightBufferType
	{
		DirectX::XMFLOAT4 ambientColor;
		DirectX::XMFLOAT4 diffuseColor;
		DirectX::XMFLOAT3 lightDirection;
		float specularPower;
		DirectX::XMFLOAT4 specularColor;
		DirectX::XMFLOAT4 fogColor;

		DirectX::XMFLOAT3 eye_position;
		unsigned int numPointLights;
		unsigned int numSpotLights;
		float lightning;
		DirectX::XMFLOAT3 lightning_dir;
		DirectX::XMFLOAT3 pos;

	};

	struct MaterialConstants
	{
		MaterialConstants()
		{
			static_assert((sizeof(MaterialConstants) % 16) == 0, "CB must be 16-byte aligned");
			Ambient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			Specular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			Emissive = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			SpecularPower = 1.0f;
			Padding0 = 0.0f;
			Padding1 = 0.0f;
			Padding2 = 0.0f;
		}

		DirectX::XMFLOAT4   Ambient;
		DirectX::XMFLOAT4   Diffuse;
		DirectX::XMFLOAT4   Specular;
		DirectX::XMFLOAT4   Emissive;
		float               SpecularPower;
		float               Padding0;
		float               Padding1;
		float               Padding2;
	};

	struct LightConstants
	{
		LightConstants()
		{
			static_assert((sizeof(LightConstants) % 16) == 0, "CB must be 16-byte aligned");
			ZeroMemory(this, sizeof(LightConstants));
			Ambient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		DirectX::XMFLOAT4   Ambient;
		DirectX::XMFLOAT4   LightColor[4];
		DirectX::XMFLOAT4   LightAttenuation[4];
		DirectX::XMFLOAT4   LightDirection[4];
		DirectX::XMFLOAT4   LightSpecularIntensity[4];
		UINT                IsPointLight[4];
		UINT                ActiveLights;
		float               Padding0;
		float               Padding1;
		float               Padding2;
	};

	struct ObjectConstants
	{
		ObjectConstants()
		{
			static_assert((sizeof(ObjectConstants) % 16) == 0, "CB must be 16-byte aligned");
			ZeroMemory(this, sizeof(ObjectConstants));
		}

		DirectX::XMMATRIX   LocalToWorld4x4;
		DirectX::XMMATRIX   LocalToProjected4x4;
		DirectX::XMMATRIX   WorldToLocal4x4;
		DirectX::XMMATRIX   WorldToView4x4;
		DirectX::XMMATRIX   UvTransform4x4;
		DirectX::XMFLOAT3   EyePosition;
		float               Padding0;
	};

	struct MiscConstants
	{
		MiscConstants()
		{
			static_assert((sizeof(MiscConstants) % 16) == 0, "CB must be 16-byte aligned");
			ViewportWidth = 1.0f;
			ViewportHeight = 1.0f;
			Time = 0.0f;
			Padding1 = 0.0f;
		}

		float ViewportWidth;
		float ViewportHeight;
		float Time;
		float Padding1;
	};

	struct Vertex
	{
		float x, y, z;
		float nx, ny, nz;
		float tx, ty, tz, tw;
		UINT color;
		float u, v;
	};



	struct VertexData
	{
	public:
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 norm;
		DirectX::XMFLOAT4 tan;
		UINT color;
		DirectX::XMFLOAT2 tex;
		//DirectX::XMFLOAT3 bin; // binormal


		void SetNormal(float x, float y, float z) { norm = DirectX::XMFLOAT3(x, y, z); }
		void SetPosition(float x, float y, float z) { pos = DirectX::XMFLOAT3(x, y, z); }
		void SetTex(float u, float v) { tex = DirectX::XMFLOAT2(u, v); }

		void SetTangent(float x, float y, float z, float w) { tan = DirectX::XMFLOAT4(x, y, z, w); }
		void SetBinormal(float x, float y, float z) {  }

		void SetColor(DirectX::XMFLOAT4 c) {/* col = c;*/ }

	};


	/*

	float4 position : POSITION0;
	float3 normal : NORMAL0;
	float4 tangent : TANGENT0;
	float4 color : COLOR0;
	float2 tex : TEXCOORD0;
	float3 binormal : BINORMAL;
	*/

	struct GroundVertexData
	{
	public:
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 norm;
		DirectX::XMFLOAT2 tex;
		float blend;
		float blend_2;
		float blend_3;
		float blend_4;
		float blend_5;
		float blend_6;
		float blend_7;
		float blend_8;

		DirectX::XMFLOAT4 col;
		DirectX::XMFLOAT3 tan;
		DirectX::XMFLOAT3 bin; // binormal

		void SetNormal(float x, float y, float z) { norm = DirectX::XMFLOAT3(x, y, z); }
		void SetPosition(float x, float y, float z) { pos = DirectX::XMFLOAT3(x, y, z); }
		void SetTex(float u, float v) { tex = DirectX::XMFLOAT2(u, v); }

		void SetBlend(float b) { blend = b; }
		void SetBlend2(float b) { blend_2 = b; }
		void SetBlend3(float b) { blend_3 = b; }
		void SetBlend4(float b) { blend_4 = b; }
		void SetBlend5(float b) { blend_5 = b; }
		void SetBlend6(float b) { blend_6 = b; }
		void SetBlend7(float b) { blend_7 = b; }

		void SetColor(float r, float g, float b, float a) { col.x = r; col.y = g; col.z = b; col.w = a; }

		void SetTan(float x, float y, float z) { tan = DirectX::XMFLOAT3(x, y, z); }
		void SetBin(float x, float y, float z) { bin = DirectX::XMFLOAT3(x, y, z); }

	};



	struct WaterVertexData
	{
	public:
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 norm;
		DirectX::XMFLOAT2 tex;
		float blend;
		float blend_2;
		DirectX::XMFLOAT4 col;
		DirectX::XMFLOAT3 tan;
		DirectX::XMFLOAT3 bin; // binormal

		void SetNormal(float x, float y, float z) { norm = DirectX::XMFLOAT3(x, y, z); }
		void SetPosition(float x, float y, float z) { pos = DirectX::XMFLOAT3(x, y, z); }
		void SetTex(float u, float v) { tex = DirectX::XMFLOAT2(u, v); }

		void SetBlend(float b) { blend = b; }
		void SetBlend2(float b) { blend_2 = b; }
		void SetColor(float r, float g, float b, float a) { col.x = r; col.y = g; col.z = b; col.w = a; }
		void SetTan(float x, float y, float z) { tan = DirectX::XMFLOAT3(x, y, z); }
		void SetBin(float x, float y, float z) { bin = DirectX::XMFLOAT3(x, y, z); }
	};


	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
		DirectX::XMFLOAT4X4 mvp; // pre calculated
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};

	struct HeightMapInfo {				// Heightmap structure
		int terrainWidth;				// Width of heightmap
		int terrainHeight;				// Height (Length) of heightmap
		DirectX::XMFLOAT3 *heightMap;	// Array to store terrain's vertex positions
		DirectX::XMFLOAT3 *normal;		// Array to store terrain's vertex positions
		DirectX::XMFLOAT4 *colour;		// Array to store terrain's vertex positions
	};

	struct VertexPositionTex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 tex;
	};

	struct VertexPositionTexCol
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 col;
		DirectX::XMFLOAT2 tex;
		DirectX::XMFLOAT3 padding;
	};

	struct ParticleInstance
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		float padding_b;
	};

	typedef struct
	{
		float x, y, z;
	}VertexType;

	typedef struct
	{
		int vIndex1, vIndex2, vIndex3;
		int tIndex1, tIndex2, tIndex3;
		int nIndex1, nIndex2, nIndex3;
	}FaceType;


	struct CameraBufferType
	{
		DirectX::XMFLOAT3 cameraPosition;
		float padding;
	};

	struct NoiseBufferType
	{
		float frameTime;
		DirectX::XMFLOAT3 scrollSpeeds;
		DirectX::XMFLOAT3 scales;
		float padding;
	};

	struct DistortionBufferType
	{
		DirectX::XMFLOAT2 distortion1;
		DirectX::XMFLOAT2 distortion2;
		DirectX::XMFLOAT2 distortion3;
		float distortionScale;
		float distortionBias;
	};


	struct MaterialBufferType
	{
		MaterialBufferType()
		{
			static_assert((sizeof(MaterialBufferType) % 16) == 0, "CB must be 16-byte aligned");
			Ambient = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
			Diffuse = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
			Specular = DirectX::XMFLOAT4(0.0f, 0.3f, 0.0f, 1.0f);
			Emissive = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			SpecularPower = 30.0f;
			normal_height = 0.0f;
			Padding1 = 0.0f;
			Padding2 = 0.0f;
		}

		DirectX::XMFLOAT4   Ambient;
		DirectX::XMFLOAT4   Diffuse;
		DirectX::XMFLOAT4   Specular;
		DirectX::XMFLOAT4   Emissive;
		float               SpecularPower;
		float               normal_height;
		float               Padding1;
		float               Padding2;
	};




	typedef struct
	{
		int part_no;
		int tail_no;
		float dist;
	} part_index;

	typedef struct
	{
		float x;
		float y;
		float z;
	}norm_t;


	struct ObjInfo
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 dir; // yaw pitch roll or vector
		DirectX::XMFLOAT3 mrf; // mass restitution friction
		int item_id;
		int group;
		int mask;
	};


	struct LevelInfo
	{
		int db_rec_id;
		int level;
		char terrain_tex1[30];
		char terrain_tex2[30];
		char terrain_tex3[30];
		char terrain_tex4[30];
		char terrain_tex5[30];
		char terrain_tex6[30];
		char terrain_tex7[30];
		char terrain_tex8[30];
		char terrain_normal1[30];
		char terrain_normal2[30];
		char terrain_normal3[30];
		char terrain_normal4[30];
		char terrain_normal5[30];
		char terrain_normal6[30];
		char terrain_normal7[30];
		char terrain_normal8[30];
		char skybox[30];
		DirectX::XMFLOAT3 diff_dir; // diffuse light direction
		DirectX::XMFLOAT4 diffuse_col; // diffuse light col
		DirectX::XMFLOAT4 ambient_col; // diffuse light col
		DirectX::XMFLOAT4 specular_col; // diffuse light col
		DirectX::XMFLOAT4 fog_col; // diffuse light col
		DirectX::XMFLOAT4 dust_col; // dust light col
		float specular_power;
		float ground_friction;
		float ground_restitution;
		float player_start_x;
		float player_start_z;
		float player_start_angle;
		int bRain;
		int bSnow;
		int bFog;
		float ground_steepness_blend;
		DirectX::XMFLOAT4 wind; // wind - x & z = dir and y = strength & w = wave scale
		float flag_angle;
		char music_track[40];

		float spot_distance;
		float point_distance;


	};





	struct VectorType
	{
		float x, y, z;
	};


	struct TempVertexType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

}