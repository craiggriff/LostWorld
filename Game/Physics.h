#pragma once

#include "../Bullet/src/btBulletDynamicsCommon.h"
#include "Content/ShaderStructures.h"

namespace Game
{

	class Physics
	{
	public:
		Physics(void);
		~Physics(void);

		void Update(float timeDelta, float timeTotal);
		btRigidBody* AddPhysicalObject(btCollisionShape* collisionShape, btMotionState* motionState, const btVector3& inertia, ObjInfo* info);

		void RemoveAllObjects(int offset);

		int cur_unique_id;

		void CheckForCollisions();

		btRigidBody* MakeWallPlane(ObjInfo* info);
		//	void Pause();
		//	void Continue();

		int bCollideTest;
		float high_impulse;

		//private:
		void ClearPhysicsObjects();

		Physics(const Physics&) {}
		Physics& operator=(const Physics&) { return *this; }

		std::unique_ptr<btBroadphaseInterface> m_broadphase;
		std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfiguration;
		std::unique_ptr<btCollisionDispatcher> m_dispatcher;
		std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
		std::unique_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;

		std::vector<std::unique_ptr<btCollisionShape>> m_shapes;
		std::vector<std::unique_ptr<btRigidBody>> m_rigidBodies;

	};

}