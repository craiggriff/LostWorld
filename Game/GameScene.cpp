 #include "pch.h"
#include "GameScene.h"

#include "DefGame.h"

using namespace Game;

using namespace DirectX;
using namespace Windows::Foundation;
using namespace concurrency;
using namespace Windows::System::Threading;


GameScene::GameScene(AllResources* p_Resources) :
	m_Resources(p_Resources)
{
	m_deviceResources = m_Resources->m_deviceResources;
	Initialize();

	if (m_Resources->bDevelopment == true)
	{
		m_Resources->m_uiControl->SetDesigner(true);
		//m_Stuff->SetPhysical(false);
	}
	else
	{
		m_Resources->m_uiControl->SetDesigner(false);
		//m_Stuff->SetPhysical(true);
	}
	
	m_GlowTexture = new RenderTextureClass;
	m_GlowTextureDownsample = new RenderTextureClass;
	m_GlowTextureDownsample2 = new RenderTextureClass;

	m_DeferredTexture = new RenderTextureClass;

	m_ParticleTexture = new RenderTextureClass;

	m_TestTexture = new RenderTextureClass;

	m_Profiler = new MyProfiler(m_Resources);



}

void GameScene::SetConstantModelBuffer(DirectX::XMFLOAT4X4* p_Matrix)
{
	m_Resources->m_Camera->m_constantBufferData.model = *p_Matrix;


	//XMVectorCatmullRom()

}

void GameScene::CreateWindowSizeDependentResources()
{	
	Windows::Foundation::Size outputSize = m_deviceResources->GetOutputSize();

	m_DeferredTexture->Initialize(m_deviceResources->GetD3DDevice(), (UINT)outputSize.Width, (UINT)outputSize.Height, DXGI_FORMAT_R16G16B16A16_FLOAT);

	m_GlowTexture->Initialize(m_deviceResources->GetD3DDevice(), (UINT)outputSize.Width , (UINT)outputSize.Height , DXGI_FORMAT_R16G16B16A16_FLOAT,4);
	m_GlowTextureDownsample->Initialize(m_deviceResources->GetD3DDevice(), (UINT)outputSize.Width / GLOW_BLUR, (UINT)outputSize.Height / GLOW_BLUR, DXGI_FORMAT_R16G16B16A16_FLOAT);
	m_GlowTextureDownsample2->Initialize(m_deviceResources->GetD3DDevice(), (UINT)outputSize.Width / GLOW_BLUR, (UINT)outputSize.Height / GLOW_BLUR, DXGI_FORMAT_R16G16B16A16_FLOAT);


	m_ParticleTexture->Initialize(m_deviceResources->GetD3DDevice(), (UINT)outputSize.Width, (UINT)outputSize.Height, DXGI_FORMAT_R16G16B16A16_FLOAT, 4);

	m_TestTexture->Initialize(m_deviceResources->GetD3DDevice(), 1024, 1024, DXGI_FORMAT_R16_FLOAT);
}


void GameScene::Initialize()
{
	bLoaded = false;
	bLoadedPhysicsDelay = false;

	m_Sky = new Sky(m_Resources);
	m_Skyplane = new SkyPlaneClass(m_Resources);
	m_Skyplane->Initialize();

	if (m_Resources->bDevelopment == 0)
	{
		m_Sky->Initialize(1500.0f, 0.0f);
	}
	else
	{
		m_Sky->Initialize(1500.0f, 0.0f);
	}

	m_Level = new Level(m_Resources, m_Sky);

	m_PanCamera = new PanCamera(m_Resources->m_Camera, m_Level);

	//m_Resources->m_PanCamera->p_Level = m_Level;

	m_Bert = new Bert(m_Resources, m_Level);
	m_Bert->Initialize(START_CHAR);

	m_Buggy = new Buggy(m_Resources);
	
	m_Buggy->Initialize(m_Level, START_BUGGY);


	m_Fire = new Fire(m_Resources);
	m_Fire->Initialize(m_Level);

	m_dirt = new dirt(m_Resources);
	m_dirt->Initialize(m_Level);

	m_steam = new steam(m_Resources);
	m_steam->Initialize(m_Level);

	m_Splash = new Splash(m_Resources);
	m_Splash->Initialize(m_Level);

	m_Rain = new Rain(m_Resources);
	m_Rain->Initialize(m_Level);
	m_Rain->p_Splash = m_Splash;

	m_Snow = new Snow(m_Resources);
	m_Snow->Initialize(m_Level,true);

	m_Fog = new Fog(m_Resources);
	m_Fog->Initialize(m_Level,true);

	m_Grass = new Grass(m_Resources);
	m_Grass->Initialize(m_Level);

	m_Statics = new Statics(m_Resources);
	m_Statics->Initialize(m_Fire, m_steam);
	m_Statics->LoadModels();

	m_Stuff = new Stuff(m_Resources);
	//m_Stuff->LoadModels();

	current_level = 2;




	//m_Level->InitializeTerrain(1, 0.02f, 0.1f);
	m_Level->InitializeTerrainVariables(32, 32, 0.1f);
	m_Level->InitializeGeneratedTerrain(current_level, 0.04f, 0.0f);
	m_Level->MakePhysics();
	m_Resources->total_terrrain_x_points = m_Level->total_x_points;
	m_Resources->total_terrrain_y_points = m_Level->total_y_points;


	m_Level->UpdateTerrain();
	
	m_LevelEdit = new LevelEdit(m_Resources, m_Level, m_Statics, m_Stuff);

	m_MetalBertCube = new RenderCube();
	m_MetalBertCube->Initialize(m_deviceResources, 512, 512, DXGI_FORMAT_R8G8B8A8_UNORM);



	LoadLevel(START_LEVEL);

	bMusicStarted = false;
}








void GameScene::LoadLevel(int level)
{
	bLoaded = false;
	bLoadedPhysicsDelay = false;

	m_Resources->LoadDatabaseLevelInfo(level);

	m_Sky->LoadSkybox(m_Resources->m_LevelInfo.skybox);

	m_Resources->m_Physics.ClearPhysicsObjects();
	
	m_Buggy->SetPhysical();

	m_Bert->MakePhysics();

	m_Statics->LoadBinary(level);

	m_Statics->MakeAllLights();
	m_Statics->MakePhysics();

	m_Level->LoadBinary(level);

	m_Stuff->LoadBinary(level);
	
	m_Bert->SetPosition(m_Resources->m_LevelInfo.player_start_x + 3.0f, m_Level->GetTerrainHeight(m_Resources->m_LevelInfo.player_start_x, m_Resources->m_LevelInfo.player_start_z) + 10.0f, m_Resources->m_LevelInfo.player_start_z);

	current_level = level;
	current_level_to = level;

	bLoaded = true;
}

void GameScene::SaveLevel(int level)
{
	m_Resources->SaveDatabaseLevelInfo();

	m_Level->SaveBinary(level);
	m_Statics->SaveBinary(level);
	m_Stuff->SaveBinary(level);
	m_Buggy->SetCarPosition(m_Resources->m_LevelInfo.player_start_x, m_Level->GetTerrainHeight(m_Resources->m_LevelInfo.player_start_z, m_Resources->m_LevelInfo.player_start_x) + 2.0f, m_Resources->m_LevelInfo.player_start_z, 0.0f, 0.0f, 0.0f);
	current_level = level;
}


void GameScene::CheckNumSaveLoad(Windows::System::VirtualKey key, int level)
{
	if (m_Resources->m_PadInput->KeyState(key, true) == true)
	{
		if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::Shift) == true)
		{
			if (current_level == level)
			{
				SaveLevel(level);
			}
		}
		else
		{
			current_level_to=level;
		}
	}
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void GameScene::Update(DX::StepTimer const& timer)
{
	int i;
	int total_count = 0;
	float f_height;
	XMFLOAT3 pos, dir;
	XMFLOAT3 cam;
	float dist = (rand() % 400) / 40.0f;
	bool bFound = false;

	if (m_Resources->shaders_loading.is_done() == false)
	{
		return;
	}

	std::vector<concurrency::task<void>> tasks;

	if (bLoaded == false)
	{
		delay_start = timer.GetTotalSeconds();
		return;
	}

	if (current_level_to != current_level)
	{
		delay_start = timer.GetTotalSeconds();
		LoadLevel(current_level_to);

		return;
	}


	if (bLoadedPhysicsDelay == false)
	{
		float es = timer.GetTotalSeconds();
		if ((es - delay_start)>10.0f)
		{
			bLoadedPhysicsDelay = true;
		}
	}
	if (bLoadedPhysicsDelay == false)
		return;


	m_Resources->m_Lights->ResetLights();



	m_Skyplane->Update(timer.GetTotalSeconds(), timer.GetElapsedSeconds());

	//m_Profiler->Start();

	tasks.push_back(concurrency::create_task([this, timer]
	{
		std::vector<concurrency::task<void>> tasks;

		
		
		if (m_Buggy->bLoadingComplete == true)
		{
			CG_SPOT_LIGHT spot_light;
			ZeroMemory(&spot_light, sizeof(CG_SPOT_LIGHT));
			spot_light.ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
			spot_light.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			spot_light.specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
			spot_light.pos = XMFLOAT3(m_Buggy->player_x + (m_Buggy->y_offset.getX()*1.2f) + (m_Buggy->car_point.getX()), 
									  m_Buggy->player_y + (m_Buggy->y_offset.getY()*1.2f) + (m_Buggy->car_point.getY()), 
									  m_Buggy->player_z + (m_Buggy->y_offset.getZ()*1.2f) + (m_Buggy->car_point.getZ()));

			spot_light.dir = XMFLOAT3(m_Buggy->car_point.getX(), m_Buggy->car_point.getY(), m_Buggy->car_point.getZ());
			spot_light.up = XMFLOAT3(m_Buggy->y_offset.getX(), m_Buggy->y_offset.getY(), m_Buggy->y_offset.getZ());
			spot_light.spot = 7.0f;

			spot_light._specular_power = 20.0f;
			spot_light.lightmap = 0;
			m_Resources->m_Lights->AddSpot(spot_light);

			
			/*
			CG_POINT_LIGHT player_point_light;
			ZeroMemory(&player_point_light, sizeof(CG_POINT_LIGHT));
			player_point_light.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			player_point_light.diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
			player_point_light.specular = XMFLOAT4(0.55f, 0.55f, 0.55f, 1.0f);
			player_point_light.pos = XMFLOAT3(m_Bert->GetPosition()->getX(), m_Bert->GetPosition()->getY() + 3.0f, m_Bert->GetPosition()->getZ());
			player_point_light.radius = 17.0f;
			player_point_light._specular_power = 25.0f;
			player_point_light.bCastShadows = true;ww
			m_Resources->m_Lights->AddPoint(player_point_light);
			*/
		}

		m_Stuff->Update();
		m_Statics->Update(timer.GetElapsedSeconds(), timer.GetTotalSeconds());



		if (m_Resources->m_LevelInfo.bSnow == 1)
		{
			tasks.push_back(m_Snow->Update(timer.GetTotalSeconds(), timer.GetElapsedSeconds()));
		}
			
		if (m_Resources->m_LevelInfo.bFog == 1)
		{
			tasks.push_back(m_Fog->Update(timer.GetTotalSeconds(), timer.GetElapsedSeconds()));
		}


		if (m_Resources->m_LevelInfo.bRain == 1)
		{
			tasks.push_back(m_Rain->Update(timer.GetTotalSeconds(), timer.GetElapsedSeconds()));
			tasks.push_back(m_Splash->Update(timer.GetTotalSeconds(), timer.GetElapsedSeconds()));
		}
		tasks.push_back(m_dirt->Update(timer.GetTotalSeconds(), timer.GetElapsedSeconds()));
		tasks.push_back(m_steam->Update(timer.GetTotalSeconds(), timer.GetElapsedSeconds()));

		return when_all(begin(tasks), end(tasks));

	}).then([this, timer]
	{

		std::vector<concurrency::task<void>> tasks;

		tasks.push_back(m_Snow->CreateVerticies());
		tasks.push_back(m_Fog->CreateVerticies());

		tasks.push_back(m_Rain->CreateVerticies());
		tasks.push_back(m_Splash->CreateVerticies());
		tasks.push_back(m_dirt->CreateVerticies());
		tasks.push_back(m_steam->CreateVerticies());

		tasks.push_back(m_Fire->Update(timer));

		return when_all(begin(tasks), end(tasks));
	}));




	tasks.push_back(m_Level->Update(timer.GetElapsedSeconds(), timer.GetTotalSeconds()).then([timer, this]
	{


		return m_Grass->Update(timer.GetTotalSeconds(), timer.GetElapsedSeconds());

	}));



	//tasks.push_back();

	auto physics_task = concurrency::create_task([this, timer]
	{

		m_Buggy->Update(timer.GetElapsedSeconds(), timer.GetTotalSeconds());
		m_Bert->Update(timer.GetElapsedSeconds(), timer.GetTotalSeconds());
	}).then([this, timer]
	{

		btVector3 bert_pos = btVector3(m_Bert->bert_pos.x, m_Bert->bert_pos.y, m_Bert->bert_pos.z);

		btVector3 bug_pos = *m_Buggy->GetPosition();
		btVector3 bug_dir = *m_Buggy->GetDirection();
		btVector3 bug_up = *m_Buggy->GetUp();

		float px = bug_pos.getX();
		float py = bug_pos.getY();
		float pz = bug_pos.getZ();

		float dx = bug_dir.getX();
		float dy = bug_dir.getY();
		float dz = bug_dir.getZ();

		float ux = bug_up.getX();
		float uy = bug_up.getY();
		float uz = bug_up.getZ();

		if (m_Buggy->bPlayerActive == true)
		{
			if (m_Buggy->bInCarView == true)
			{
				m_PanCamera->SetEyeAtUp(px + (ux*3.0f), py + (uy*3.0f), pz + (uz*3.0f), px + (ux*3.0f) + dx, py + (uy*3.0f) + dy, pz + (uz*3.0f) + dz, ux, uy, uz, true);

			}
			else
			{
				m_PanCamera->SetEyeAt(bug_pos.getX() - (bug_dir.getX()*3.0f), bug_pos.getY() + 3.0f, bug_pos.getZ() - (bug_dir.getZ()*3.0f), bug_pos.getX(), bug_pos.getY() + 2.0f, bug_pos.getZ());
			}
			m_Resources->SetplayerPos(bug_pos.getX(), bug_pos.getY(), bug_pos.getZ());
		}

		if (m_Bert->bPlayerActive == true)
		{
			//m_Resources->m_PanCamera->SetEyeAt(bert_pos.getX(), bert_pos.getY(), bert_pos.getZ(), bert_pos.getX(), bert_pos.getY() + 0.0f, bert_pos.getZ());
			m_PanCamera->SetEyeAt(bert_pos.getX() + (sin(m_Bert->view_angle)*5.0f), bert_pos.getY() + 2.0f+ m_Bert->view_pos_y, bert_pos.getZ() - (cos(m_Bert->view_angle)*5.0f), bert_pos.getX(), bert_pos.getY() + 0.0f, bert_pos.getZ());
			
			//m_Resources->m_PanCamera->SetEyeAt(bert_pos.getX() + (sin(m_Bert->view_angle)*15.0f), bert_pos.getY() + 12.0f, bert_pos.getZ() - (cos(m_Bert->view_angle)*15.0f), bert_pos.getX(), bert_pos.getY() + 0.0f, bert_pos.getZ());

			m_Resources->SetplayerPos(bert_pos.getX(), bert_pos.getY(), bert_pos.getZ());
		}

		//m_Resources->m_Lights->SetEyePosition(m_Resources->m_PanCamera->vEye.getX(), m_Resources->m_PanCamera->vEye.getY(), m_Resources->m_PanCamera->vEye.getZ());

		if (m_Resources->bFreeCamera == false)
			m_PanCamera->Update(timer.GetElapsedSeconds());


		if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::PageUp) == true)
		{
			m_Resources->bFreeCamera = true;
		}
		if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::PageDown) == true)
		{
			m_Resources->bFreeCamera = false;
		}

	});
	tasks.push_back(physics_task);


	auto joinTask = when_all(begin(tasks), end(tasks));

	joinTask.wait();


	m_Grass->UpdateVertexBuffers();
	m_dirt->UpdateVertexBuffers();
	m_Fire->UpdateVertexBuffers();
	m_Snow->UpdateVertexBuffers();
	m_Fog->UpdateVertexBuffers();

	m_Rain->UpdateVertexBuffers();
	m_Splash->UpdateVertexBuffers();
	m_steam->UpdateVertexBuffers();

	/*
	if (set_delay > 0.0f)
	{
	set_delay -= timer.GetElapsedSeconds();
	return;
	}
	*/

	if (m_Resources->bDevelopment == true)
	{
		m_Resources->m_Camera->SetSkyProjection();
		m_LevelEdit->Update(timer);
	}



		//m_Resources->m_Lights->Addpoint(XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f), XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f), XMFLOAT4(0.55f, 0.55f, 0.55f, 1.0f), XMFLOAT3(m_LevelEdit->pos_x, m_LevelEdit->pos_y + 5.0f, m_LevelEdit->pos_z), m_LevelEdit->stretch_width, 10.0f);




		//m_Resources->m_uiControl->SetInfoVal(0, m_Buggy->GetPosition()->getX());
		//m_Resources->m_uiControl->SetInfoVal(1, m_Buggy->GetPosition()->getZ());
		//m_Resources->m_uiControl->SetInfoVal(2, atan2f(m_Buggy->car_point.getX(), m_Buggy->car_point.getZ()));

		//m_Resources->m_uiControl->SetInfoVal(3, m_Bert->GetPosition()->getX());
		//m_Resources->m_uiControl->SetInfoVal(4, m_Bert->GetPosition()->getZ());
		//m_Resources->m_uiControl->SetInfoVal(5, m_Bert->char_angle);
	//}

	/*
	ModelViewProjectionConstantBuffer temp_buffer = m_Resources->m_Camera->m_constantBufferData;
	RenderAlphaMode(0);
	RenderShadows();
	m_deviceResources->RestoreViewportAndRenderTarget();

	m_Resources->m_Camera->m_constantBufferData = temp_buffer;
	*/

	//m_Profiler->Stop();

	//m_Profiler->Display();
}

void GameScene::RenderAlphaMode(int alpha_mode)
{
	switch (alpha_mode)
	{
	case 0:
	case 1:
		m_Resources->DisableBlending();
		m_Resources->SetCull(true);
		m_Resources->SetDepthStencil(true);
		break;
	
		//m_Resources->EnableBlendingTexAlph();
		//m_Resources->SetDepthStencil(true);
		//m_Resources->SetCull(true);
		//break;
	case 2:
		m_Resources->EnableBlendingTexAlph();
		//m_Resources->EnableBlendingGlassAlph();
		m_Resources->SetDepthStencil(false);
		m_Resources->SetCull(false);
		break;
	case 3:
		m_Resources->EnableBlendingSkyplane();
		//m_Resources->EnableBlendingGlassAlph();
		m_Resources->SetDepthStencil(false);
		m_Resources->SetCull(false);
		break;
	}
}

void GameScene::RenderShadows()
{
	int i, j;

	m_Resources->m_Camera->SetCubeProjection();
	if (true)
		for (j = 0; j<m_Resources->m_Lights->current_point_shadow_index; j++)
		{
			m_Resources->m_Lights->setPointFrustumPlanePosition(j);
			m_Resources->m_Lights->m_RenderCubeArray->ClearTargets();

			//m_Resources->m_Lights->setPointFrustumPlanePosition(j);
			//m_Resources->m_Camera->m_constantBufferData.view = m_Resources->m_Camera->MakeViewParams(XMFLOAT3(m_Resources->m_Lights->m_RenderCube[j]->render_position.x, m_Resources->m_Lights->m_RenderCube[j]->render_position.y, m_Resources->m_Lights->m_RenderCube[j]->render_position.z), XMFLOAT3(0.0f,-1.0f,0.0f), XMFLOAT3(0.0f,1.0f,0.0f));
			//m_Resources->m_Camera->UpdateConstantBuffer();
			//m_Resources->m_Lights->ClearSpotCubeDepth();
			for (i = 0; i < 6; i++)
			{
				m_Resources->m_Lights->RenderPointDepthCubSide(i, j);

				//m_Resources->m_Lights->m_lightBufferData.dirProj = m_Resources->m_Camera->close_projection;
				//m_Resources->m_Lights->UpdateConstantBuffer();
				//m_Statics->RenderDepth(0,i);
				//m_Stuff->RenderDepth(0, i);
				if (true)//(i == 3)
				{
					m_Buggy->RenderDepth(0, i);
					m_Resources->SetSkinDepthShader();
					m_Bert->RenderDepth(0, i);

					m_Resources->SetDepthShader();
					m_Statics->RenderDepth(0, i);
					m_Stuff->RenderDepth(0, i);


					//m_Level->Render();

					if (true)
					{
						m_Resources->SetDepthAlphaShader();
						m_Statics->RenderDepth(1, i);
						m_Statics->RenderDepth(2, i);

						//m_Grass->Render();
					}
					//m_Level->Render();
				}
			}
		}

	if(false)
	{

		for (i = 0; i < m_Resources->m_Lights->current_spot_shadow_index; i++)
		{
			m_TestTexture->SetRenderTarget(m_deviceResources->GetD3DDeviceContext());
			m_TestTexture->ClearRenderTarget(m_deviceResources->GetD3DDeviceContext(), 1.0f, 0.0f, 0.0f, 1.0f);

			m_Resources->m_Camera->UpdateConstantBuffer(m_Resources->m_Lights->spot_shadow[i].dirProj,
														m_Resources->m_Lights->spot_shadow[i].dirView);


			m_Resources->m_Lights->m_SpotTextureArray->SetRenderTarget(i, m_deviceResources->GetD3DDeviceContext());

			m_Resources->m_Lights->m_SpotTextureArray->ClearRenderTarget(i, m_deviceResources->GetD3DDeviceContext(), 1.0f, 0.0f, 0.0f, 1.0f);

			m_Resources->m_Camera->m_constantBufferData.projection = m_Resources->m_Lights->spot_shadow[i].dirProj;
			m_Resources->m_Camera->m_constantBufferData.view = m_Resources->m_Lights->spot_shadow[i].dirView;
			m_Resources->m_Camera->UpdateConstantBuffer();
			//m_Resources->m_Lights->UpdateConstantBuffer();

			m_Resources->SetDepthShader();
			
			RenderAlphaMode(0);
			//m_Bert->RenderDepth(0);
			//m_Statics->RenderDepth(0);
			//m_Stuff->RenderDepth(0);

		}
		
		
		//m_Statics->Render(0, true);
		//m_Stuff->Render(0);
		//m_Statics->Render(1, true);
		//m_Stuff->Render(1);


	}

	m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(6, 1, m_TestTexture->GetShaderResourceView().GetAddressOf());

	m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(60, m_Resources->m_Lights->current_point_shadow_index, m_Resources->m_Lights->m_RenderCubeArray->m_shaderResourceView);

	m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(100, m_Resources->m_Lights->current_spot_shadow_index, m_TestTexture->GetShaderResourceView().GetAddressOf());
	/*
	m_TestTexture->SetRenderTarget(m_deviceResources->GetD3DDeviceContext());
	m_TestTexture->ClearRenderTarget(m_deviceResources->GetD3DDeviceContext(), 1.0f, 0.0f, 0.0f, 1.0f);
	m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(4, 1, m_TestTexture->GetShaderResourceView().GetAddressOf());
	*/
	//m_Resources->m_deviceResources->GetD3DDeviceContext()->VSSetShaderResources(100, MAX_SPOT_SHADOWS, m_Resources->m_Lights->m_SpotTextureArray->m_shaderResourceView);
	//m_Resources->m_deviceResources->GetD3DDeviceContext()->VSSetShaderResources(60, MAX_POINT_SHADOWS, m_Resources->m_Lights->m_RenderCubeArray->m_pointshadowMapShaderResourceView);

}

void GameScene::RenderSkyCubeMap()
{
	int i;
	if (true)
	{
		m_Resources->SetSkyShader();
		m_Resources->m_Camera->SetSkyCubeProjection();
		m_MetalBertCube->SetRenderPosition(0.0f, 0.0f, 0.0f);

		if (m_Sky->bEnvironmentCreated == false)
		{
			for (i = 0; i < 6; ++i)
			{
				m_Sky->m_SkyCubeMap->SetRenderPosition(0.0f, 0.0f, 0.0f);
				XMStoreFloat4x4(&m_Resources->m_Camera->m_constantBufferData.view, XMMatrixTranspose(m_Sky->m_SkyCubeMap->RenderCubeSide(i, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f))));

				if (true)
				{
					RenderAlphaMode(0);
					m_Resources->SetSkyShader();
					m_Sky->RenderCenter();
				}
			}
			m_Sky->bEnvironmentCreated = true;
		}

		//m_Resources->m_deviceResources->GetD3DDeviceContext()->CopyResource((ID3D11Resource*)m_Sky->m_SkyPlaneCubeMap->m_pointshadowMapTexture.Get(), (ID3D11Resource*)m_Sky->m_SkyCubeMap->m_pointshadowMapTexture.Get());
		
		if (true)
		{
			RenderAlphaMode(3);
			for (i = 0; i < 6; ++i)
			{
				if (i == 3)
					continue;  // dont need to render bottom plane nothing there

				m_Sky->m_SkyPlaneCubeMap->SetRenderPosition(0.0f, 0.0f, 0.0f);
				XMStoreFloat4x4(&m_Resources->m_Camera->m_constantBufferData.view, XMMatrixTranspose(m_Sky->m_SkyPlaneCubeMap->RenderCubeSide(i, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f))));
				if (true)
				{
					m_Resources->SetSkyplaneShader();
					m_Skyplane->RenderCenter();
				}
			}
		}
	}

	//m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(5, 1, m_Sky->m_SkyCubeMap->GetShaderResourceView().GetAddressOf());


}

void GameScene::RenderGlowTexture()
{
	//m_deviceResources->get
	
	// copy the depth buffer
	//m_Resources->m_deviceResources->GetD3DDeviceContext()->CopyResource((ID3D11Resource*)m_GlowTexture->m_environmentMapDepthStencilTexture.Get(), (ID3D11Resource*)m_deviceResources->depthStencil.Get());


	m_GlowTexture->SetRenderTarget(m_deviceResources->GetD3DDeviceContext(), m_deviceResources->GetDepthStencilView());

	m_GlowTexture->ClearRenderTarget(m_deviceResources->GetD3DDeviceContext(), 0.0f, 0.0f, 0.0f, 0.0f);
	
	m_Resources->m_Camera->SetCloseProjection();
	m_Resources->m_Camera->UpdateConstantBuffer();
	m_Resources->SetGlassShader();
	m_Resources->SetDepthStencil(false);

	m_Statics->Render(2, false);
	m_Stuff->Render(2);

	if (false)
	{
		m_Resources->SetQuadIndexBuffer();
		m_Resources->SetFireShader();
		//m_Resources->SetDepthStencil(false);
		m_Fire->Render();
	}
	m_deviceResources->GetD3DDeviceContext()->GenerateMips(m_GlowTexture->GetShaderResourceView().Get());
	// do the bluring
	m_Resources->m_Camera->UpdateConstantBufferOrth();

	m_GlowTextureDownsample->SetRenderTarget(m_deviceResources->GetD3DDeviceContext());
	m_GlowTextureDownsample->ClearRenderTarget(m_deviceResources->GetD3DDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	RenderAlphaMode(0);
	m_Resources->SetGlowShader();
	m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_GlowTexture->GetShaderResourceView().GetAddressOf());

	m_Resources->m_FullScreenWindow->Render(m_Resources->m_deviceResources->GetD3DDeviceContext());

	//m_deviceResources->GetD3DDeviceContext()->GenerateMips(m_GlowTextureDownsample->GetShaderResourceView().Get());

	m_GlowTextureDownsample2->SetRenderTarget(m_deviceResources->GetD3DDeviceContext());
	m_GlowTextureDownsample2->ClearRenderTarget(m_deviceResources->GetD3DDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	m_Resources->SetHblurShader();

	m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_GlowTexture->GetShaderResourceView().GetAddressOf());
	m_Resources->m_FullScreenWindow->Render(m_Resources->m_deviceResources->GetD3DDeviceContext());

	//m_deviceResources->GetD3DDeviceContext()->GenerateMips(m_GlowTextureDownsample2->GetShaderResourceView().Get());

	m_GlowTextureDownsample->SetRenderTarget(m_deviceResources->GetD3DDeviceContext());
	m_GlowTextureDownsample->ClearRenderTarget(m_deviceResources->GetD3DDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	m_Resources->SetVblurShader();

	m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_GlowTextureDownsample2->GetShaderResourceView().GetAddressOf());
	m_Resources->m_FullScreenWindow->Render(m_Resources->m_deviceResources->GetD3DDeviceContext());

	//m_deviceResources->GetD3DDeviceContext()->GenerateMips(m_GlowTextureDownsample->GetShaderResourceView().Get());
	// extra blur passes
	for (int i = 0; i < 4; i++)
	{
		m_GlowTextureDownsample2->SetRenderTarget(m_deviceResources->GetD3DDeviceContext());
		m_GlowTextureDownsample2->ClearRenderTarget(m_deviceResources->GetD3DDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

		m_Resources->SetHblurShader();

		m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_GlowTextureDownsample->GetShaderResourceView().GetAddressOf());
		m_Resources->m_FullScreenWindow->Render(m_Resources->m_deviceResources->GetD3DDeviceContext());

		//m_deviceResources->GetD3DDeviceContext()->GenerateMips(m_GlowTextureDownsample2->GetShaderResourceView().Get());

		m_GlowTextureDownsample->SetRenderTarget(m_deviceResources->GetD3DDeviceContext());
		m_GlowTextureDownsample->ClearRenderTarget(m_deviceResources->GetD3DDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

		m_Resources->SetVblurShader();

		m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_GlowTextureDownsample2->GetShaderResourceView().GetAddressOf());
		m_Resources->m_FullScreenWindow->Render(m_Resources->m_deviceResources->GetD3DDeviceContext());

		//m_deviceResources->GetD3DDeviceContext()->GenerateMips(m_GlowTextureDownsample->GetShaderResourceView().Get());
	}
}

void GameScene::RenderParticles()
{
	m_ParticleTexture->SetRenderTarget(m_deviceResources->GetD3DDeviceContext(), m_deviceResources->GetDepthStencilView());

	m_ParticleTexture->ClearRenderTarget(m_deviceResources->GetD3DDeviceContext(), 0.0f, 0.0f, 0.0f, 0.0f);

	m_Resources->m_Camera->SetCloseProjection();
	m_Resources->m_Camera->UpdateConstantBuffer();

	RenderAlphaMode(0);

	if (m_Resources->m_LevelInfo.bSnow == 1)
	{
		m_Resources->SetCull(false);
		m_Resources->SetInsPointShader();
		m_Snow->Render();
		m_Resources->SetPointShader();
	}

	m_Resources->m_FullScreenWindow->Render(m_Resources->m_deviceResources->GetD3DDeviceContext());
}


// Renders one frame using the vertex and pixel shaders.
void GameScene::Render()
{
	if (true)
	{
		RenderDeferred();
		return;
	}

	int i,j;
	m_Profiler->Start();
	if (m_Resources->shaders_loading.is_done() == false)
	{
		return;
	}
	if (bLoadedPhysicsDelay == false || bLoaded == false)
		return;


	if (bMusicStarted == false)
	{
		bMusicStarted = true;
		if (START_MUSIC == 1)
		{
			m_Resources->m_audio.Start();
		}
	}

	if (m_Resources->bAudioInitialized == true)
		m_Resources->m_audio.Render();


	m_Resources->m_Lights->UpdateConstantBuffer();
	
	ModelViewProjectionConstantBuffer temp_buffer = m_Resources->m_Camera->m_constantBufferData;

	RenderAlphaMode(0);
	RenderShadows();
	
	RenderSkyCubeMap();

	//m_Resources->m_Lights->RestoreDepthViewToSpotView();

	m_deviceResources->RestoreViewportAndRenderTarget();


	m_Resources->m_Camera->m_constantBufferData = temp_buffer;

	
	



	//m_Resources->m_deviceResources->GetD3DDeviceContext()->ClearRenderTargetView()

	m_deviceResources->GetD3DDeviceContext()->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	m_Resources->m_Camera->SetCloseProjection();
	m_Resources->m_Camera->UpdateConstantBuffer();
	m_Resources->m_Lights->UpdateConstantBuffer();
	m_Resources->SetShinyShader();
	m_Resources->SetDepthStencil(true);
	RenderAlphaMode(0);



	m_Bert->Render(0,false);
	m_Buggy->Render();




	//m_LevelEdit->Render(0);
	m_Statics->Render(0,false);
	m_Stuff->Render(0);
	m_Statics->Render(1,false);
	m_Stuff->Render(1);

	if(m_Resources->bDevelopment==true)
		m_LevelEdit->Render();

	m_Resources->SetGroundShader();

	m_Level->Render();

	RenderAlphaMode(2);

	m_Resources->SetWaterShader();
	m_Level->RenderWater();

	m_Resources->SetDepthStencil(true);
	m_Resources->SetSkyShader();
	m_Resources->m_Camera->SetSkyProjection();
	m_Sky->Render();

	m_Resources->SetCull(true);
	m_Skyplane->Render();

	m_Resources->SetQuadIndexBuffer();
	m_Resources->SetFireShader();
	//m_Resources->SetDepthStencil(false);
	m_Fire->Render();

	
	m_Resources->SetPointShader();

	m_Resources->SetDepthStencil(false);

	if (m_Resources->m_LevelInfo.bRain == 1)
	{
		m_Rain->Render();
		m_Splash->Render();
	}

	m_dirt->Render();
	m_steam->Render();
	m_Grass->Render();
	if (m_Resources->m_LevelInfo.bSnow == 1)
	{
		m_Snow->Render();
	}

	if (m_Resources->m_LevelInfo.bFog == 1)
	{
		m_Fog->Render();
	}

	m_Resources->SetShinyShader();
	m_Resources->SetDepthStencil(false);

	RenderAlphaMode(2);
	if (m_Resources->bDevelopment == true)
		m_LevelEdit->RenderGSphere();


	m_Resources->SetShinyShader();
	m_Statics->Render(2,false);
	m_Stuff->Render(2);
	m_Bert->Render(2,false);

	m_Profiler->Stop();

	m_Profiler->Display();

}



// Renders one frame using the vertex and pixel shaders.
void GameScene::RenderDeferred()
{
	int i, j;
	m_Profiler->Start();
	if (m_Resources->shaders_loading.is_done() == false)
	{
		return;
	}
	if (bLoadedPhysicsDelay == false || bLoaded == false)
		return;


	if (bMusicStarted == false)
	{
		bMusicStarted = true;
		if (START_MUSIC == 1)
		{
			m_Resources->m_audio.Start();
		}
	}

	if (m_Resources->bAudioInitialized == true)
		m_Resources->m_audio.Render();

	m_Resources->m_Lights->UpdateConstantBuffer();


	m_Resources->m_Camera->UpdateConstantBuffer();
	m_deviceResources->RestoreViewportAndRenderTarget();
	ModelViewProjectionConstantBuffer temp_buffer = m_Resources->m_Camera->m_constantBufferData;

	RenderAlphaMode(0);
	RenderShadows();

	RenderSkyCubeMap();

	

	//RenderGlowTexture();

	//m_Resources->m_Lights->RestoreDepthViewToSpotView();

	m_deviceResources->RestoreViewportAndRenderTarget();


	m_Resources->m_Camera->m_constantBufferData = temp_buffer;




	//m_Resources->m_deviceResources->GetD3DDeviceContext()->ClearRenderTargetView()

	m_deviceResources->GetD3DDeviceContext()->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	m_Resources->m_Camera->SetCloseProjection();
	m_Resources->m_Camera->UpdateConstantBuffer();
	m_Resources->SetShinyShader();
	m_Resources->SetDepthStencil(true);
	RenderAlphaMode(0);


	m_Resources->SetDeferredShader();

	m_Resources->m_DeferredBuffers->SetRenderTargets(m_Resources->m_deviceResources->GetD3DDeviceContext(), m_deviceResources->GetDepthStencilView());
	m_Resources->m_DeferredBuffers->ClearRenderTargets(m_Resources->m_deviceResources->GetD3DDeviceContext(), 0.0f, 0.0f, 0.0f, 0.0f);

	m_Resources->SetDefSkinShader();
	m_Bert->Render(0,true);
	//m_Bert->Render(0);

	m_Resources->SetDeferredShader();
	m_Buggy->Render();

	m_Statics->Render(0,true);
	m_Stuff->Render(0);
	m_Resources->SetCull(true);
	m_Statics->Render(1,true);
	m_Stuff->Render(1);


	m_Resources->SetDefPointShader();
	m_Resources->SetQuadIndexBuffer();
	RenderAlphaMode(0);
	m_Resources->SetCull(false);
	m_Grass->Render();

	m_Resources->SetCull(true);
	m_Resources->SetDefGroundShader();
	RenderAlphaMode(0);
	m_Level->Render();






	m_Resources->SetDepthStencil(false);

	RenderGlowTexture();
	//RenderParticles();

	//m_graphics.GetCamera()->SetViewParams(XMFLOAT3(0.0f, 0.0f, 35.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_Resources->m_Camera->UpdateConstantBufferOrth();


// Downsample Glow



	//m_Resources->SetGlowShader();

	//m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_GlowTexture->GetShaderResourceView().GetAddressOf());
	//m_Resources->m_FullScreenWindow->Render(m_Resources->m_deviceResources->GetD3DDeviceContext());

	m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(5, 1, m_Sky->m_SkyPlaneCubeMap->GetShaderResourceView().GetAddressOf());

	//m_deviceResources->RestoreViewportAndRenderTarget();
	m_Resources->SetDepthStencil(false);
	m_DeferredTexture->SetRenderTarget(m_deviceResources->GetD3DDeviceContext(), m_deviceResources->GetDepthStencilView());
	m_DeferredTexture->ClearRenderTarget(m_deviceResources->GetD3DDeviceContext(), 0.0f, 0.0f, 0.0f, 0.0f);

	m_Resources->SetLightShader();
	m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_Resources->m_DeferredBuffers->GetShaderResourceView(0).GetAddressOf());
	m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(1, 1, m_Resources->m_DeferredBuffers->GetShaderResourceView(1).GetAddressOf());
	m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(2, 1, m_Resources->m_DeferredBuffers->GetShaderResourceView(2).GetAddressOf());
	m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(3, 1, m_Resources->m_DeferredBuffers->GetShaderResourceView(3).GetAddressOf());
	m_Resources->m_FullScreenWindow->Render(m_Resources->m_deviceResources->GetD3DDeviceContext());
	

	RenderAlphaMode(0);
	m_Resources->SetDepthStencil(false);
	m_Resources->SetSkyShader();
	m_Resources->m_Camera->SetSkyProjection();
	m_Sky->Render();
	RenderAlphaMode(3);
	m_Resources->SetCull(true);
	m_Resources->m_deviceResources->GetD3DDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	m_Skyplane->Render();
	m_Resources->m_deviceResources->GetD3DDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	RenderAlphaMode(2);
	m_Resources->m_Camera->SetCloseProjection();
	m_deviceResources->RestoreViewportAndRenderTarget();
	//m_Resources->SetDepthStencil(false);
	//m_Resources->SetWaterShader();
	//m_Level->RenderWater();


	//m_Resources->m_Camera->UpdateConstantBufferOrth();

	if (true)//(rand() % 2 == 1)
	{
		m_deviceResources->RestoreViewportAndRenderTarget();
		m_Resources->m_Camera->UpdateConstantBufferOrth();
		m_Resources->SetDepthStencil(false);
		m_Resources->SetGlowMapShader();
		m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_DeferredTexture->GetShaderResourceView().GetAddressOf());
		m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(1, 1, m_GlowTextureDownsample->GetShaderResourceView().GetAddressOf());
		m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(2, 1, m_GlowTexture->GetShaderResourceView().GetAddressOf());

		m_Resources->m_FullScreenWindow->Render(m_Resources->m_deviceResources->GetD3DDeviceContext());
	}
	

	


	RenderAlphaMode(2);

	m_Resources->SetWaterShader();
	m_Level->RenderWater();



	//m_Resources->SetDepthStencil(true);
	//m_Resources->SetSkyShader();
	//m_Resources->m_Camera->SetSkyProjection();
	//m_Sky->Render();

	//m_Resources->SetCull(false);
	//m_Skyplane->Render();

	m_Resources->m_Camera->SetCloseProjection();


	m_Resources->SetQuadIndexBuffer();
	m_Resources->SetFireShader();
	//m_Resources->SetDepthStencil(false);
	m_Fire->Render();

	if (true)//(m_Resources->bCPULightParticles == true)
	{
		m_Resources->SetPointShader();
	}
	else
	{
		m_Resources->SetPointLitShader();
	}

	m_Resources->SetDepthStencil(false);

	if (m_Resources->m_LevelInfo.bRain == 1)
	{
		m_Rain->Render();
		m_Splash->Render();
	}
	
	m_dirt->Render();
	m_steam->Render();

	m_Resources->SetCull(false);
	m_Resources->SetInsPointShader();
	if (m_Resources->m_LevelInfo.bSnow == 1)
	{
		m_Snow->Render();
	}
	
	if (m_Resources->m_LevelInfo.bFog == 1)
	{
		m_Fog->Render();
	}
	m_Resources->SetPointShader();

	
	m_Resources->SetShinyShader();
	m_Resources->SetDepthStencil(false);
	m_Resources->SetCull(true);
	RenderAlphaMode(2);
	if (m_Resources->bDevelopment == true)
		m_LevelEdit->RenderGSphere();
	if (m_Resources->bDevelopment == true)
		m_LevelEdit->Render();

	m_Resources->SetGlassShader();
	m_Statics->Render(2,true);
	m_Stuff->Render(2);


	m_Resources->SetDepthStencil(false);
	
	if (false)
	{
		m_Resources->m_Camera->UpdateConstantBufferOrth();
		m_deviceResources->RestoreViewportAndRenderTarget();
		RenderAlphaMode(0);
		m_Resources->SetGlowMapShader();
		m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_DeferredTexture->GetShaderResourceView().GetAddressOf());
		m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(1, 1, m_GlowTextureDownsample->GetShaderResourceView().GetAddressOf());
		m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(2, 1, m_GlowTexture->GetShaderResourceView().GetAddressOf());

		m_Resources->m_FullScreenWindow->Render(m_Resources->m_deviceResources->GetD3DDeviceContext());
	}
	
	/*

	m_deviceResources->RestoreViewportAndRenderTarget();

	RenderAlphaMode(0);
	m_Resources->SetGlowShader();
	m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(0, 1, m_DeferredTexture->GetShaderResourceView().GetAddressOf());
	m_Resources->m_deviceResources->GetD3DDeviceContext()->PSSetShaderResources(1, 1, m_GlowTextureDownsample->GetShaderResourceView().GetAddressOf());

	m_Resources->m_FullScreenWindow->Render(m_Resources->m_deviceResources->GetD3DDeviceContext());
	*/

	m_Profiler->Stop();

	m_Profiler->Display();

}
