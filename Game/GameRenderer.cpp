#include "pch.h"
#include "GameRenderer.h"

#include "..\Common\DirectXHelper.h"
#include "BasicLoader.h"

using namespace Game;

using namespace DirectX;
using namespace Windows::Foundation;
using namespace concurrency;
using namespace Windows::System::Threading;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
SceneRenderer::SceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources, Game::IGameUIControl^ UIControl, DX::StepTimer* pp_timer) :
	m_uiControl(UIControl),
	m_loadingComplete(false),
	m_deviceResources(deviceResources)
{
	bLoadingComplete = false;

	m_Resources = new AllResources(deviceResources, m_uiControl, pp_timer);
	m_Scene = new GameScene(m_Resources);
	m_Logic = new GameLogic(m_Resources, m_Scene);


	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
	
	bLoadingComplete = true;
	render_delay = 0.0f;
}

// Initializes view parameters when the window size changes.
void SceneRenderer::CreateWindowSizeDependentResources()
{
	Windows::Foundation::Size outputSize = m_deviceResources->GetOutputSize();

	//m_Scene->bLoaded = false;
	//m_Scene->bLoadedPhysicsDelay = false;

	m_Resources->m_Camera->CreateWindowSizeDependentResources();

	ScreenSizeBufferType ssize;

	ssize.height_size = outputSize.Height/ GLOW_BLUR;
	ssize.width_size = outputSize.Width/ GLOW_BLUR;

	m_Resources->UpdateScreenSizeConstants(ssize);
	//delete m_Resources->m_FullScreenWindow;
	//m_Resources->m_DeferredBuffers->Shutdown();

	// Initialize the full screen ortho window object.
	m_Resources->m_FullScreenWindow->Initialize(m_deviceResources->GetD3DDevice(), (UINT)outputSize.Width, (UINT)outputSize.Height);

	m_Resources->m_DownsampleWindow->Initialize(m_deviceResources->GetD3DDevice(), (UINT)outputSize.Width / GLOW_BLUR, (UINT)outputSize.Height / GLOW_BLUR);

	

	// Initialize the deferred buffers object.
	m_Resources->m_DeferredBuffers->Initialize(m_deviceResources->GetD3DDevice(), (UINT)outputSize.Width, (UINT)outputSize.Height, 400.0f, 0.1f);

	m_Scene->CreateWindowSizeDependentResources();

}





// Called once per frame, rotates the cube and calculates the model and view matrices.
void SceneRenderer::Update(DX::StepTimer const& timer)
{
	if (bLoadingComplete == false)
		return;


	///auto physics_task = concurrency::create_task([this, timer]
	//{
	//	GetResources()->m_Physics.Update(timer.GetElapsedSeconds(), timer.GetTotalSeconds());
	//});

	if (render_delay > 0.0f)
	{
		render_delay -= timer.GetElapsedSeconds();
	}
	else
	{
		if (m_Scene->bLoaded == true)
		{

			std::vector<concurrency::task<void>> tasks;

			//auto physics_task = concurrency::create_task([this, timer]
			//{
			//m_Logic->PhysicsCollisions();
			m_Scene->CheckNumSaveLoad(Windows::System::VirtualKey::Number1, 1);
			m_Scene->CheckNumSaveLoad(Windows::System::VirtualKey::Number2, 2);
			m_Scene->CheckNumSaveLoad(Windows::System::VirtualKey::Number3, 3);
			m_Scene->CheckNumSaveLoad(Windows::System::VirtualKey::Number4, 4);
			m_Scene->CheckNumSaveLoad(Windows::System::VirtualKey::Number5, 5);
			m_Scene->CheckNumSaveLoad(Windows::System::VirtualKey::Number6, 6);
			m_Scene->CheckNumSaveLoad(Windows::System::VirtualKey::Number7, 7);
			m_Scene->CheckNumSaveLoad(Windows::System::VirtualKey::Number8, 8);
			m_Scene->CheckNumSaveLoad(Windows::System::VirtualKey::Number9, 9);
			m_Scene->CheckNumSaveLoad(Windows::System::VirtualKey::Number0, 10);

			if (m_Scene->bLoadedPhysicsDelay == true)
			{
				//m_Logic->PhysicsCollisions();
				m_Scene->m_Resources->m_Physics.Update(timer.GetElapsedSeconds(), timer.GetTotalSeconds());
			}
				//m_Scene->m_Buggy->Update(timer.GetElapsedSeconds(), timer.GetTotalSeconds());
				//m_Scene->m_Bert->Update(timer.GetElapsedSeconds(), timer.GetTotalSeconds());
				
			//});
			//tasks.push_back(physics_task);

			auto other_task = concurrency::create_task([this, timer]
			{
				m_Resources->Update(timer);
				m_Logic->Update(timer);
				m_Scene->Update(timer);
			});
			tasks.push_back(other_task);

			auto joinTask = when_all(begin(tasks), end(tasks));

			joinTask.wait();
		}
	}
}


// Renders one frame using the vertex and pixel shaders.
void SceneRenderer::Render()
{
	if (bLoadingComplete == false)
		return;
	// return;
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}
	if (!(render_delay > 0.0f))
	{
		m_Scene->Render();
	}
	else
	{

	}
}

void SceneRenderer::CreateDeviceDependentResources()
{
	// create shaders and buffers
	if (m_loadingComplete == false)
	{
		m_Resources->CreateDeviceDependentResources();
	}
	m_loadingComplete = true;
}

void SceneRenderer::ReleaseDeviceDependentResources()
{

	m_Resources->ReleaseDeviceDependentResources();

	m_loadingComplete = false;

	//m_constantBuffer.Reset();

}