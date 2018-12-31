#pragma once



namespace Game
{
	class MotionObject : public btMotionState
	{
	public:
		MotionObject(std::shared_ptr<DX::DeviceResources> pm_deviceResources, Physics* phys);
		~MotionObject();

		void GetMatrix(DirectX::XMFLOAT4X4* p_ModelMatrix);
		void GetMatrixOffsetScale(DirectX::XMFLOAT4X4* p_ModelMatrix, float x, float y, float z, float _scale);

		void getWorldTransform(btTransform &worldTransform) const;
		void setWorldTransform(const btTransform &worldTransform);

		void ResetToPosition(DirectX::XMFLOAT3 pos);

		void SetPosition(float x, float y, float z, float yaw, float pitch, float roll);
		void SetPositionnw(float x, float y, float z, float yaw, float pitch, float roll);

		DirectX::XMFLOAT4X4 m_Matrix;

		int item_id;

		ObjInfo ob_info;

		btVector3 last_velocity;
		btVector3 last_angular_velocity;
		bool big_change_in_velocity;

		btTransform m_initialTransform;

		btQuaternion q_rotation;

		Physics* m_phys;

		btRigidBody* m_rigidbody;

		ModelLoader* m_model;

		btRigidBody* MakePhysicsCompoundBoxFromMesh(ModelLoader* p_model, float x, float y, float z);
		btRigidBody* MakePhysicsBoxFromMesh(ModelLoader* p_model, float _scale = 1.0f);
		btRigidBody* MakePhysicsSphereFromMesh(ModelLoader* p_model, float _scale = 1.0f);

		btRigidBody* MakePhysicsBoxFromFBX(FBXModelLoader* p_model, float _scale);

		ID3D11ShaderResourceView* m_Texture;

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

	};

}