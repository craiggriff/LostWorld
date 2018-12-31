#pragma once



#include "MeshMotion.h"


#include "Level.h"

#include "statics.h"
#include "stuff.h"

#include "Sky.h"
#include "skyplaneclass.h"

#include "buggy.h"
#include "bert.h"

#include "LevelEdit.h"

#include "steam.h"
#include "dirt.h"
#include "Splash.h"
#include "Rain.h"
#include "Snow.h"
#include "fog.h"
#include "grass.h"
#include "Fire.h"

#include "rendertextureclass.h"
#include "RenderTextureArray.h"

#include "myprofiler.h"

#include "PanCamera.h"

namespace Game
{


	class GameScene
	{
	public:
		GameScene(AllResources* p_Resources);

		void Initialize();

		void Update(DX::StepTimer const& timer);
		void RenderAlphaMode(int alpha_mode);
		void Render();
		void RenderSkyCubeMap();
		void RenderDeferred();
		void RenderShadows();
		void RenderGlowTexture();
		void RenderParticles();

		void SetConstantModelBuffer(DirectX::XMFLOAT4X4* p_Matrix);

		void CreateWindowSizeDependentResources();

		void LoadLevel(int level);
		void SaveLevel(int level);


		bool bInitialized;

		int current_level;
		int current_level_to;

		int make_delay;

		bool bMusicStarted;

		bool bRested; // dice rested

		bool bLoaded;
		bool bLoadedPhysicsDelay;
		double delay_start;
		float y_pos;

		bool bUpdating;

		PanCamera* m_PanCamera;

		MyProfiler* m_Profiler;

		RenderTextureClass* m_TestTexture;

		RenderTextureClass* m_GlowTexture;
		RenderTextureClass* m_GlowTextureDownsample;
		RenderTextureClass* m_GlowTextureDownsample2;

		RenderTextureClass* m_DeferredTexture;

		RenderTextureClass* m_ParticleTexture;

		RenderCube* m_MetalBertCube;


		ObjInfo info;

		Level*	m_Level;

		Statics* m_Statics;
		Stuff* m_Stuff;

		Sky* m_Sky;
		SkyPlaneClass* m_Skyplane;

		Splash* m_Splash;
		Rain* m_Rain;
		Snow* m_Snow;
		Fog* m_Fog;

		dirt* m_dirt;

		steam* m_steam;

		Grass* m_Grass;
		Fire* m_Fire;


		LevelEdit* m_LevelEdit;

		Buggy* m_Buggy;
		Bert* m_Bert;



		int up_index[16];// which side number is pointing upwards


		AllResources* m_Resources;

		void CheckNumSaveLoad(Windows::System::VirtualKey key, int level);

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;


	};

}