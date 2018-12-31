#pragma once

#include "mesh.h"

namespace Game
{
	class MeshMotion : public btMotionState
	{
	public:
		MeshMotion(std::shared_ptr<DX::DeviceResources> pm_deviceResources, Physics* phys, ObjInfo* pob_info = nullptr);
		~MeshMotion();

		void MakeMatrix();
		void GetMatrix(DirectX::XMFLOAT4X4* p_ModelMatrix);
		void GetMatrixOffsetScale(DirectX::XMFLOAT4X4* p_ModelMatrix, float x, float y, float z, float _scale);

		void getWorldTransform(btTransform &worldTransform) const;
		void setWorldTransform(const btTransform &worldTransform);

		void ResetToPosition(DirectX::XMFLOAT3 pos);

		void SetPosition(float x, float y, float z, float yaw = 0.0f, float pitch = 0.0f, float roll = 0.0f);
		void SetPositionnw(float x, float y, float z, float yaw, float pitch, float roll);

		DirectX::XMFLOAT4X4 m_Matrix;

		int item_id;

		float cam_dist;
		float dist;
		int collision_range;

		int static_index;

		ObjInfo ob_info;

		btVector3 last_velocity;
		btVector3 last_angular_velocity;
		bool big_change_in_velocity;

		btTransform m_initialTransform;

		btQuaternion q_rotation;

		Physics* m_phys;

		btRigidBody* m_rigidbody;

		Mesh* m_model;

		//btRigidBody* MakePhysicsCompoundBoxFromMesh(ModelLoader* p_model, float x, float y, float z);
		//btRigidBody* MakePhysicsBoxFromMesh(ModelLoader* p_model, float _scale = 1.0f);
		//btRigidBody* MakePhysicsSphereFromMesh(ModelLoader* p_model, float _scale = 1.0f);
		btRigidBody* MakePhysicsSquareplane(float size, float y_offset = 0.0f);

		btRigidBody* MakePhysicsEllipseoidFromFBX(Mesh* p_model, float _scale);
		btRigidBody* MakePhysicsBoxFromFBX(Mesh* p_model, float _scale);
		btRigidBody* MakePhysicsCompoundBoxFromFBX(Mesh* p_model, float x, float y, float z, float _scale);
		btRigidBody* MakePhysicsSphereFromFBX(Mesh* p_model, float _scale);
		btRigidBody* MakePhysicsCylinderFBX(Mesh* p_model, float _scale);
		btRigidBody* MakePhysicsCylinderExtents(float extent1, float extent2, float extent3, float _scale);
		btRigidBody* MakePhysicsBoxExtents(float extent1, float extent2, float extent3, float _scale);
		btRigidBody* MeshMotion::MakePhysicsConvexHullFromFBX(Mesh* p_model, float _scale);
		btRigidBody* MakePhysicsNonColision();


		ID3D11ShaderResourceView* m_Texture;

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

	};

}