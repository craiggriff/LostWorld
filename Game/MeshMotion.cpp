#include "pch.h"

#include "MeshMotion.h"

using namespace Game;
using namespace DX;
using namespace DirectX;

MeshMotion::MeshMotion(std::shared_ptr<DX::DeviceResources> pm_deviceResources, Physics* phys, ObjInfo* pob_info)
{
	m_phys = phys;

	m_deviceResources = pm_deviceResources;

	if (pob_info != nullptr)
	{
		ob_info = *pob_info;
	}
}

void MeshMotion::getWorldTransform(btTransform &worldTransform) const
{
	worldTransform = m_initialTransform;
}

void MeshMotion::setWorldTransform(const btTransform &worldTransform)
{

}

MeshMotion::~MeshMotion()
{


}

void MeshMotion::ResetToPosition(DirectX::XMFLOAT3 pos)
{
	m_initialTransform = btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(pos.x, pos.y, pos.z));
	m_rigidbody->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
	m_rigidbody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
	m_rigidbody->setWorldTransform(m_initialTransform);
}

void MeshMotion::SetPosition(float x, float y, float z, float yaw, float pitch, float roll)
{
	m_initialTransform = btTransform(btQuaternion(yaw, pitch, roll), btVector3(x, y /*+ (m_model->total_height*0.5f)*/, z));
	m_rigidbody->setWorldTransform(m_initialTransform);
	setWorldTransform(m_initialTransform);
}

void MeshMotion::SetPositionnw(float x, float y, float z, float yaw, float pitch, float roll)
{
	btQuaternion floor_rot = btQuaternion(yaw, pitch, roll, 0.0f);
	m_initialTransform.setRotation(floor_rot);
	m_initialTransform.setOrigin(btVector3(x, y, z));

	m_rigidbody->setWorldTransform(m_initialTransform);

	setWorldTransform(m_initialTransform);
}

void MeshMotion::MakeMatrix()
{
	btTransform trans;

	big_change_in_velocity = false;

	btVector3 velocity = m_rigidbody->getLinearVelocity();
	btVector3 angular_velocity = m_rigidbody->getAngularVelocity();

	/*
	if ((last_velocity.distance(velocity) > 2.0f) || (last_angular_velocity.distance(angular_velocity) > 1.0f))
	{
	if (rand() % 5 == 1)
	{
	big_change_in_velocity = true;
	}
	}
	*/


	last_velocity = velocity;
	last_angular_velocity = angular_velocity;

	trans = m_rigidbody->getWorldTransform();

	q_rotation = trans.getRotation();

	XMVECTOR data;
	XMVECTORF32 floatingVector = { q_rotation.getX(), q_rotation.getY(), q_rotation.getZ(), q_rotation.getW() };
	data = floatingVector;

	auto rotationMatrix = XMMatrixRotationQuaternion(data);

	auto translationMatrix = XMMatrixTranslation(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ());

	XMStoreFloat4x4(&m_Matrix, XMMatrixTranspose(rotationMatrix * translationMatrix));

}

void MeshMotion::GetMatrix(XMFLOAT4X4* p_ModelMatrix)
{
	*p_ModelMatrix = m_Matrix;
}

void MeshMotion::GetMatrixOffsetScale(DirectX::XMFLOAT4X4* p_ModelMatrix, float x, float y, float z, float _scale)
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

	auto translationMatrix = XMMatrixTranslation(trans.getOrigin().getX() + x, trans.getOrigin().getY() + y, trans.getOrigin().getZ() + z);

	XMStoreFloat4x4(p_ModelMatrix, XMMatrixTranspose(scaleMatrix * rotationMatrix * translationMatrix));
	m_Matrix = *p_ModelMatrix;

}

btRigidBody*  MeshMotion::MakePhysicsNonColision()
{
	ob_info = {
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.01f, 0.1f, 0.01f),
		0
		, (COL_WHEELLIFT | COL_OBJECTS)
		, (COL_WHEEL) };


	m_initialTransform = btTransform(btQuaternion(ob_info.dir.x, ob_info.dir.y, ob_info.dir.z), btVector3(ob_info.pos.x, ob_info.pos.y, ob_info.pos.z));
	//bt
	auto fallShape = new btSphereShape(btSphereShape(1.0f));

	btMotionState* fallMotionState = (btMotionState*)this;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info.mrf.x, fallInertia);
	m_rigidbody = m_phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, &ob_info);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshMotion::MakePhysicsEllipseoidFromFBX(Mesh* p_model, float _scale)
{
	item_id = ob_info.item_id;

	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	m_model = p_model;

	extent1 = m_model->Extents().MaxX*_scale;
	extent2 = m_model->Extents().MaxY*_scale*0.6f;
	extent3 = m_model->Extents().MaxZ*_scale;

	float uniform_value = 1.0f;

	m_initialTransform = btTransform(btQuaternion(ob_info.dir.x, ob_info.dir.y, ob_info.dir.z), btVector3(ob_info.pos.x, ob_info.pos.y, ob_info.pos.z));

	auto fallShape = new btMultiSphereShape(&btVector3(0.0f, 0.0f, 0.0f), &uniform_value, 1);

	fallShape->setLocalScaling(btVector3(extent1, extent2, extent3));

	btMotionState* fallMotionState = (btMotionState*)this;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info.mrf.x, fallInertia);
	m_rigidbody = m_phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, &ob_info);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshMotion::MakePhysicsSphereFromFBX(Mesh* p_model, float _scale)
{
	item_id = ob_info.item_id;

	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	m_model = p_model;

	extent1 = m_model->Extents().MaxX*_scale;
	extent2 = m_model->Extents().MaxY*_scale;
	extent3 = m_model->Extents().MaxZ*_scale;

	if (extent2 > extent1)
		extent1 = extent2;

	if (extent3 > extent1)
		extent1 = extent3;


	m_initialTransform = btTransform(btQuaternion(ob_info.dir.x, ob_info.dir.y, ob_info.dir.z), btVector3(ob_info.pos.x, ob_info.pos.y, ob_info.pos.z));

	auto fallShape = new btSphereShape(btSphereShape(extent1));

	btMotionState* fallMotionState = (btMotionState*)this;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info.mrf.x, fallInertia);
	m_rigidbody = m_phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, &ob_info);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshMotion::MakePhysicsCylinderExtents(float extent1, float extent2, float extent3, float _scale)
{
	extent1 *= _scale;
	extent2 *= _scale;
	extent3 *= _scale;

	m_initialTransform = btTransform(btQuaternion(ob_info.dir.x, ob_info.dir.y, ob_info.dir.z), btVector3(ob_info.pos.x, ob_info.pos.y, ob_info.pos.z));

	auto fallShape = new btCylinderShape(btCylinderShape(btVector3(extent1, extent2, extent3)));

	btMotionState* fallMotionState = (btMotionState*)this;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info.mrf.x, fallInertia);
	m_rigidbody = m_phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, &ob_info);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshMotion::MakePhysicsCylinderFBX(Mesh* p_model, float _scale)
{
	item_id = ob_info.item_id;

	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	m_model = p_model;

	extent1 = m_model->Extents().MaxX*_scale;
	extent2 = m_model->Extents().MaxY*_scale;
	extent3 = m_model->Extents().MaxZ*_scale;

	m_initialTransform = btTransform(btQuaternion(ob_info.dir.x, ob_info.dir.y, ob_info.dir.z), btVector3(ob_info.pos.x, ob_info.pos.y, ob_info.pos.z));

	auto fallShape = new btCylinderShape(btCylinderShape(btVector3(extent1, extent2, extent3)));

	btMotionState* fallMotionState = (btMotionState*)this;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info.mrf.x, fallInertia);
	m_rigidbody = m_phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, &ob_info);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshMotion::MakePhysicsSquareplane(float size, float y_offset)
{
	item_id = ob_info.item_id;

	btVector3 A;
	btVector3 B;
	btVector3 C;
	btTriangleMesh* data = new btTriangleMesh();

	A = btVector3(-size, y_offset, size);
	B = btVector3(size, y_offset, size);
	C = btVector3(size, y_offset, -size);
	data->addTriangle(A, B, C, false);

	A = btVector3(size, y_offset, -size);
	B = btVector3(-size, y_offset, -size);
	C = btVector3(-size, y_offset, size);
	data->addTriangle(A, B, C, false);

	m_initialTransform = btTransform(btQuaternion(ob_info.dir.x, ob_info.dir.y, ob_info.dir.z), btVector3(ob_info.pos.x, ob_info.pos.y, ob_info.pos.z));

	auto fallShape = new btBvhTriangleMeshShape(data, true);

	btMotionState* fallMotionState = (btMotionState*)this;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info.mrf.x, fallInertia);
	m_rigidbody = m_phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, &ob_info);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshMotion::MakePhysicsBoxExtents(float extent1, float extent2, float extent3, float _scale)
{
	item_id = ob_info.item_id;



	m_initialTransform = btTransform(btQuaternion(ob_info.dir.x, ob_info.dir.y, ob_info.dir.z), btVector3(ob_info.pos.x, ob_info.pos.y, ob_info.pos.z));

	auto fallShape = new btBoxShape(btBoxShape(btVector3(extent1, extent2, extent3)));

	btMotionState* fallMotionState = (btMotionState*)this;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info.mrf.x, fallInertia);
	m_rigidbody = m_phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, &ob_info);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}



btRigidBody*  MeshMotion::MakePhysicsBoxFromFBX(Mesh* p_model, float _scale)
{
	item_id = ob_info.item_id;

	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	m_model = p_model;
	//m_model->Extents
	extent1 = m_model->Extents().MaxX*_scale;
	extent2 = m_model->Extents().MaxY*_scale;
	extent3 = m_model->Extents().MaxZ*_scale;

	m_initialTransform = btTransform(btQuaternion(ob_info.dir.x, ob_info.dir.y, ob_info.dir.z), btVector3(ob_info.pos.x, ob_info.pos.y, ob_info.pos.z));

	auto fallShape = new btBoxShape(btBoxShape(btVector3(extent1, extent2, extent3)));

	btMotionState* fallMotionState = (btMotionState*)this;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info.mrf.x, fallInertia);
	m_rigidbody = m_phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, &ob_info);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}


btRigidBody*  MeshMotion::MakePhysicsConvexHullFromFBX(Mesh* p_model, float _scale)
{
	item_id = ob_info.item_id;

	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	auto fallShape = new btConvexHullShape();
	//btConvexTriangleMeshShape
	//m_model->SubMeshes[0]
	btVector3 vec;
	for (int i = 0; i < p_model->Triangles().size(); i++)
	{
		vec.setX(p_model->Points().at(i).point.x);
		vec.setY(p_model->Points().at(i).point.y);
		vec.setZ(p_model->Points().at(i).point.z);

		fallShape->addPoint(vec);
	}



	m_model = p_model;
	//m_model->Extents
	//extent1 = m_model->Extents().MaxX*_scale;
	//extent2 = m_model->Extents().MaxY*_scale;
	//extent3 = m_model->Extents().MaxZ*_scale;

	m_initialTransform = btTransform(btQuaternion(ob_info.dir.x, ob_info.dir.y, ob_info.dir.z), btVector3(ob_info.pos.x, ob_info.pos.y, ob_info.pos.z));

	

	btMotionState* fallMotionState = (btMotionState*)this;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info.mrf.x, fallInertia);
	m_rigidbody = m_phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, &ob_info);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  MeshMotion::MakePhysicsCompoundBoxFromFBX(Mesh* p_model, float x, float y, float z, float _scale)
{
	item_id = ob_info.item_id;

	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	m_model = p_model;

	extent1 = m_model->Extents().MaxX*_scale;
	extent2 = m_model->Extents().MaxY*_scale;
	extent3 = m_model->Extents().MaxZ*_scale;

	extent2 = abs(extent2) *0.6f;

	m_initialTransform = btTransform(btQuaternion(ob_info.dir.x, ob_info.dir.y, ob_info.dir.z), btVector3(ob_info.pos.x, ob_info.pos.y, ob_info.pos.z));


	btCollisionShape* boxShape = new btBoxShape(btVector3(extent1, extent2, extent3));
	//btCollisionShape* boxShape = new btSphereShape(2.0f);
	btCompoundShape* compound = new btCompoundShape();
	btTransform localTrans;
	localTrans.setIdentity();
	//localTrans effectively shifts the center of mass with respect to the chassis
	localTrans.setOrigin(btVector3(x, y, z));
	compound->addChildShape(localTrans, boxShape);



	btMotionState* fallMotionState = (btMotionState*)this;
	btVector3 fallInertia(0, 0, 0);

	compound->calculateLocalInertia(ob_info.mrf.x, fallInertia);
	m_rigidbody = m_phys->AddPhysicalObject(compound, fallMotionState, fallInertia, &ob_info);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}