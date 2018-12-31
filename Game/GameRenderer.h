#pragma once

#include "..\Common\DeviceResources.h"
#include "Content/ShaderStructures.h"
#include "..\Common\StepTimer.h"

#include "GameUIControl.h"
#include <ppltasks.h>
#include <agile.h>
#include <concrt.h>

#include "AllResources.h"
#include "GameScene.h"
#include "GameLogic.h"

namespace Game
{
	// This sample renderer instantiates a basic rendering pipeline.
	class SceneRenderer
	{
	public:
		SceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources, Game::IGameUIControl^ UIControl,DX::StepTimer* pp_timer);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();
		
		AllResources* GetResources() { return m_Resources; };

		GameScene* m_Scene;

		GameLogic* m_Logic;


	private:
		float render_delay; // 
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		AllResources* m_Resources;


		Game::IGameUIControl^ m_uiControl;
		bool bLoadingComplete;
		// Variables used with the rendering loop.
		bool	m_loadingComplete;
	};
}

