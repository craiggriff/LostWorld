#pragma once
namespace Game
{


	class GameLogic
	{
	public:
		GameLogic(AllResources* p_Resources,GameScene* pp_GameScene);

		void Update(DX::StepTimer const& timer);
		void PhysicsCollisions();


		void SetNewPlayerState(int _state);
		
		btVector3 wheel_last_contact[4];

		concurrency::task<void> GameLogic::HTTPStreamingAsync();
		bool bStreamComplete;

		void SetPlayerState();

		float frame_pos;

		int current_player_state;

		int current_level;

		bool bChangePressed;
		bool bInCarPressed;

		AllResources* m_Resources;
		
		GameScene* m_Scene;

	};
};
