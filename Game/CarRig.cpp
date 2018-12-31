#include "pch.h"

#include "../DirectXTK/Inc/DDSTextureLoader.h"
//#include "cpprest/http_client.h"

//#include "cpprest/json.h"

#include "CarRig.h"

using namespace Game;
using namespace DirectX;
using namespace DX;
using namespace std;

//using namespace utility;

CarRig::CarRig(AllResources* p_Resources)
{
	points = 0;

	m_Resources = p_Resources;
	m_deviceResources = m_Resources->m_deviceResources;

	m_phys = &m_Resources->m_Physics;

	dir_position = btVector3(0.0f, 0.0f, 0.0f);
	dir_last_osition = btVector3(0.0f, 0.0f, 0.0f);

	bIsHolding = false;
	bReleaseForce = false;

	bShooting = false;
	bShoot = false;

	Wheelposition = 0;

	b_show_rockets = 0;

	carspeed = 0.0f;

	break_value = 400.0f;

	rockets = 0;

	respot_delay = 0.0f;

	bPlayerActive = false;

	last_floor_norm = btVector3(0.0f, 1.0f, 0.0f);

	bPhysicsMade = false;
	bLoadingComplete = false;


	car_point = btVector3(0.1f, 0.1f, 1.0f);
	y_offset = btVector3(0.1f, 1.0f, 0.1f);

	bSBUp = false;
	bSBDown = false;
	bSBLeft = false;
	bSBRight = false;

	bInCarView = false;

	shoot_delay_time = 0.3f;
	bAB = false;
}

void CarRig::SetSteerPosition(float _pos)
{
	steer_position = _pos;

}

btVector3* CarRig::GetDirection()
{
	return &car_point;
}

btVector3* CarRig::GetUp()
{
	return &y_offset;
}




void CarRig::Update(float timeDelta, float timeTotal)
{
	int i;


	if (bLoadingComplete == false)
		return;

	float wheel_height[4];
	float highest = -100.0f;
	float lowest = 100.0f;
	float highest_space = -100.0f;
	float lowest_space = 100.0f;

	for (i = 0; i < 4; i++)
	{
		btVector3 wheel_pos = btVector3(wheel[i]->m_Matrix._14, wheel[i]->m_Matrix._24, wheel[i]->m_Matrix._34);

		float wheel_height = p_Level->GetTerrainHeight(wheel_pos.getX(), wheel_pos.getZ());

		if (wheel_pos.getY() < wheel_height)
		{
			btTransform wheel_transform = wheel[i]->m_rigidbody->getWorldTransform();
			wheel_transform.getOrigin().setY(wheel_height+10.0f);

		}
	}

	float spacing = 0.0f;

	carbody->MakeMatrix();

	player_pos = btVector3(carbody->m_Matrix._14, carbody->m_Matrix._24, carbody->m_Matrix._34);

	player_x = carbody->m_Matrix._14;
	player_y = carbody->m_Matrix._24;
	player_z = carbody->m_Matrix._34;

	rot_mat = btMatrix3x3(carbody->m_Matrix._11,
							carbody->m_Matrix._12,
							carbody->m_Matrix._13,
							carbody->m_Matrix._21,
							carbody->m_Matrix._22,
							carbody->m_Matrix._23,
							carbody->m_Matrix._31,
							carbody->m_Matrix._32,
							carbody->m_Matrix._33);

	x_offset = btVector3(1.0f, 0.0f, 0.0f);
	x_offset = rot_mat* x_offset;

	y_offset = btVector3(0.0f, 0.2f, 0.0f);
	y_offset = rot_mat* y_offset;

	down_offset = btVector3(0.0f, -0.3f, 0.0f);
	down_offset = rot_mat* down_offset;

	rot_mat_front = btMatrix3x3(wheel[1]->m_Matrix._11,
		wheel[1]->m_Matrix._12,
		wheel[1]->m_Matrix._13,
		wheel[1]->m_Matrix._21,
		wheel[1]->m_Matrix._22,
		wheel[1]->m_Matrix._23,
		wheel[1]->m_Matrix._31,
		wheel[1]->m_Matrix._32,
		wheel[1]->m_Matrix._33);

	x_frontoffset1 = btVector3(0.0f, 1.0f,0.0f);
	x_frontoffset1 = rot_mat_front* x_frontoffset1;

	rot_mat_front = btMatrix3x3(wheel[3]->m_Matrix._11,
		wheel[3]->m_Matrix._12,
		wheel[3]->m_Matrix._13,
		wheel[3]->m_Matrix._21,
		wheel[3]->m_Matrix._22,
		wheel[3]->m_Matrix._23,
		wheel[3]->m_Matrix._31,
		wheel[3]->m_Matrix._32,
		wheel[3]->m_Matrix._33);

	x_frontoffset2 = btVector3(0.0f, 1.0f, 0.0f);
	x_frontoffset2 = rot_mat_front* x_frontoffset2;

	//if (false)
	//{
	//	for (i = 0; i < 4; i++)
	//	{
	//		btTransform tran = wheel[i]->m_rigidbody->getWorldTransform();
	//		//tran = wheel[i]->m_model->
	//		float height = tran.getOrigin().getY();
	//		float floor_height = p_Level->GetTerrainHeight(tran.getOrigin().getX(), tran.getOrigin().getZ());
	//		if (height - wheel[i]->m_model->largest_extent < floor_height + 0.5f)
	//		{
	//			tran.setOrigin(btVector3(tran.getOrigin().getX(),
	//				floor_height + 0.5f + wheel[i]->m_model->largest_extent,
	//				tran.getOrigin().getZ()));
	//		}
	//	}
	//}
	////return ;
	////return;


	//if (false)
	//{
	//	for(i=0;i<4;i++)
	//	{
	//		btTransform block_tran;
	//		//spacing = wheel[i]->m_rigidbody->getWorldTransform().getOrigin().getY();
	//		wheel[i]->MakeMatrix();
	//		float wheel_x = wheel[i]->m_Matrix._14;// -(y_offset.getX()*2.0f);
	//		float wheel_z = wheel[i]->m_Matrix._34;// -(y_offset.getZ()*2.0f);

	//		float floor_height = p_Level->GetTerrainHeight(wheel_x, wheel_z);
	//		btVector3 floor_norm = p_Level->GetNormal(wheel_x, wheel_z);

	//		//btQuaternion norm_rot = btQuaternion(-(M_PI*1.5f),(M_PI*1.5f), (M_PI*0.5f));
	//		//btMatrix3x3 rot_mat = btMatrix3x3(norm_rot);

	//		//floor_norm = rot_mat * floor_norm;

	//		float norm_x = floor_norm.getX();
	//		float norm_y = floor_norm.getY();
	//		float norm_z = floor_norm.getZ();

	//		//floor_norm.setX(floor_norm.getX());
	//		//floor_norm.setY(floor_norm.getY());
	//		//floor_norm.setZ(floor_norm.getZ());

	//		floor_norm.normalize();

	//		//btQuaternion norm_rot = btQuaternion((M_PI*0.5f), -(M_PI*0.5f), (M_PI*0.5f));

	//		btQuaternion floor_rot = btQuaternion(floor_norm.getX()*0.2f, floor_norm.getY(), floor_norm.getZ()*0.2f  , 0.0f);
	//		//floor_rot = norm_rot * floor_rot;
	//		//floor_rot.setY(-floor_rot.getY());

	//		block_tran.setOrigin(btVector3(wheel_x-floor_norm.getX()*0.3f, floor_height+1.7f+wheel[0]->m_model->largest_extent, wheel_z - floor_norm.getZ()*0.3f));

	//		block_tran.setRotation(floor_rot);
	//	
	//		wheel_mount[i]->m_rigidbody->setWorldTransform(block_tran);
	//	}
	//}
	
	if(true)
	{
		bool bHasHit = false;
		btTransform block_tran;
		//spacing = wheel[i]->m_rigidbody->getWorldTransform().getOrigin().getY();
		float wheel_x = carbody->m_Matrix._14;// -(y_offset.getX()*2.0f);
		float wheel_y = carbody->m_Matrix._24;// -(y_offset.getX()*2.0f);
		float wheel_z = carbody->m_Matrix._34;// -(y_offset.getZ()*2.0f);
		btVector3 player_pos = btVector3(wheel_x, wheel_y, wheel_z);
		btCollisionWorld::ClosestRayResultCallback RayCallbackB(btVector3(wheel_x, wheel_y, wheel_z), btVector3(wheel_x - (y_offset.getX()*10.0f), wheel_y - (y_offset.getY()*10.0f), wheel_z - (y_offset.getZ()*10.0f)));
		m_Resources->m_Physics.m_dynamicsWorld->rayTest(btVector3(wheel_x, wheel_y, wheel_z), btVector3(wheel_x - (y_offset.getX()*10.0f), wheel_y - (y_offset.getY()*10.0f), wheel_z - (y_offset.getZ()*10.0f)), RayCallbackB);
		if (RayCallbackB.hasHit())
		{
			bHasHit = true;

			const btBroadphaseProxy* proxy = RayCallbackB.m_collisionObject->getBroadphaseHandle();

			wheel_x = RayCallbackB.m_hitPointWorld.getX();
			wheel_z = RayCallbackB.m_hitPointWorld.getZ();
			
			


			float floor_height = p_Level->GetTerrainHeight(wheel_x, wheel_z);
			btVector3 floor_norm = p_Level->GetNormal(wheel_x, wheel_z);

			//btQuaternion norm_rot = btQuaternion(-(M_PI*1.5f),(M_PI*1.5f), (M_PI*0.5f));
			//btMatrix3x3 rot_mat = btMatrix3x3(norm_rot);

			//floor_norm = rot_mat * floor_norm;

			float norm_x = floor_norm.getX();
			float norm_y = floor_norm.getY();
			float norm_z = floor_norm.getZ();

			//floor_norm.setX(floor_norm.getX());
			//floor_norm.setY(floor_norm.getY());
			//floor_norm.setZ(floor_norm.getZ());

			floor_norm.normalize();

			float dot_prod = floor_norm.dot(y_offset);

			
			//m_Resources->m_uiControl->SetLevel(dot_prod * 100);
			//btQuaternion norm_rot = btQuaternion((M_PI*0.5f), -(M_PI*0.5f), (M_PI*0.5f));
			// cool
			if (floor_norm.dot(y_offset) > 0.18f && player_pos.distance(RayCallbackB.m_hitPointWorld)<2.0f && proxy->m_collisionFilterMask == (COL_TERRAIN | COL_RAY))//(wheel_y-3.0f<floor_height)
			{
				btQuaternion floor_rot = btQuaternion(floor_norm.getX(), floor_norm.getY(), floor_norm.getZ(), 0.0f);
				//floor_rot = norm_rot * floor_rot;
				//floor_rot.setY(-floor_rot.getY());

				block_tran.setOrigin(btVector3(wheel_x - floor_norm.getX()*0.95f, floor_height - (floor_norm.getY()*0.95f), wheel_z - floor_norm.getZ()*0.95f));

				block_tran.setRotation(floor_rot);

				//floorblock[i]->setWorldTransform(block_tran);
				m_FloorPlane->setWorldTransform(block_tran);
				//wheel[i]->getWorldTransform(block_tran);
				//floorblock[i]->setWorldTransform(block_tran);
				//floorblock[i]->SetPositionnw(wheel_x, floor_height - 1.7f, wheel_z, floor_norm.getX(), floor_norm.getY(), floor_norm.getZ());
				//spacing = spacing - wheel_height[i];
			}
			else
			{
				bHasHit = false;
			}
		}

		if(bHasHit==false)
		{
			btTransform block_tran;
			block_tran.setOrigin(btVector3(0.0f, p_Level->GetTerrainHeight(wheel_x, wheel_z)-2.0f, 0.0f));

			btQuaternion floor_rot = btQuaternion(0.0f, 1.0f, 0.0f, 0.0f);
			block_tran.setRotation(floor_rot);

			m_FloorPlane->setWorldTransform(block_tran);

			for (i = 0; i < 4; i++)
			{
				btBroadphaseProxy* proxy = wheel[i]->m_rigidbody->getBroadphaseProxy();
				proxy->m_collisionFilterGroup = (COL_OBJECTS | COL_WALLS | COL_CHAR | COL_TERRAIN);
			}
			btBroadphaseProxy* proxy = carbody->m_rigidbody->getBroadphaseProxy();
			proxy->m_collisionFilterGroup = (COL_OBJECTS | COL_WALLS | COL_CHAR | COL_TERRAIN);
		}
		else
		{

			for (i = 0; i < 4; i++)
			{
				btBroadphaseProxy* proxy = wheel[i]->m_rigidbody->getBroadphaseProxy();
				proxy->m_collisionFilterGroup = (COL_WHEELLIFT | COL_OBJECTS | COL_WALLS | COL_CHAR );
			}
			btBroadphaseProxy* proxy = carbody->m_rigidbody->getBroadphaseProxy();
			proxy->m_collisionFilterGroup = (COL_WHEELLIFT | COL_OBJECTS | COL_WALLS | COL_CHAR);
		}
	}
	/*
	else
	{
		btTransform block_tran;
		block_tran.setOrigin(btVector3(0.0f, 0.0f, 0.0f));

		btQuaternion floor_rot = btQuaternion(0.0f, 1.0f, 0.0f, 0.0f);
		block_tran.setRotation(floor_rot);

		m_FloorPlane->setWorldTransform(block_tran);
		
		for (i = 0; i < 4; i++)
		{
			btBroadphaseProxy* proxy = wheel[i]->m_rigidbody->getBroadphaseProxy();
			proxy->m_collisionFilterGroup = (COL_OBJECTS | COL_WALLS | COL_CHAR | COL_TERRAIN);
		}
		btBroadphaseProxy* proxy = carbody->m_rigidbody->getBroadphaseProxy();
		proxy->m_collisionFilterGroup = (COL_WHEELLIFT | COL_OBJECTS | COL_WALLS | COL_CHAR);
	}
	*/
	/*
	btTransform ground_tran = p_Level->m_FloorPlane->getWorldTransform();

	float floor_height = p_Level->GetTerrainHeight(player_x + down_offset.getX(), player_z + down_offset.getZ());

	btVector3 floor_norm = p_Level->GetNormal(player_x + down_offset.getX(), player_z + down_offset.getZ());

	float heigh_diff = 1.5f;

	if (((highest - lowest)<3.0f) && (highest_space<2.0f))//(ball_tran.getOrigin().getY() - heigh_diff && wheel_tran0.getOrigin().getY() - floor_height<heigh_diff && wheel_tran1.getOrigin().getY() - floor_height<heigh_diff && wheel_tran2.getOrigin().getY() - floor_height<heigh_diff && wheel_tran3.getOrigin().getY() - floor_height<heigh_diff) // ( btVector3(0.0f, 1.0f, 0.0f).distance(floor_norm)<0.4f)
	{
		last_floor_norm += (floor_norm - last_floor_norm)*(timeDelta*4.5);

		btQuaternion floor_rot = btQuaternion(last_floor_norm.getX(), last_floor_norm.getY(), last_floor_norm.getZ(), 0.0f);

		ground_tran.setOrigin(btVector3(player_x, floor_height - 0.7f, player_z));

		ground_tran.setRotation(floor_rot);

		if (floor_norm.getX() != -999.0f)
		{
			p_Level->m_FloorPlane->setWorldTransform(ground_tran);
		}
	}
	else
	{
		ground_tran.setOrigin(btVector3(0.0f, 0.0f, 0.0f));

		btQuaternion floor_rot = btQuaternion(0.0f, 1.0f, 0.0f, 0.0f);
		ground_tran.setRotation(floor_rot);

		if (floor_norm.getX() != -999.0f)
		{
			p_Level->m_FloorPlane->setWorldTransform(ground_tran);
		}
	}
	*/

	if (shooting_delay > 0)
	{
		shooting_delay -= timeDelta;

	}
	else
	{
		if (bShooting == true)
		{
			shooting_delay = shoot_delay_time;
			bShoot = true;
		}
	}


	// find the forwards vectors
	btVector3 lin_vel = carbody->m_rigidbody->getLinearVelocity();


	car_point = btVector3(0.0f, 0.0f, 1.0f);
	car_point = rot_mat* car_point;

	carspeed = lin_vel.dot(car_point);

	// copy rotation matrix
	for (int row = 0; row<3; ++row)
		for (int column = 0; column<3; ++column)
			point_martix.m[row][column] = rot_mat[column][row];

	// copy position
	for (int column = 0; column<3; ++column)
		point_martix.m[3][column] = 0.0f;

	if (bPlayerActive == true)
	{
		if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::W) == true || bSBUp==true)
		{
			if (carspeed < -0.5f)
				Breaking();
			else
				SetMotors(cvalues.max_speed, cvalues.torque);
		}

		if (m_Resources->m_PadInput->RightTrigger() > 0.0f)
		{
			if (carspeed < -0.5f)
				Breaking();
			else
				SetMotors(cvalues.max_speed*m_Resources->m_PadInput->RightTrigger(), cvalues.torque);
		}

		if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::S) == true || bSBDown == true)
		{
			if (carspeed > 0.5f)
				Breaking();
			else
				SetMotors((-cvalues.max_speed*0.5f), -cvalues.torque*0.5f);
		}
		if (m_Resources->m_PadInput->LeftTrigger() > 0.0f)
		{
			if (carspeed > 0.5f)
				Breaking();
			else
				SetMotors((-cvalues.max_speed*0.3f)*m_Resources->m_PadInput->LeftTrigger(), -cvalues.torque*0.3f);
		}
		
		if (steer_position != 0.0f)
		{
			SetFrontWheelAngle(cvalues.max_wheel_turn*steer_position);
		}
		else
		{
			if (m_Resources->m_PadInput->MoveCommandX() < -0.2f || m_Resources->m_PadInput->MoveCommandX() > 0.2f)
			{
				SetFrontWheelAngle(cvalues.max_wheel_turn*m_Resources->m_PadInput->MoveCommandX());

			}
			else
			{
				if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::A) == true)// || m_Resources->m_PadInput->MoveCommandX()<-0.2f || bSBLeft == true)
				{
					if (front_wheel_angle > -cvalues.max_wheel_turn)
					{
						SetFrontWheelAngle(front_wheel_angle - (cvalues.max_wheel_turn*0.05f));
					}
				}
				else
				{
					if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::D) == true)// || m_Resources->m_PadInput->MoveCommandX()>0.2f || bSBRight == true)
					{
						if (front_wheel_angle < cvalues.max_wheel_turn)
						{
							SetFrontWheelAngle(front_wheel_angle + (cvalues.max_wheel_turn*0.05f));
						}
					}
					else
					{
						
						if (front_wheel_angle > 0.0f)
						{
							front_wheel_angle -= 0.03f;
							if (front_wheel_angle < 0.0f)
								front_wheel_angle = 0.0f;
						}
						if (front_wheel_angle < 0.0f)
						{
							front_wheel_angle += 0.03f;
							if (front_wheel_angle > 0.0f)
								front_wheel_angle = 0.0f;
						}
						
						SetFrontWheelAngle(front_wheel_angle);
					}
				}
			}
		}
		if ((m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::R) == true || m_Resources->m_PadInput->ButtonYwas()==true) && respot_delay<=0.0f)
		{
			respot_delay = 3.0f;
			if (p_Level->GetTerrainHeight(player_x, player_z) < 1.0)
			{
				for (i = 0; i < 200; i++)
				{
					if (p_Level->GetTerrainHeight(player_x-(float)i, player_z)>1.0f)
					{
						i = i + 8;
						SetCarPosition(player_x - (float)i, p_Level->GetTerrainHeight(player_x - (float)i, player_z) + 5.0f, player_z, atan2f(car_point.getX(), car_point.getZ()), 0.0f, 0.0f);
						break;
					}
					if (p_Level->GetTerrainHeight(player_x + (float)i, player_z)>1.0f)
					{
						i = i + 8;
						SetCarPosition(player_x + (float)i, p_Level->GetTerrainHeight(player_x + (float)i, player_z) + 5.0f, player_z, atan2f(car_point.getX(), car_point.getZ()), 0.0f, 0.0f);
						break;
					}
					if (p_Level->GetTerrainHeight(player_x , player_z - (float)i)>1.0f)
					{
						i = i + 8;
						SetCarPosition(player_x, p_Level->GetTerrainHeight(player_x , player_z - (float)i) + 5.0f, player_z - (float)i, atan2f(car_point.getX(), car_point.getZ()), 0.0f, 0.0f);
						break;
					}
					if (p_Level->GetTerrainHeight(player_x, player_z + (float)i)>1.0f)
					{
						i = i + 8;
						SetCarPosition(player_x, p_Level->GetTerrainHeight(player_x, player_z + (float)i) + 5.0f, player_z + (float)i, atan2f(car_point.getX(), car_point.getZ()), 0.0f, 0.0f);
						break;
					}
				}
			}
			else
			{
				SetCarPosition(player_x, p_Level->GetTerrainHeight(player_x, player_z) + 5.0f, player_z, atan2f(car_point.getX(), car_point.getZ()), 0.0f, 0.0f);
			}
		}
		else
		{
			respot_delay -= timeDelta;
		}

	}
	else
	{
		Breaking();
	}
}

void Game::CarRig::SetWheelActivation(bool bActive)
{
	for (int i = 0; i < 4; i++)
	{
		if (bActive == true)
		{
			wheel[i]->m_rigidbody->setActivationState(DISABLE_DEACTIVATION);
		}
		else
		{
			wheel[i]->m_rigidbody->setActivationState(WANTS_DEACTIVATION);
		}
	}
}


btVector3* CarRig::GetPosition()
{

	return &player_pos;
}


void CarRig::Breaking()
{
	wheel[0]->m_rigidbody->setDamping(0.0f, break_value);
	wheel[1]->m_rigidbody->setDamping(0.0f, break_value);
	wheel[2]->m_rigidbody->setDamping(0.0f, break_value);
	wheel[3]->m_rigidbody->setDamping(0.0f, break_value);

}


void CarRig::SetMotors(float targetVelocity, float maxMotorImpulse)
{
	wheel[0]->m_rigidbody->setDamping(0.0f, cvalues.wheel_angular_damping);
	wheel[1]->m_rigidbody->setDamping(0.0f, cvalues.wheel_angular_damping);
	wheel[2]->m_rigidbody->setDamping(0.0f, cvalues.wheel_angular_damping);
	wheel[3]->m_rigidbody->setDamping(0.0f, cvalues.wheel_angular_damping);
	if (targetVelocity < 0.0f)
	{
		targetVelocity = -targetVelocity;
		maxMotorImpulse = -maxMotorImpulse;
		if (wheel[0]->m_rigidbody->getAngularVelocity().length() < targetVelocity)
			wheel[0]->m_rigidbody->applyTorqueImpulse(-x_offset*maxMotorImpulse);

		if (wheel[1]->m_rigidbody->getAngularVelocity().length() < targetVelocity)
			wheel[1]->m_rigidbody->applyTorqueImpulse(-x_frontoffset1*maxMotorImpulse);

		if (wheel[2]->m_rigidbody->getAngularVelocity().length() < targetVelocity)
			wheel[2]->m_rigidbody->applyTorqueImpulse(-x_offset*maxMotorImpulse);

		if (wheel[3]->m_rigidbody->getAngularVelocity().length() < targetVelocity)
			wheel[3]->m_rigidbody->applyTorqueImpulse(-x_frontoffset2*maxMotorImpulse);
	}
	else
	{
		if (wheel[0]->m_rigidbody->getAngularVelocity().length() < targetVelocity)
			wheel[0]->m_rigidbody->applyTorqueImpulse(x_offset*maxMotorImpulse);

		if (wheel[1]->m_rigidbody->getAngularVelocity().length() < targetVelocity)
			wheel[1]->m_rigidbody->applyTorqueImpulse(x_frontoffset1*maxMotorImpulse);

		if (wheel[2]->m_rigidbody->getAngularVelocity().length() < targetVelocity)
			wheel[2]->m_rigidbody->applyTorqueImpulse(x_offset*maxMotorImpulse);

		if (wheel[3]->m_rigidbody->getAngularVelocity().length() < targetVelocity)
			wheel[3]->m_rigidbody->applyTorqueImpulse(x_frontoffset2*maxMotorImpulse);
	}
}

void CarRig::Initialize(Level* pp_Level)
{
	float car_mass = 5.1f;

	p_Level = pp_Level;
	bLoadingComplete = false;

	Mesh::LoadFromFile(
		*m_Resources,
		cvalues.body_fname,
		L"",
		L"",
		m_bodyModel, cvalues.body_scale*cvalues.car_scale);
	
	m_bodyModel[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture(cvalues.body_tex0), 0);
	m_bodyModel[1]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture(cvalues.body_tex0), 0);
	
	Mesh::LoadFromFile(
		*m_Resources,
		cvalues.wheel_fname,
		L"",
		L"",
		m_wheelModel, cvalues.wheel_scale*cvalues.car_scale);
	
	if (strlen(cvalues.wheel_tex0) > 0 && m_wheelModel.size()>0)
	{
		m_wheelModel[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture(cvalues.wheel_tex0), 0);
		//m_wheelModel[0]->SetMaterialNormal(L"", 0, m_Resources->m_Textures->LoadTexture("185_norm"), 1.0f);
	}
	if (strlen(cvalues.wheel_nor0) > 0 && m_wheelModel.size()>0)
	{
		m_wheelModel[0]->SetMaterialNormal(L"", 0, m_Resources->m_Textures->LoadTexture(cvalues.wheel_nor0), 1.0f);
	}

	if (strlen(cvalues.wheel_tex1) > 0 && m_wheelModel.size()>1)
	{
		m_wheelModel[1]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture(cvalues.wheel_tex1), 0);
		//m_wheelModel[0]->SetMaterialNormal(L"", 0, m_Resources->m_Textures->LoadTexture("185_norm"), 1.0f);
	}
	if (strlen(cvalues.wheel_tex2) > 0 && m_wheelModel.size()>2)
	{
		m_wheelModel[2]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture(cvalues.wheel_tex2), 0);
		//m_wheelModel[0]->SetMaterialNormal(L"", 0, m_Resources->m_Textures->LoadTexture("185_norm"), 1.0f);
	}

}

void CarRig::SetPhysical()
{
	MakePhysics();
	SetCarPosition(m_Resources->m_LevelInfo.player_start_x, p_Level->GetTerrainHeight(m_Resources->m_LevelInfo.player_start_x, m_Resources->m_LevelInfo.player_start_z) + 10.0f, m_Resources->m_LevelInfo.player_start_z, m_Resources->m_LevelInfo.player_start_angle, 0.0f, 0.0f);
	m_FloorPlane->setFriction(m_Resources->m_LevelInfo.ground_friction);
	m_FloorPlane->setRestitution(m_Resources->m_LevelInfo.ground_restitution);
	bLoadingComplete = true;

}


void CarRig::RemoveContraints()
{
	//return;
	if (bPhysicsMade == false)
		return;

	carbody->m_rigidbody->removeConstraintRef(sus[0]);
	carbody->m_rigidbody->removeConstraintRef(sus[1]);
	carbody->m_rigidbody->removeConstraintRef(sus[2]);
	carbody->m_rigidbody->removeConstraintRef(sus[3]);

	wheel[0]->m_rigidbody->removeConstraintRef(sus[0]);
	wheel[1]->m_rigidbody->removeConstraintRef(sus[1]);
	wheel[2]->m_rigidbody->removeConstraintRef(sus[2]);
	wheel[3]->m_rigidbody->removeConstraintRef(sus[3]);
}



void CarRig::SetWheelTurn(int turn_to)
{
	float _turn = turn_to*0.3f;

	SetFrontWheelAngle(_turn);
	//SetRearWheelAngle(-_turn);
}

void CarRig::SetFrontWheelAngle(float _angle)
{
	front_wheel_angle = _angle;

	btQuaternion rot_quat = btQuaternion((M_PI*0.5f) - _angle, 0.0f, 0.0f);

	frameInA[1].setRotation(rot_quat);
	sus[1]->setFrames(frameInA[1], frameInB[1]);

	frameInA[3].setRotation(rot_quat);
	sus[3]->setFrames(frameInA[3], frameInB[3]);
}


void CarRig::SetRearWheelAngle(float _angle)
{
	front_wheel_angle = _angle;

	btQuaternion rot_quat = btQuaternion((M_PI*0.5f) - _angle, 0.0f, 0.0f);

	frameInA[1].setRotation(rot_quat);
	sus[1]->setFrames(frameInA[1], frameInB[1]);

	frameInA[3].setRotation(rot_quat);
	sus[3]->setFrames(frameInA[3], frameInB[3]);
}



void CarRig::MakePhysics()
{
	int i;

	btQuaternion rot_quat;

	float softness = 1.0f;
	float bias = 1.0f;
	float relaxation = 1.0f;
	float wheelheight = -0.22f;

	float hit_fraction = 0.0f;

	float steer_ammount = 0.3f;
	float wheel_shine = 0.1f;

	wheel_x_off = cvalues.wheel_x_off *cvalues.car_scale;
	wheel_y_off = cvalues.wheel_y_off *cvalues.car_scale;
	wheel_z_off = cvalues.wheel_z_off *cvalues.car_scale;


	float lin_strength = 0.9f;
	float ang_strength = 0.5f;

	bool bEllipsoidWheels = false;

	ObjInfo dum_objInfo = {
		XMFLOAT3(0.0f, 5.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.1f, 1.3f),
		0
		, (COL_WHEEL)
		, (COL_WHEELLIFT) };

	btDefaultMotionState* floor = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 1.0f, 0.0f)));
	auto floorShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);//floor
	m_FloorPlane = m_phys->AddPhysicalObject(floorShape, floor, btVector3(0, 0, 0), &dum_objInfo);

	ObjInfo car_info = {
		XMFLOAT3(0.0f, 5.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(cvalues.body_mass, 0.1f, 1.3f),
		0
		, ( COL_OBJECTS | COL_WALLS | COL_CHAR | COL_TERRAIN)
		, (COL_CARBODY | COL_RAY) };

	carbody = new MeshMotion(m_deviceResources, m_phys);
	carbody->ob_info = car_info;
	//carbody->MakePhysicsCompoundBoxFromFBX(m_bodyModel[0], 0.0f, cvalues.body_floor_distance, 0.0f, 0.8f);
	carbody->MakePhysicsConvexHullFromFBX(m_bodyModel[0],1.0f);


	ObjInfo info;

	info = {
		XMFLOAT3(2.0f, 5.0f, -2.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(cvalues.wheel_mass, 0.5f, cvalues.back_friction),
		0
		, (COL_WHEELLIFT | COL_OBJECTS | COL_WALLS | COL_CHAR)
		, (COL_WHEEL | COL_RAY) };


	for (i = 0; i < 4; i++)
	{
		switch (i)
		{
			/* REAR LEFT */
		case 0:info.mrf.z = cvalues.back_friction; wheel_offset[i] = btVector3(wheel_x_off, wheel_y_off, -wheel_z_off); break;
			/* FRONT LEFT */
		case 1:info.mrf.z = cvalues.front_friction; wheel_offset[i] = btVector3(wheel_x_off, wheel_y_off, wheel_z_off); break;
			/* REAR RIGHT */
		case 2:info.mrf.z = cvalues.back_friction; wheel_offset[i] = btVector3(-wheel_x_off, wheel_y_off, -wheel_z_off); break;
			/* FRONT RIGHT */
		case 3:info.mrf.z = cvalues.front_friction; wheel_offset[i] = btVector3(-wheel_x_off, wheel_y_off, wheel_z_off); break;
		}


		wheel[i] = new MeshMotion(m_deviceResources, m_phys);
		wheel[i]->ob_info = info;
		if (bEllipsoidWheels == false)
		{
			wheel[i]->MakePhysicsSphereFromFBX(m_wheelModel[0], cvalues.wheel_rig_scale_to_model);
		}
		else
		{
			wheel[i]->MakePhysicsEllipseoidFromFBX(m_wheelModel[0], cvalues.wheel_rig_scale_to_model);
		}

		wheel[i]->m_rigidbody->setActivationState(DISABLE_DEACTIVATION);
		wheel[i]->m_rigidbody->setDamping(cvalues.wheel_linear_damping, cvalues.wheel_angular_damping);
		wheel[i]->m_rigidbody->setHitFraction(hit_fraction);

		if (cvalues.enable_antistropic_friction == true)
		{
			wheel[i]->m_rigidbody->setAnisotropicFriction(
				btVector3(cvalues.antistropic_friction_x, 
							cvalues.antistropic_friction_y, 
							cvalues.antistropic_friction_z), 1);
		}

		btVector3 pivotInA = wheel_offset[i];
		btVector3 pivotInB = btVector3(0.0f, 0.0f, 0.0f);
		btVector3 axisInA = btVector3(1.0f, 0.0f, 0.0f);
		btVector3 axisInB = btVector3(0.0f, 1.0f, 0.0f);

		frameInA[i] = btTransform::getIdentity();
		frameInA[i].setOrigin(pivotInA);
		frameInB[i] = btTransform::getIdentity();
		frameInB[i].setOrigin(pivotInB);

		rot_quat = btQuaternion(M_PI*0.5, 0.0f, 0.0f);
		frameInA[i].setRotation(rot_quat);

		sus[i] = new btGeneric6DofSpringConstraint(*carbody->m_rigidbody, *wheel[i]->m_rigidbody,
			frameInA[i],
			frameInB[i], true);

		sus[i]->setLinearUpperLimit(btVector3(0, cvalues.sus_travel, 0));
		sus[i]->setLinearLowerLimit(btVector3(0, 0, 0));
		sus[i]->setEquilibriumPoint(2, 0.0f);
		sus[i]->setAngularLowerLimit(btVector3(M_PI + M_PI*0.5f, 0.0f, 1.0f));
		sus[i]->setAngularUpperLimit(btVector3(M_PI + M_PI*0.5f, 0.0f, 0.0f));

		sus[i]->setDbgDrawSize(btScalar(5.f));

		sus[i]->enableSpring(1, true);
		sus[i]->setStiffness(1, cvalues.sus_stiffness);
		sus[i]->setDamping(1, cvalues.sus_damping);

		sus[i]->setParam(BT_CONSTRAINT_STOP_CFM, lin_strength, 0);
		sus[i]->setParam(BT_CONSTRAINT_STOP_CFM, lin_strength, 1);
		sus[i]->setParam(BT_CONSTRAINT_STOP_CFM, lin_strength, 2);
		sus[i]->setParam(BT_CONSTRAINT_STOP_CFM, ang_strength, 3);
		sus[i]->setParam(BT_CONSTRAINT_STOP_CFM, ang_strength, 4);
		sus[i]->setParam(BT_CONSTRAINT_STOP_CFM, ang_strength, 5);
		m_phys->m_dynamicsWorld->addConstraint(sus[i], true);
	}
}



void CarRig::RenderDepth(int alpha_mode, int point_plane)
{
	int i;

	if (m_Resources->m_Lights->CheckPointPlanes(point_plane, carbody->m_Matrix._14, carbody->m_Matrix._24, carbody->m_Matrix._34, carbody->m_model->Extents().Radius))
	{
		carbody->GetMatrix(m_Resources->ConstantModelBuffer());
		m_Resources->m_Camera->UpdateConstantBuffer();

		for (Mesh* m : m_bodyModel)
		{
			m->Render(*m_Resources);
		}
	}

	for (i = 0; i < 4; i++)
	{
		wheel[i]->MakeMatrix();
		if (m_Resources->m_Lights->CheckPointPlanes(point_plane, wheel[i]->m_Matrix._14, wheel[i]->m_Matrix._24, wheel[i]->m_Matrix._34, wheel[i]->m_model->Extents().Radius))
		{
			wheel[i]->GetMatrix(m_Resources->ConstantModelBuffer());
			
			if (i > 1) // turn wheel round
			{
				XMStoreFloat4x4(m_Resources->ConstantModelBuffer(), 
					XMLoadFloat4x4(m_Resources->ConstantModelBuffer()) * 
					XMMatrixRotationRollPitchYaw(0.0f, 0.0f, M_PI));
			}

			m_Resources->m_Camera->UpdateConstantBuffer();
			
			for (Mesh* m : m_wheelModel)
			{
				m->Render(*m_Resources);
			}
		}
	}
}

void CarRig::Render()
{
	int i;

	if (true)//(m_Resources->m_Lights->CheckPointPlanes(point_plane, carbody->m_Matrix._14, carbody->m_Matrix._24, carbody->m_Matrix._34, carbody->m_model->Extents().Radius))
	{
		carbody->GetMatrix(m_Resources->ConstantModelBuffer());
		m_Resources->m_Camera->UpdateConstantBuffer();

		for (Mesh* m : m_bodyModel)
		{
			m->Render(*m_Resources);
		}
	}

	for (i = 0; i < 4; i++)
	{
		wheel[i]->MakeMatrix();
		if (true)//(m_Resources->m_Lights->CheckPointPlanes(point_plane, wheel[i]->m_Matrix._14, wheel[i]->m_Matrix._24, wheel[i]->m_Matrix._34, wheel[i]->m_model->Extents().Radius))
		{
			wheel[i]->GetMatrix(m_Resources->ConstantModelBuffer());

			if (i > 1) // turn wheel round
			{
				XMStoreFloat4x4(m_Resources->ConstantModelBuffer(),
					XMLoadFloat4x4(m_Resources->ConstantModelBuffer()) *
					XMMatrixRotationRollPitchYaw(0.0f, 0.0f, M_PI));
			}

			m_Resources->m_Camera->UpdateConstantBuffer();

			for (Mesh* m : m_wheelModel)
			{
				m->Render(*m_Resources);
			}
		}
	}
}

void CarRig::SetPosition(float x, float y, float z, float yaw, float pitch, float roll)
{
	wheel[0]->SetPosition(x, y, z, yaw, pitch, roll - M_PI*0.5f);
	wheel[1]->SetPosition(x, y, z, yaw, pitch, roll - M_PI*0.5f);
	wheel[2]->SetPosition(x, y, z, yaw, pitch, roll - M_PI*0.5f);
	wheel[3]->SetPosition(x, y, z, yaw, pitch, roll - M_PI*0.5f);

	carbody->SetPosition(x, y, z, yaw, pitch, roll);
}

void CarRig::SetCarPosition(float x, float y, float z, float yaw, float pitch, float roll)
{
	btQuaternion rot_quat = btQuaternion(yaw, pitch, roll);

	btMatrix3x3 rotation = btMatrix3x3(rot_quat);

	btVector3 off0 = rotation * wheel_offset[0];
	btVector3 off1 = rotation * wheel_offset[1];
	btVector3 off2 = rotation * wheel_offset[2];
	btVector3 off3 = rotation * wheel_offset[3];

	wheel[0]->SetPosition(x + off0.getX() , y + off0.getY(), z + off0.getZ(), yaw, pitch, roll - M_PI*0.5f);// setWorldTransform(transform1);
	wheel[1]->SetPosition(x + off1.getX() , y + off1.getY(), z + off1.getZ(), yaw, pitch, roll - M_PI*0.5f);
	wheel[2]->SetPosition(x + off2.getX() , y + off2.getY(), z + off2.getZ(), yaw, pitch, roll - M_PI*0.5f);
	wheel[3]->SetPosition(x + off3.getX() , y + off3.getY(), z + off3.getZ(), yaw, pitch, roll - M_PI*0.5f);

	carbody->m_rigidbody->setLinearVelocity(btVector3(0.0f, 0.1f, 0.0f));
	carbody->m_rigidbody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));

	wheel[0]->m_rigidbody->setLinearVelocity(btVector3(0.0f, 0.1f, 0.0f));
	wheel[0]->m_rigidbody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
	wheel[1]->m_rigidbody->setLinearVelocity(btVector3(0.0f, 0.1f, 0.0f));
	wheel[1]->m_rigidbody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
	wheel[2]->m_rigidbody->setLinearVelocity(btVector3(0.0f, 0.1f, 0.0f));
	wheel[2]->m_rigidbody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
	wheel[3]->m_rigidbody->setLinearVelocity(btVector3(0.0f, 0.1f, 0.0f));
	wheel[3]->m_rigidbody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));

	carbody->SetPosition(x, y, z, yaw, pitch, roll);
}



CarRig::~CarRig(void)
{


}
