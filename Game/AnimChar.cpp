#include "pch.h"
#include "AnimChar.h"

using namespace Game;
using namespace DirectX;
using namespace DX;
using namespace std;

AnimChar::AnimChar(AllResources* p_Resources, Level* pp_Level):
m_loadingComplete(false)
{
	//pos = XMFLOAT3(0.0f,5.0f,)
	p_Level = pp_Level;
	m_Resources = p_Resources;

	view_angle = 0.0f;
	view_angle_to = 0.0f;
	view_angle_temp = 0.0f;
	//m_AnimChar = m_Resources->m_FBXModels->LoadFBX("bert", 0.4f);
	//m_bert->SetTexResource(0, m_Resources->m_Textures->LoadTexture("checker"), 0);

	model_pos_y = 0.5f;
	 
	view_pos_y = 0.0f;

	x_motion = 0.0f;
	z_motion = 0.0f;
	y_motion = 0.0f;

	x_motion_to = 0.0f;

	jump_anim = 0.0f;

	touch_jump = false;

	bJump = false;

	bAttack = false;

	attack_timer = 0.0f;

	attack_anim = 0.0f;

	bPlayerActive = false;
	
	walk_speed = 0.0f;

	touch_rotate = 0.0f;

	bWorldContact = false;

	bTouchAttack = false;

	char_angle = 0.0f;
	char_angle_to = 0.0f;
	

}

void AnimChar::Initialize()
{
	//m_cube = m_Resources->m_FBXModels->LoadFBX("player_body", cvalues.char_scale);



	//z - character
	//model_matrix = m_Resources->MakeMatrix(0.0f, 0.0f, 0.0f, -M_PI*0.5f, M_PI*0.5f,0.0f, char_scale);
	//knight
	//model_matrix = m_Resources->MakeMatrix(0.0f, 0.0f, 0.0f, M_PI*0.5f, M_PI*0.5f, 0.0f, char_scale);
	//lily
	//model_matrix = m_Resources->MakeMatrix(0.0f, 0.0f, 0.0f, 0.0f, M_PI*0.5f, 0.0f, char_scale);
	//elf
	//model_matrix = m_Resources->MakeMatrix(0.0f, 0.0f, 0.0f, -M_PI*0.5f, -M_PI*0.5f, 0.0f, char_scale);
	// Load the scene objects.

	model_matrix = m_Resources->MakeMatrix(cvalues.off_x, cvalues.off_y, cvalues.off_z, cvalues.rot_x, cvalues.rot_y, cvalues.rot_z, cvalues.mesh_scale);


	auto loadMeshTask = Mesh::LoadFromFileAsync(
		*m_Resources,
		cvalues.model_fname,
		L"",
		L"",
		m_meshModels,
		1.0f).then([this]()
	{

		m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture(cvalues.texture_fname), 0);
		m_meshModels[0]->SetMaterialNormal(L"", 0, m_Resources->m_Textures->LoadTexture(cvalues.normal_fname), 1.0f);
		//m_static_model[16]->m_meshModels[0]->SetMaterialTexture(L"", 0, m_Resources->m_Textures->LoadTexture("tree_bark"), 0);


		// Initialize animated models.
		for (Mesh* m : m_meshModels)
		{

			if (m->BoneInfoCollection().empty() == false)
			{
				auto animState = new AnimationState();
				animState->m_boneWorldTransforms.resize(m->BoneInfoCollection().size());
				animState->m_boneWorldTransformsB.resize(m->BoneInfoCollection().size());
				animState->m_boneWorldTransformsC.resize(m->BoneInfoCollection().size());
				animState->m_boneWorldTransformsD.resize(m->BoneInfoCollection().size());
				m->Tag = animState;
			}
		}

		// Each mesh object has its own "time" used to control the glow effect.
		m_time.clear();
		for (size_t i = 0; i < m_meshModels.size(); i++)
		{
			m_time.push_back(0.0f);
		}
	});

	m_skinnedMeshRenderer.Initialize(m_Resources->m_deviceResources->GetD3DDevice(), m_Resources->m_deviceResources->GetD3DDeviceContext(),m_Resources->m_materialBuffer);

	(loadMeshTask).then([this]()
	{
		// Scene is ready to be rendered.
		m_loadingComplete = true;

	});

}

void AnimChar::MakePhysics()
{

	ObjInfo bert_info = {
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.1f, 0.1f, 1.0f),
		0
		, (COL_WALLS | COL_CARBODY | COL_WHEEL | COL_OBJECTS)
		, (COL_CHAR) };

	m_bert_motion = new MeshMotion(m_Resources->m_deviceResources, &m_Resources->m_Physics, &bert_info);


	m_bert_motion->MakePhysicsCylinderExtents(1.0f,1.0f,1.0f, 0.5f);
	m_bert_motion->m_rigidbody->setDamping(0.0f, 0.2f);
	m_bert_motion->m_rigidbody->setActivationState(DISABLE_DEACTIVATION);


}

void AnimChar::SetPosition(float x, float y, float z, float yaw, float pitch, float roll)
{
	btCollisionWorld::ClosestRayResultCallback RayCallback(btVector3(x, y + 1.0f, z), btVector3(x, y - 100.0f, z));

	m_Resources->m_Physics.m_dynamicsWorld->rayTest(btVector3(x, y + 1.0f, z), btVector3(x, y - 100.0f, z), RayCallback);

	if (RayCallback.hasHit())
	{
		y = RayCallback.m_hitPointWorld.getY()+0.45f;

	}

	m_bert_motion->SetPosition(x, y, z,yaw,pitch,roll);

	bert_pos = XMFLOAT3(x, y, z);
	bert_mom = XMFLOAT3(0.0f, 0.0f, 0.0f);
}



void AnimChar::Update(float timeDelta, float timeTotal)
{
	btTransform block_tran;

	if (bPlayerActive == false || m_loadingComplete==false)
		return;

	
	// Update animated models.

	if(true)///(bKeyDown == false)
	{
		btTransform tran = m_bert_motion->m_rigidbody->getWorldTransform();
		float xdif = abs(bert_pos.x - tran.getOrigin().getX());
		float zdif = abs(bert_pos.z - tran.getOrigin().getZ());
		//float ydif = abs(bert_pos.y - tran.getOrigin().getY());
		
		/*
		btVector3 vec_dif = btVector3(abs(bert_pos.x - tran.getOrigin().getX()), abs(bert_pos.y - tran.getOrigin().getY()), abs(bert_pos.z - tran.getOrigin().getZ()));

		if (xdif > 0.000005f && xdif<1.0f &&
			ydif>0.000005f && ydif<1.0f &&
			zdif>0.000005f && zdif < 1.0f)
		{
			if (vec_dif.length() > 1.0f)
				vec_dif.normalize();

			bert_pos.x = vec_dif.getX();
			bert_pos.y = vec_dif.getY();
			bert_pos.z = vec_dif.getZ();
		}
		*/

		
		if(xdif>0.000005f && xdif<1.0f)
			bert_pos.x = tran.getOrigin().getX();

		//bert_pos.y = tran.getOrigin().getY();
		if(zdif>0.000005f && zdif<1.0f)
			bert_pos.z = tran.getOrigin().getZ();
		/*
		if (zdif>0.000005f && zdif<1.0f)
			bert_pos.z = tran.getOrigin().getZ();
			*/
		/*
		btVector3 vel = m_bert_motion->m_rigidbody->getLinearVelocity();
		bert_mom.x = vel.getX();// *timeDelta;
		//bert_mom.y = vel.getY()*timeDelta*4.0f;
		bert_mom.z = vel.getZ();// *timeDelta;
		*/
	}

	/*
	if (m_Resources->m_PadInput->PanCommandX() != 0.0f)
	{
		view_angle += m_Resources->m_PadInput->PanCommandX()*0.04f;
		if (view_angle > M_PI*2.0f)
			view_angle -= M_PI*2.0f;

		if (view_angle <0)
			view_angle += M_PI*2.0f;
	}
	else
	if (touch_rotate != 0.0f)
	{
		view_angle += touch_rotate;
		if (view_angle > M_PI*2.0f)
			view_angle -= M_PI*2.0f;

		if (view_angle <0)
			view_angle += M_PI*2.0f;
	}
	else
	if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::Left) == true)
	{
		view_angle -= 0.04f;
		if (view_angle <0)
			view_angle += M_PI*2.0f;
	}
	else
	if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::Right) == true)
	{
		view_angle += 0.04f;
		if (view_angle > M_PI*2.0f)
			view_angle -= M_PI*2.0f;

	}
	*/
	//else
	{
		//view_angle += 10.0f;
		//char_angle += 10.0f;
		view_angle_to = char_angle;

		if (view_angle_to > M_PI*2.0f)
			view_angle_to -= M_PI*2.0f;

		if (view_angle_temp<0.0f)
			view_angle_temp += M_PI*2.0f;

		if (view_angle_temp>M_PI*2.0f)
			view_angle_temp -= M_PI*2.0f;

		if ((view_angle_to - view_angle_temp)>M_PI)
		{
			view_angle_to -= M_PI*2.0f;
		}
		else
		{
			if ((view_angle_to - view_angle_temp)<-M_PI)
			{
				view_angle_to += M_PI*2.0f;
			}
		}

		float diffr = abs(abs(view_angle_temp) - abs(view_angle_to));
		if (diffr > 0.05f)
		{
			diffr = 0.05f;
		}

		if (view_angle_temp > view_angle_to)
			view_angle_temp -= diffr;
		if (view_angle_temp < view_angle_to)
			view_angle_temp += diffr;

		view_angle = view_angle_temp - M_PI*0.5f;
		/*

		//view_angle_to = view_angle;// +(M_PI*0.5f);
		if (true)//(char_angle > M_PI*0.5f)
		{
			view_angle_to -= (view_angle_to - (char_angle))*timeDelta*3.0f;
		}
		else
		{
			//view_angle -= (view_angle - (char_angle + (M_PI*1.5f)))*timeDelta*3.0f;
		}
		view_angle = view_angle_to-(M_PI*0.5f);
		//view_angle -= 10.0f;
		//char_angle -= 10.0f;
*/
		//if (view_angle <0)
		//	view_angle += M_PI*2.0f;
		//if (view_angle > M_PI*2.0f)
		//	view_angle -= M_PI*2.0f;
	}



	bKeyDown = false;

	if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::Space) == true || m_Resources->m_PadInput->ButtonX() == true || bTouchAttack == true)
	{
		if (bAttack == false)
		{
			m_skinnedMeshRenderer.ResetAnimTimer(3);
			bAttack = true;
			attack_timer = 0.0f;
		}

	}

	if (bAttack == true)
	{
		if (attack_anim < 1.0f)
			attack_anim += 0.2f;
		if (attack_anim > 1.0f)
			attack_anim = 1.0f;

		attack_timer += timeDelta;
		if (attack_timer > 0.26f)
		{
			//bAttack = false;
			if (attack_anim > 0.0f)
				attack_anim -= 0.2f;
			if (attack_anim < 0.0f)
				attack_anim = 0.0f;
		}

		if (attack_timer > 0.3f)
		{
			bAttack = false;
		}
	}
	else
	{
		//bAttack = false;
		if (attack_anim > 0.0f)
			attack_anim -= 0.1f;
		if (attack_anim < 0.0f)
			attack_anim = 0.0f;
	}
	x_motion = 0.0f;
	z_motion = 0.0f;

	if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::W) == true && bAttack==false)
	{
		z_motion += 10.0f;
		bKeyDown = true;
	}
	if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::S) == true && bAttack == false)
	{
		z_motion -= 10.0f;
		bKeyDown = true;
	}
	if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::A) == true && bAttack == false)
	{
		if (x_motion_to < 0.0f)
			x_motion_to = 0.0f;
		x_motion_to += 0.3f;
		if (x_motion_to > 3.0f)
			x_motion_to = 3.0f;
		x_motion += x_motion_to;
		//x_motion += 5.0f;
		bKeyDown = true;
	}
	//m_Resources->m_PadwddInput->
	if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::D) == true && bAttack == false)
	{
		if (x_motion_to > 0.0f)
			x_motion_to = 0.0f;
		x_motion_to -= 0.3f;
		if (x_motion_to < -3.0f)
			x_motion_to = -3.0f;
		x_motion += x_motion_to;
		//x_motion -= 5.0f;
		bKeyDown = true;
	}
	y_motion = 0.0f;

	x_motion_to *= 0.1f;

	if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::R) == true)
	{
		if (view_pos_y < 10.0f)
		{
			view_pos_y += 0.1f;
		}
		bKeyDown = true;
	}
	
	if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::F) == true)
	{
		if (view_pos_y > 0.0f)
		{
			view_pos_y -= 0.1f;
		}
		bKeyDown = true;
	}


	if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::Shift) == true || m_Resources->m_PadInput->ButtonA()==true || touch_jump==true)
	{
		if (bJump == false && bWorldContact==true)
		{
			bJump = true;
			touch_jump = false;
			y_motion = 10.0f;
			bert_mom.y = 0.2f;
			//m_Resources->m_audio.PlaySoundEffect(SFXJump);
			m_skinnedMeshRenderer.ResetAnimTimer(2);
		}
	}
	else
	{
		bJump = false;
	}

	if (touch_move_x!=0.0f || touch_move_y!=0.0f)
	{
		float x, z;
		
		x = -(touch_move_x-90.0f);
		z = -(touch_move_y-90.0f)*0.01;

		if(x<20.0f && x>-20.0f)
		{
			x = 0.0f;
		}
		else
		{
			if (x > 20.0f)
				x -= 20.0f;
			else
				if (x < -20.0f)
					x += 20.0f;
		}

		x *= 0.01f;

		x_motion = x *10.0f;
		z_motion = z *10.0f;
		bKeyDown = true;
	}


	if ((m_Resources->m_PadInput->MoveCommandX() != 0.0f || m_Resources->m_PadInput->MoveCommandY() != 0.0f) && bAttack==false)
	{
		x_motion = -m_Resources->m_PadInput->MoveCommandX();
		z_motion = m_Resources->m_PadInput->MoveCommandY();
		bKeyDown = true;
	}




	btVector3 v2 = btVector3(x_motion, 0.0f, z_motion);
	v2.normalize();
	btVector3 v3 = v2.rotate(btVector3(0.0f, 1.0f, 0.0f), -view_angle);



	//v2.a


	if(bKeyDown == true)
	{
		char_angle_to = atan2f(v3.getZ(), v3.getX());// +view_angle;

		if (char_angle_to > M_PI*2.0f)
			char_angle_to -= M_PI*2.0f;

		if (char_angle<0.0f)
			char_angle += M_PI*2.0f;

		if (char_angle>M_PI*2.0f)
			char_angle -= M_PI*2.0f;

		if ((char_angle_to - char_angle)>M_PI)
		{
			char_angle_to -= M_PI*2.0f;
		}
		else
		{
			if ((char_angle_to - char_angle)<-M_PI)
			{
				char_angle_to += M_PI*2.0f;
			}
		}

		float diffr = abs(abs(char_angle) - abs(char_angle_to));
		if (diffr > 0.2f)
		{
			diffr = 0.2f;
		}


		if (char_angle < char_angle_to)
			char_angle += diffr;
		if (char_angle > char_angle_to)
			char_angle -= diffr;
		// knight
		//DirectX::XMFLOAT4X4 matr = m_Resources->MakeMatrix(0.0f, 0.0f, 0.0f, M_PI*0.5f, M_PI*0.5f - char_angle, 0.0f, char_scale);
		// lily
		//DirectX::XMFLOAT4X4 matr = m_Resources->MakeMatrix(0.0f, 0.0f, 0.0f, 0.0f, M_PI*0.5f - ang, 0.0f, char_scale);
		// Elf
		DirectX::XMFLOAT4X4 matr = m_Resources->MakeMatrix(cvalues.off_x, cvalues.off_y, cvalues.off_z, cvalues.rot_x, cvalues.rot_y - char_angle, cvalues.rot_z, cvalues.mesh_scale);


		model_matrix._11 = matr._11;
		model_matrix._12 = matr._12;
		model_matrix._13 = matr._13;
		model_matrix._21 = matr._21;
		model_matrix._22 = matr._22;
		model_matrix._23 = matr._23;
		model_matrix._31 = matr._31;
		model_matrix._32 = matr._32;
		model_matrix._33 = matr._33;

		if (walk_speed < cvalues.max_walk_speed)
			walk_speed += timeDelta*18.0f;

		if (walk_speed >= cvalues.max_walk_speed)
			walk_speed = cvalues.max_walk_speed;
	}
	else
	{
		if (walk_speed > 0.0f)
			walk_speed -= timeDelta*12.0f;

		if (walk_speed <= 0.0f)
			walk_speed = 0.0f;
	}
	//x_motion = v3.getX() *walk_speed;
	//z_motion = v3.getZ() *walk_speed;

	x_motion = cos(char_angle) *walk_speed;
	z_motion = sin(char_angle) *walk_speed;

	btCollisionWorld::ClosestRayResultCallback RayCallbackB(btVector3(bert_pos.x, bert_pos.y+1.0f, bert_pos.z), btVector3(bert_pos.x, bert_pos.y + 10.0f, bert_pos.z));
	m_Resources->m_Physics.m_dynamicsWorld->rayTest(btVector3(bert_pos.x, bert_pos.y+1.0f , bert_pos.z), btVector3(bert_pos.x, bert_pos.y + 10.0f, bert_pos.z), RayCallbackB);
	if (RayCallbackB.hasHit())
	{
		btVector3 pos = RayCallbackB.m_hitPointWorld;
		float dist = pos.getY() - model_matrix._24;
		if (dist < 0.3f)
		{
			bert_pos.y = pos.getY() - 0.3f;
			bert_mom.y = 0.0f;
		}
	
	}
	
	btCollisionWorld::ClosestRayResultCallback RayCallback(btVector3(bert_pos.x, bert_pos.y+1.0f, bert_pos.z), btVector3(bert_pos.x, bert_pos.y - 100.0f, bert_pos.z));
	m_Resources->m_Physics.m_dynamicsWorld->rayTest(btVector3(bert_pos.x, bert_pos.y + 1.0f, bert_pos.z), btVector3(bert_pos.x, bert_pos.y - 100.0f, bert_pos.z), RayCallback);
	if (RayCallback.hasHit())
	{
		btVector3 pos = RayCallback.m_hitPointWorld;
		float dist = model_matrix._24 - pos.getY();

		if (dist > 0.36f)
		{
			if (jump_anim < 1.0f)
				jump_anim += 0.2f;
			if (jump_anim > 1.0f)
				jump_anim = 1.0f;
						
			bert_mom.y -= timeDelta*0.5f;
			bWorldContact = false;
			bJump = false;
		}
		else
		{
			if (jump_anim > 0.0f)
				jump_anim -= 0.2f;
			if (jump_anim < 0.0f)
				jump_anim = 0.0f;

			bert_pos.y = pos.getY() + 0.25f;

			if(bJump==false)
			{
				bert_mom.y = 0.0f;
			}
			bWorldContact = true;
		}
	}
	else
	{
		if (jump_anim > 0.0f)
			jump_anim -= 0.2f;
		if (jump_anim < 0.0f)
			jump_anim = 0.0f;

		bWorldContact = false;
		if (bJump == false)
		{
			bert_mom.y = 0.0f;
		}
	}

	float anim_speed = timeDelta;
	m_skinnedMeshRenderer.UpdateAnimationMix(timeDelta, m_meshModels, (1.0f / cvalues.max_walk_speed)*(walk_speed), jump_anim, attack_anim);
	//m_skinnedMeshRenderer.UpdateAnimation(timeDelta, m_meshModels, L"my_avatar|run");

	bert_pos_last = bert_pos;
	//bert_pos.x += x_motion *timeDelta;
	bert_pos.y += bert_mom.y;
	//bert_pos.z += z_motion*timeDelta;
	m_bert_motion->SetPosition(bert_pos.x, bert_pos.y+0.5f, bert_pos.z,0.0f,0.0f,0.0f);
	m_bert_motion->m_rigidbody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));

	//view_angle += 10.0f;
	//char_angle += 10.0f;

	float t_height = p_Level->GetTerrainHeight(bert_pos.x, bert_pos.z);
	float t_height2 = p_Level->GetTerrainHeight(bert_pos.x+x_motion, bert_pos.z+z_motion);


	//x_motion *= (p_Level->GetNormal(bert_pos.x+ x_motion, bert_pos.z+ z_motion).getY()-0.4f);
	//z_motion *= (p_Level->GetNormal(bert_pos.x+ x_motion, bert_pos.z+ z_motion).getY() - 0.4f);
	//view_angle -= 10.0f;
	//char_angle -= 10.0f;

	if(true)
	{
		
		m_bert_motion->m_rigidbody->setLinearVelocity(btVector3(x_motion*0.5f, 0.0f, z_motion*0.5f));
	}
	else
	{
		//m_bert_motion->m_rigidbody->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
	}
	
}

btVector3* AnimChar::GetPosition()
{
	static btVector3 player_pos;

	player_pos = btVector3(bert_pos.x, bert_pos.y, bert_pos.z);

	return &player_pos;
}

void AnimChar::RenderDepth(int alpha_mode, int point_plane)
{
	if (bPlayerActive == false)
		return;

	XMFLOAT4X4 model_matrix_b;

	bool bRender;

	//m_bert_motion->MakeMatrix();
	m_bert_motion->GetMatrix(m_Resources->ConstantModelBuffer());

	model_matrix._14 = bert_pos.x;
	model_matrix._24 = bert_pos.y;
	model_matrix._34 = bert_pos.z;

	model_matrix_b = model_matrix;

	model_matrix_b._24 += model_pos_y;

	m_Resources->m_Camera->SetModelMatrix(&model_matrix);

	m_Resources->m_Camera->UpdateConstantBuffer();

	//model_matrix = m_Resources->MakeMatrix(0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f);
	//m_bert->Render(alpha_mode);


	//SubMesh mesh = m_meshModels[0]->SubMeshes();


	if (m_loadingComplete == true)
	{
		//m_Resources->SetTexture(cvalues.texture_fname, 0);



		//m_Resources->SetSkinShader();

		//m_Resources->m_Lights->UpdateConstantBuffer();
		//m_Resources->m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_Resources->m_materialBuffer.Get(), 0, nullptr, &cvalues.model_material, 0, 0);
		bRender = true;
		if (point_plane > -1)
		{
			// check point light frustum
			bRender = m_Resources->m_Lights->CheckPointPlanes(point_plane, bert_pos.x, bert_pos.y, bert_pos.z, m_meshModels[0]->Extents().Radius);
			//bRender = true;
		}

		if (bRender == true)//m_Resources->m_Lights->CheckPoint(m_static[p_index[i].part_no].x, m_static[p_index[i].part_no].y, m_static[p_index[i].part_no].z, m_static_model[m_static[p_index[i].part_no].model_index]->m_meshModels[0]->Extents().Radius, nullptr) > 0.0f)
		{


			for (UINT i = 0; i < m_meshModels.size(); i++)
			{
				if (m_meshModels[i]->alpha_mode == alpha_mode)
				{
					if (m_meshModels[i]->Tag != nullptr)
					{
						//m_meshModels[i]->Render(*m_Resources);
						m_skinnedMeshRenderer.RenderSkinnedMesh(m_meshModels[i], *m_Resources);
					}
				}
			}
		}

		/*
		m_Resources->SetMeshShader();
		for (UINT i = 0; i < m_meshModels.size(); i++)
		{
			if (m_meshModels[i]->Tag == nullptr)
			{

				UINT stride = sizeof(Vertex);
				UINT offset = 0;
				for (const Game::Mesh::SubMesh& submesh : m_meshModels[i]->SubMeshes())
				{
					const ID3D11Buffer* vbs = m_meshModels[i]->VertexBuffers()[submesh.VertexBufferIndex];

					m_Resources->m_deviceResources->GetD3DDeviceContext()->IASetIndexBuffer(m_meshModels[i]->IndexBuffers()[submesh.IndexBufferIndex], DXGI_FORMAT_R16_UINT, 0);
					m_Resources->m_deviceResources->GetD3DDeviceContext()->IASetVertexBuffers(0, 1, &m_meshModels[i]->VertexBuffers()[submesh.VertexBufferIndex], &stride, &offset);

					m_Resources->m_deviceResources->GetD3DDeviceContext()->DrawIndexed(submesh.PrimCount * 3, submesh.StartIndex, 0);
				}
			}
		}
		*/
	}


}

void AnimChar::Render(int alpha_mode, bool _bdef)
{
	if (bPlayerActive == false)
		return;

	XMFLOAT4X4 model_matrix_b;

	//m_bert_motion->MakeMatrix();
	m_bert_motion->GetMatrix(m_Resources->ConstantModelBuffer());
	
	model_matrix._14 = bert_pos.x;
	model_matrix._24 = bert_pos.y;
	model_matrix._34 = bert_pos.z;

	model_matrix_b = model_matrix;

	model_matrix_b._24 += model_pos_y;

	m_Resources->m_Camera->SetModelMatrix(&model_matrix_b);

	m_Resources->m_Camera->UpdateConstantBuffer();

	//model_matrix = m_Resources->MakeMatrix(0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f);
	//m_bert->Render(alpha_mode);


	//SubMesh mesh = m_meshModels[0]->SubMeshes();


	if (m_loadingComplete == true)
	{
		//m_Resources->SetTexture(cvalues.texture_fname, 0);
		//m_Resources->SetTexture(cvalues.normal_fname, 3);

		if (_bdef == true)
		{
			m_Resources->SetDefSkinShader();
		}
		else
		{
			m_Resources->SetSkinShader();
		}
		//m_Resources->m_Lights->UpdateConstantBuffer();
		m_Resources->m_deviceResources->GetD3DDeviceContext()->UpdateSubresource(m_Resources->m_materialBuffer.Get(), 0, nullptr, &cvalues.model_material, 0, 0);

		for (UINT i = 0; i < m_meshModels.size(); i++)
		{
			if (m_meshModels[i]->Tag != nullptr && m_meshModels[i]->alpha_mode== alpha_mode)
			{
				//m_meshModels[i]->Render(*m_Resources);
				m_skinnedMeshRenderer.RenderSkinnedMesh(m_meshModels[i], *m_Resources);
			}
		}


		if (_bdef == true)
		{
			m_Resources->SetDeferredShader();
		}
		else
		{
			m_Resources->SetMeshShader();
		}
		for (UINT i = 0; i < m_meshModels.size(); i++)
		{
			if (m_meshModels[i]->Tag == nullptr && m_meshModels[i]->alpha_mode == alpha_mode)
			{

				UINT stride = sizeof(Vertex);
				UINT offset = 0;
				for (const Game::Mesh::SubMesh& submesh : m_meshModels[i]->SubMeshes())
				{
					const ID3D11Buffer* vbs = m_meshModels[i]->VertexBuffers()[submesh.VertexBufferIndex];

					m_Resources->m_deviceResources->GetD3DDeviceContext()->IASetIndexBuffer(m_meshModels[i]->IndexBuffers()[submesh.IndexBufferIndex], DXGI_FORMAT_R16_UINT, 0);
					m_Resources->m_deviceResources->GetD3DDeviceContext()->IASetVertexBuffers(0, 1, &m_meshModels[i]->VertexBuffers()[submesh.VertexBufferIndex], &stride, &offset);

					m_Resources->m_deviceResources->GetD3DDeviceContext()->DrawIndexed(submesh.PrimCount * 3, submesh.StartIndex, 0);
				}
			}
		}
	}

	
	m_Resources->SetShinyShader();

	// to see See the cylinder
	 /*
	m_bert_motion->MakeMatrix();
	m_bert_motion->GetMatrix(m_Resources->ConstantModelBuffer());
	m_Resources->m_Camera->UpdateConstantBuffer();
	m_cube->Render(alpha_mode);
	*/
}