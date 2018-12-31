#pragma once





namespace Game
{
	class LevelEdit
	{
	public:
		LevelEdit(AllResources* p_Resources, Level* pp_Level, Statics* pp_Statics, Stuff* pp_Stuff);
		~LevelEdit(void);


		void Update(DX::StepTimer const& timer);
		void Render();

		void MakePath();
		void ClearPaths();
		void NewPath();


		void SetBlendNum(int _num);

		bool bLoopPath;
		bool bDipPath;

		bool bMakePath;
		bool bClearPaths;
		bool bNewPath;

		bool bGenerateTerrain;


		float stretch_height;
		float stretch_width;

		bool bMouseClicked;

		float fixed_height;

		float last_place_x, last_place_z;

		float pos_x, pos_y, pos_z;

		float pointer_scale;
		float pointer_rot_y;
		float pointer_rot_x;
		float pointer_rot_z;
		float pointer_above;

		float render_point_x;
		float render_point_y;
		float render_point_z;

		float t1val;
		float t2val;
		float t3val;
		float t4val;

		//FBXModelLoader* g_sphere;
		//ModelLoader* m_Marker;

		std::vector<Mesh*> g_sphere;
		std::vector<Mesh*> m_Marker;


		int design_spacing;

		int current_blend_edit;


		bool bLoaded;

		bool bEditing;

		bool bColKeyDown;;

		bool bPointerPressed;

		void SetStaticsHeightToTerrrain();

		void SetEditing(int ed);// { current_editing = ed; }
		void RenderGSphere();
		int current_editing;

		DirectX::XMFLOAT3 pos; // cursor pos
		DirectX::XMFLOAT3 last_pos; // cursor pos

		Windows::Foundation::Point mouse_point;

	private:
		AllResources* m_Resources;
		Level* p_Level;
		Statics* p_Statics;
		Stuff* p_Stuff;
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		
	};


};

