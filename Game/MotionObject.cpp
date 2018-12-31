#include "pch.h"
#include "MotionObject.h"

using namespace Game;
using namespace DX;
using namespace DirectX;

MotionObject::MotionObject(std::shared_ptr<DX::DeviceResources> pm_deviceResources, Physics* phys)
{
	m_phys = phys;

	m_deviceResources = pm_deviceResources;

}

void MotionObject::getWorldTransform(btTransform &worldTransform) const
{
	worldTransform = m_initialTransform;
}

void MotionObject::setWorldTransform(const btTransform &worldTransform)
{

}

MotionObject::~MotionObject()
{


}

void MotionObject::ResetToPosition(DirectX::XMFLOAT3 pos)
{
	m_initialTransform = btTransform(btQuaternion(0.0f,0.0f,0.0f), btVector3(pos.x, pos.y, pos.z));
	m_rigidbody->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
	m_rigidbody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
	m_rigidbody->setWorldTransform(m_initialTransform);
}

void MotionObject::SetPosition(float x, float y, float z, float yaw, float pitch, float roll)
{
	m_initialTransform = btTransform(btQuaternion(yaw, pitch, roll), btVector3(x, y /*+ (m_model->total_height*0.5f)*/, z));
	m_rigidbody->setWorldTransform(m_initialTransform);
	setWorldTransform(m_initialTransform);
}

void MotionObject::SetPositionnw(float x, float y, float z, float yaw, float pitch, float roll)
{
	btQuaternion floor_rot = btQuaternion(yaw, pitch, roll, 0.0f);
	m_initialTransform.setRotation(floor_rot);
	m_initialTransform.setOrigin(btVector3(x, y, z));

	m_rigidbody->setWorldTransform(m_initialTransform);

	setWorldTransform(m_initialTransform);
}

void MotionObject::GetMatrix(XMFLOAT4X4* p_ModelMatrix)
{
	btTransform trans;
	
	big_change_in_velocity = false;

	btVector3 velocity = m_rigidbody->getLinearVelocity();
	btVector3 angular_velocity = m_rigidbody->getAngularVelocity();

	if ((last_velocity.distance(velocity) > 2.0f) || (last_angular_velocity.distance(angular_velocity) > 1.0f))
	{
		if (rand() % 5 == 1)
		{
			big_change_in_velocity = true;
		}
	}
	last_velocity = velocity;
	last_angular_velocity = angular_velocity;

	trans = m_rigidbody->getWorldTransform();
	
	q_rotation = trans.getRotation();

	XMVECTOR data;
	XMVECTORF32 floatingVector = { q_rotation.getX(), q_rotation.getY(), q_rotation.getZ(), q_rotation.getW() };
	data = floatingVector;
	
	auto rotationMatrix = XMMatrixRotationQuaternion(data);

	auto translationMatrix = XMMatrixTranslation(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ());

	XMStoreFloat4x4(p_ModelMatrix, XMMatrixTranspose(rotationMatrix * translationMatrix));
	m_Matrix = *p_ModelMatrix;

}

void MotionObject::GetMatrixOffsetScale(DirectX::XMFLOAT4X4* p_ModelMatrix, float x, float y, float z,float _scale)
{
	btTransform trans;

	big_change_in_velocity = false;

	btVector3 velocity = m_rigidbody->getLinearVelocity();
	btVector3 angular_velocity = m_rigidbody->getAngularVelocity();

	if ((last_velocity.distance(velocity) > 2.0f) || (last_angular_velocity.distance(angular_velocity) > 1.0f))
	{
		if (rand() % 5 == 1)
		{
			big_change_in_velocity = true;
		}
	}
	last_velocity = velocity;
	last_angular_velocity = angular_velocity;

	trans = m_rigidbody->getWorldTransform();

	q_rotation = trans.getRotation();

	XMVECTOR data;
	XMVECTORF32 floatingVector = { q_rotation.getX(), q_rotation.getY(), q_rotation.getZ(), q_rotation.getW() };
	data = floatingVector;

	auto scaleMatrix = XMMatrixScaling(_scale, _scale, _scale);

	auto rotationMatrix = XMMatrixRotationQuaternion(data);

	auto translationMatrix = XMMatrixTranslation(trans.getOrigin().getX()+x, trans.getOrigin().getY()+y, trans.getOrigin().getZ()+z);

	XMStoreFloat4x4(p_ModelMatrix, XMMatrixTranspose(scaleMatrix * rotationMatrix * translationMatrix));
	m_Matrix = *p_ModelMatrix;

}


btRigidBody*  MotionObject::MakePhysicsSphereFromMesh(ModelLoader* p_model, float _scale)
{
	item_id = ob_info.item_id;

	btVector3 A;
	btVector3 B;
	btVector3 C;

	float radius1 = 0.0f;

	m_model = p_model;

	for (int i = 0; i<m_model->GetVerticesCount() - 3; i += 3)
	{
		if (abs(m_model->m_phy_vertices[i].x) > radius1)
			radius1 = abs(m_model->m_phy_vertices[i].x);
		if (abs(m_model->m_phy_vertices[i + 1].x) > radius1)
			radius1 = abs(m_model->m_phy_vertices[i + 1].x);
		if (abs(m_model->m_phy_vertices[i + 2].x) > radius1)
			radius1 = abs(m_model->m_phy_vertices[i + 2].x);

		if (abs(m_model->m_phy_vertices[i].y) > radius1)
			radius1 = abs(m_model->m_phy_vertices[i].y);
		if (abs(m_model->m_phy_vertices[i + 1].y) > radius1)
			radius1 = abs(m_model->m_phy_vertices[i + 1].y);
		if (abs(m_model->m_phy_vertices[i + 2].y) > radius1)
			radius1 = abs(m_model->m_phy_vertices[i + 2].y);

		if (abs(m_model->m_phy_vertices[i].z) > radius1)
			radius1 = abs(m_model->m_phy_vertices[i].z);
		if (abs(m_model->m_phy_vertices[i + 1].z) > radius1)
			radius1 = abs(m_model->m_phy_vertices[i + 1].z);
		if (abs(m_model->m_phy_vertices[i + 2].z) > radius1)
			radius1 = abs(m_model->m_phy_vertices[i + 2].z);
	}

	radius1 = radius1 * _scale;

	m_initialTransform = btTransform(btQuaternion(ob_info.dir.x, ob_info.dir.y, ob_info.dir.z), btVector3(ob_info.pos.x, ob_info.pos.y, ob_info.pos.z));

	auto fallShape = new btSphereShape(btSphereShape(radius1));

	btMotionState* fallMotionState = (btMotionState*)this;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info.mrf.x, fallInertia);
	m_rigidbody = m_phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, &ob_info);
	//m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}


btRigidBody*  MotionObject::MakePhysicsBoxFromFBX(FBXModelLoader* p_model, float _scale)
{
	item_id = ob_info.item_id;

	btVector3 A;
	btVector3 B;
	btVector3 C;

	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	//m_model = p_model;

	for (int i = 0; i<m_model->GetVerticesCount() - 3; i += 3)
	{
		if (abs(m_model->m_vertices[i].pos.x) > extent1)
			extent1 = abs(m_model->m_vertices[i].pos.x);
		if (abs(m_model->m_vertices[i + 1].pos.x) > extent1)
			extent1 = abs(m_model->m_vertices[i + 1].pos.x);
		if (abs(m_model->m_vertices[i + 2].pos.x) > extent1)
			extent1 = abs(m_model->m_vertices[i + 2].pos.x);

		if (abs(m_model->m_vertices[i].pos.y) > extent2)
			extent2 = abs(m_model->m_vertices[i].pos.y);
		if (abs(m_model->m_vertices[i + 1].pos.y) > extent2)
			extent2 = abs(m_model->m_vertices[i + 1].pos.y);
		if (abs(m_model->m_vertices[i + 2].pos.y) > extent2)
			extent2 = abs(m_model->m_vertices[i + 2].pos.y);

		if (abs(m_model->m_vertices[i].pos.z) > extent3)
			extent3 = abs(m_model->m_vertices[i].pos.z);
		if (abs(m_model->m_vertices[i + 1].pos.z) > extent3)
			extent3 = abs(m_model->m_vertices[i + 1].pos.z);
		if (abs(m_model->m_vertices[i + 2].pos.z) > extent3)
			extent3 = abs(m_model->m_vertices[i + 2].pos.z);

	}
	extent1 = extent1*_scale;
	extent2 = extent2*_scale;
	extent3 = extent3*_scale;

	m_initialTransform = btTransform(btQuaternion(ob_info.dir.x, ob_info.dir.y, ob_info.dir.z), btVector3(ob_info.pos.x, ob_info.pos.y, ob_info.pos.z));

	auto fallShape = new btBoxShape(btBoxShape(btVector3(extent1, extent2, extent3)));

	btMotionState* fallMotionState = (btMotionState*)this;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info.mrf.x, fallInertia);
	m_rigidbody = m_phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, &ob_info);
	m_rigidbody->setActivationState(DISABLE_DEACTIVATION);

	return m_rigidbody;
}



btRigidBody*  MotionObject::MakePhysicsBoxFromMesh(ModelLoader* p_model, float _scale)
{
	item_id = ob_info.item_id;

	btVector3 A;
	btVector3 B;
	btVector3 C;

	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	m_model = p_model;

	for (int i = 0; i<m_model->GetVerticesCount() - 3; i += 3)
	{
		if (abs(m_model->m_phy_vertices[i].x) > extent1)
			extent1 = abs(m_model->m_phy_vertices[i].x);
		if (abs(m_model->m_phy_vertices[i + 1].x) > extent1)
			extent1 = abs(m_model->m_phy_vertices[i + 1].x);
		if (abs(m_model->m_phy_vertices[i + 2].x) > extent1)
			extent1 = abs(m_model->m_phy_vertices[i + 2].x);

		if (abs(m_model->m_phy_vertices[i].y) > extent2)
			extent2 = abs(m_model->m_phy_vertices[i].y);
		if (abs(m_model->m_phy_vertices[i + 1].y) > extent2)
			extent2 = abs(m_model->m_phy_vertices[i + 1].y);
		if (abs(m_model->m_phy_vertices[i + 2].y) > extent2)
			extent2 = abs(m_model->m_phy_vertices[i + 2].y);

		if (abs(m_model->m_phy_vertices[i].z) > extent3)
			extent3 = abs(m_model->m_phy_vertices[i].z);
		if (abs(m_model->m_phy_vertices[i + 1].z) > extent3)
			extent3 = abs(m_model->m_phy_vertices[i + 1].z);
		if (abs(m_model->m_phy_vertices[i + 2].z) > extent3)
			extent3 = abs(m_model->m_phy_vertices[i + 2].z);

	}
	extent1 = extent1*_scale;
	extent2 = extent2*_scale;
	extent3 = extent3*_scale;

	m_initialTransform = btTransform(btQuaternion(ob_info.dir.x, ob_info.dir.y, ob_info.dir.z), btVector3(ob_info.pos.x, ob_info.pos.y , ob_info.pos.z));

	auto fallShape = new btBoxShape(btBoxShape(btVector3(extent1, extent2, extent3)));

	btMotionState* fallMotionState = (btMotionState*)this;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info.mrf.x, fallInertia);
	m_rigidbody = m_phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, &ob_info);
	m_rigidbody->setActivationState(DISABLE_DEACTIVATION);

	return m_rigidbody;
}




btRigidBody*  MotionObject::MakePhysicsCompoundBoxFromMesh(ModelLoader* p_model, float x, float y, float z)
{
	item_id = ob_info.item_id;

	btVector3 A;
	btVector3 B;
	btVector3 C;

	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	m_model = p_model;
	//btTriangleMesh* data = new btTriangleMesh();

	for (int i = 0; i<m_model->GetVerticesCount() - 3; i += 3)
	{
		if (abs(m_model->m_phy_vertices[i].x) > extent1)
			extent1 = abs(m_model->m_phy_vertices[i].x);
		if (abs(m_model->m_phy_vertices[i + 1].x) > extent1)
			extent1 = abs(m_model->m_phy_vertices[i + 1].x);
		if (abs(m_model->m_phy_vertices[i + 2].x) > extent1)
			extent1 = abs(m_model->m_phy_vertices[i + 2].x);

		if (abs(m_model->m_phy_vertices[i].y) > extent2)
			extent2 = abs(m_model->m_phy_vertices[i].y);
		if (abs(m_model->m_phy_vertices[i + 1].y) > extent2)
			extent2 = abs(m_model->m_phy_vertices[i + 1].y);
		if (abs(m_model->m_phy_vertices[i + 2].y) > extent2)
			extent2 = abs(m_model->m_phy_vertices[i + 2].y);

		if (abs(m_model->m_phy_vertices[i].z) > extent3)
			extent3 = abs(m_model->m_phy_vertices[i].z);
		if (abs(m_model->m_phy_vertices[i + 1].z) > extent3)
			extent3 = abs(m_model->m_phy_vertices[i + 1].z);
		if (abs(m_model->m_phy_vertices[i + 2].z) > extent3)
			extent3 = abs(m_model->m_phy_vertices[i + 2].z);

	}
	extent2 -= abs(y)*0.5f;


	btCollisionShape* boxShape = new btBoxShape(btVector3(extent1, extent2, extent3));
	//btCollisionShape* boxShape = new btSphereShape(2.0f);
	btCompoundShape* compound = new btCompoundShape();
	btTransform localTrans;
	localTrans.setIdentity();
	//localTrans effectively shifts the center of mass with respect to the chassis
	localTrans.setOrigin(btVector3(x, y, z));
	compound->addChildShape(localTrans, boxShape);


	//auto fallShape = new btBoxShape(btBoxShape(btVector3(extent1, extent2, extent3)));
	//auto fallShape = new btSphereShape(btSphereShape(1.5f));
	//auto fallShape = new btTriangleMesh (btSphereShape(1.5f));
	btMotionState* fallMotionState = (btMotionState*)this;//(cube, btTransform(btQuaternion(0,0,0,1),btVector3(x,y,z)));
														  //btScalar mass = 0.2;
	btVector3 fallInertia(0, 0, 0);

	compound->calculateLocalInertia(ob_info.mrf.x, fallInertia);
	m_rigidbody = m_phys->AddPhysicalObject(compound, fallMotionState, fallInertia, &ob_info);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	//delete data;


	return m_rigidbody;
}

