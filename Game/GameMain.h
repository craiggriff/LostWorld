#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "GameRenderer.h"

#include "GameUIControl.h"

#include <ppltasks.h>

// Renders Direct2D and 3D content on the screen.
namespace Game
{
	class GameMain : public DX::IDeviceNotify
	{
	public:
		GameMain(const std::shared_ptr<DX::DeviceResources>& deviceResources, Game::IGameUIControl^ UIControl);
		~GameMain();
		void CreateWindowSizeDependentResources();

		void TrackingUpdate(float positionX) { m_pointerLocationX = positionX; }

		void StartRenderLoop();
		void StopRenderLoop();
		Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

		// TODO: Replace with your own content renderers.
		std::unique_ptr<SceneRenderer> m_sceneRenderer;

		//CpuClass* m_Cpu;

	private:

		void Update();
		bool Render();

		GameMain::IGameUIControl^                   m_uiControl;

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;



		Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
		Concurrency::critical_section m_criticalSection;

		// Rendering loop timer.
		DX::StepTimer m_timer;

		// Track current input pointer position.
		float m_pointerLocationX;
	};
}