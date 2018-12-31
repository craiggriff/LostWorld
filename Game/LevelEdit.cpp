#include "pch.h"
#include "LevelEdit.h"

using namespace Game;
using namespace DirectX;
using namespace DX;
using namespace Microsoft::WRL;
using namespace concurrency;
using namespace Windows::System::Threading;

LevelEdit::LevelEdit(AllResources* p_Resources, Level* pp_Level, Statics* pp_Statics, Stuff* pp_Stuff) :
	m_Resources(p_Resources), p_Level(pp_Level), p_Statics(pp_Statics), p_Stuff(pp_Stuff)
{
	m_deviceResources = m_Resources->m_deviceResources;
	bLoaded = false;
	//m_Marker = new ModelLoader(m_deviceResources);
	//m_Marker = m_Resources->m_Models->LoadModel("marker",1.0f);


	Mesh::LoadFromFile(
		*m_Resources,
		L"marker.cmo",
		L"",
		L"",
		m_Marker,
		1.0f);
	
	m_Marker[0]->SetMaterialTexture(L"mat1", 0, m_Resources->m_Textures->LoadTexture("Granite"), 0);


	stretch_width = 5.0f;
	current_editing = 1;// terrain;
	bEditing = false;

	pointer_scale = 1.0f;
	pointer_rot_y = 0.0f;
	pointer_above = 0.0f;
	pointer_rot_x = 0.0f;
	pointer_rot_z = 0.0f;

	last_place_x = 0.0f;
	last_place_z = 0.0f;

	bMouseClicked = false;

	bLoopPath = true;
	bDipPath = true;

	bGenerateTerrain = false;

	//g_sphere = m_Resources->m_FBXModels->LoadFBX("g_sphere", 1.0f);
	//g_sphere->SetTexResource(0, m_Resources->m_Textures->LoadTexture("heart"), 1);

	
	Mesh::LoadFromFile(
		*m_Resources,
		L"g_sphere.cmo",
		L"",
		L"",
		g_sphere,
		1.0f);
	
	g_sphere[0]->SetMaterialTexture(L"mat1", 0, m_Resources->m_Textures->LoadTexture("heart"), 0);

	

	//g_sphere->MeshMaterial(0)->specularPow = 5.0f;
	//g_sphere->MeshMaterial(0)->specularLvl = 2.0f;
	//g_sphere->MeshMaterial(0)->diffuseLvl = 0.5f;
	//g_sphere->MeshMaterial(0)->vertexCol = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.1f);
	bMakePath=false;
	bClearPaths=false;
	bNewPath=false;


	bLoaded = true;

	design_spacing = 1;

	current_blend_edit = 0;

	t1val = 0.0f;
	t2val = 0.0f;
	t3val = 0.0f;
	t4val = 0.0f;

	
	bPointerPressed = false;
}

void LevelEdit::SetBlendNum(int _num)
{
	current_blend_edit = _num;
	p_Level->m_Paths->SetBlendArray(p_Level->tex_blend[current_blend_edit]);

}

void LevelEdit::RenderGSphere()
{
	if (bLoaded == false)
		return;

	m_Resources->m_Camera->SetModelMatrix( &m_Resources->MakeMatrix(pos_x, pos_y, pos_z, 0.0f, 0.0f, 0.0f, stretch_width));
	m_Resources->m_Camera->UpdateConstantBuffer();
	g_sphere[0]->Render(*m_Resources);
}


void LevelEdit::Update(DX::StepTimer const& timer)
{
	XMFLOAT3 dir;
	float t_height;

	bool bTerrainHit=false;
	XMFLOAT3 hit_pos;

	mouse_point = m_Resources->m_PadInput->GetPointerPos();
	if (mouse_point.X>m_Resources->m_Camera->outputSize.Width - 300)
	{
		return;
	}

	if (bClearPaths == true)
	{
		bClearPaths = false;
		p_Level->ClearBlendEdit(current_blend_edit);

		//p_Level->m_Paths->ClearArrays();
		p_Level->UpdateTerrain(true);
	}

	if (bNewPath == true)
	{
		bNewPath = false;
		p_Level->m_Paths->ClearPath(0);
		
	}

	// Make Path
	if (bMakePath == true)
	{
		bMakePath = false;

		p_Level->ClearPlaneUpdate();

		if( bLoopPath==true)
		{
			if (p_Level->m_Paths->GetTotalPoints(0) > 3)
			{
				for (int i = 0; i < p_Level->m_Paths->GetTotalPoints(0); i++)
				{
					for (int j = 0; j < 100; j++)
					{
						XMFLOAT3 pos;
						XMStoreFloat3(&pos, p_Level->m_Paths->GetCatRomPoint(0, i, (float)j*0.01f));

						p_Level->UpdateGeneratedTerrainPath(pos.z, pos.y, pos.x, stretch_width, 1,bDipPath);
					}
				}
			}
		}
		else
		{
			if (p_Level->m_Paths->GetTotalPoints(0) > 3)
			{
				for (int i = 1; i < p_Level->m_Paths->GetTotalPoints(0)-2; i++)
				{
					for (int j = 0; j < 100; j++)
					{
						XMFLOAT3 pos;
						XMStoreFloat3(&pos, p_Level->m_Paths->GetCatRomPoint(0, i, (float)j*0.01f));

						p_Level->UpdateGeneratedTerrainPath(pos.z, pos.y, pos.x, stretch_width, 1, bDipPath);
					}
				}

				PathPoint point1 = p_Level->m_Paths->GetPoints(0).at(0);
				PathPoint point2 = p_Level->m_Paths->GetPoints(0).at(1);
				btVector3 vpoint1 = btVector3(point1.pos.x, point1.pos.y, point1.pos.z);
				btVector3 vpoint2 = btVector3(point2.pos.x, point2.pos.y, point2.pos.z);
				for (int j = 0; j < 100; j++)
				{
					btVector3 lpoint = vpoint1.lerp(vpoint2, (float)j*0.01f);

					p_Level->UpdateGeneratedTerrainPath(lpoint.getZ(), lpoint.getY(), lpoint.getX(), stretch_width, 1, bDipPath);
				}
				point1 = p_Level->m_Paths->GetPoints(0).at(p_Level->m_Paths->GetTotalPoints(0)-2);
				point2 = p_Level->m_Paths->GetPoints(0).at(p_Level->m_Paths->GetTotalPoints(0)-1);
				vpoint1 = btVector3(point1.pos.x, point1.pos.y, point1.pos.z);
				vpoint2 = btVector3(point2.pos.x, point2.pos.y, point2.pos.z);
				for (int j = 0; j < 100; j++)
				{
					btVector3 lpoint = vpoint1.lerp(vpoint2, (float)j*0.01f);

					p_Level->UpdateGeneratedTerrainPath(lpoint.getZ(), lpoint.getY(), lpoint.getX(), stretch_width, 1, bDipPath);
				}

			}
			else
			{
				if (p_Level->m_Paths->GetTotalPoints(0) > 2)
				{
					PathPoint point1 = p_Level->m_Paths->GetPoints(0).at(0);
					PathPoint point2 = p_Level->m_Paths->GetPoints(0).at(1);
					btVector3 vpoint1 = btVector3(point1.pos.x, point1.pos.y, point1.pos.z);
					btVector3 vpoint2 = btVector3(point2.pos.x, point2.pos.y, point2.pos.z);
					for (int j = 0; j < 100; j++)
					{
						btVector3 lpoint = vpoint1.lerp(vpoint2, (float)j*0.01f);

						p_Level->UpdateGeneratedTerrainPath(lpoint.getZ(), lpoint.getY(), lpoint.getX(), stretch_width, 1, bDipPath);
					}
					point1 = p_Level->m_Paths->GetPoints(0).at(1);
					point2 = p_Level->m_Paths->GetPoints(0).at(2);
					vpoint1 = btVector3(point1.pos.x, point1.pos.y, point1.pos.z);
					vpoint2 = btVector3(point2.pos.x, point2.pos.y, point2.pos.z);
					for (int j = 0; j < 100; j++)
					{
						btVector3 lpoint = vpoint1.lerp(vpoint2, (float)j*0.01f);

						p_Level->UpdateGeneratedTerrainPath(lpoint.getZ(), lpoint.getY(), lpoint.getX(), stretch_width, 1, bDipPath);
					}
				}
				else
				{
					if (p_Level->m_Paths->GetTotalPoints(0) > 1)
					{
						PathPoint point1 = p_Level->m_Paths->GetPoints(0).at(0);
						PathPoint point2 = p_Level->m_Paths->GetPoints(0).at(1);
						btVector3 vpoint1 = btVector3(point1.pos.x, point1.pos.y, point1.pos.z);
						btVector3 vpoint2 = btVector3(point2.pos.x, point2.pos.y, point2.pos.z);
						for (int j = 0; j < 100; j++)
						{
							btVector3 lpoint = vpoint1.lerp(vpoint2, (float)j*0.01f);

							p_Level->UpdateGeneratedTerrainPath(lpoint.getZ(), lpoint.getY(), lpoint.getX(), stretch_width, 1, bDipPath);
						}
					}
				}
			}

		}
		
		p_Level->UpdatePlaneUpdate();
		SetStaticsHeightToTerrrain();

		return;
	}

	if (bGenerateTerrain == true)
	{
		bGenerateTerrain = false;
		
		p_Level->MakeNewTerrain(t1val, t2val, t3val, t4val);

		p_Level->UpdatePlaneUpdate(true);
		SetStaticsHeightToTerrrain();
	}

	
	float pointX = mouse_point.X;// -m_Resources->m_Camera->outputSize.Width*0.5f;// (mouse_point.X*2.0f) - mouse_point.X;
	float pointY = mouse_point.Y;// -m_Resources->m_Camera->outputSize.Height*0.5f;// (mouse_point.Y*2.0f) - mouse_point.Y;

	pos = m_Resources->m_Camera->GetEye();
	
	m_Resources->m_Camera->GetWorldLine((UINT)pointX, (UINT)pointY, &pos, &dir);

	btVector3 vec = btVector3(dir.x, dir.y, dir.z);
	//vec.normalize();
	//dir.x = vec.getX();
	//dir.y = vec.getY();
	//dir.z = vec.getZ();
	pos = m_Resources->m_Camera->GetEye();

	m_Resources->m_uiControl->SetInfoVal(0, pos.x);
	m_Resources->m_uiControl->SetInfoVal(1, pos.y);
	m_Resources->m_uiControl->SetInfoVal(2, pos.z);

	m_Resources->m_uiControl->SetInfoVal(3, dir.x);
	m_Resources->m_uiControl->SetInfoVal(4, dir.y);
	m_Resources->m_uiControl->SetInfoVal(5, dir.z);



	for (int i = 0; i < 220; i++)
	{
		t_height = p_Level->GetTerrainHeight(pos.x, pos.z);
		if (t_height > pos.y)
		{
			if (design_spacing > 1)
			{
				pos.x = ((int)pos.x / design_spacing)*design_spacing;// +(int)pos.x % design_spacing;
				pos.z = ((int)pos.z / design_spacing)*design_spacing;// (int)pos.z +(int)pos.z % design_spacing;
			}
			else
			{
				pos.x = (int)pos.x;// +(int)pos.x % 2;
				pos.z = (int)pos.z;// +(int)pos.z % 2;
			}



			t_height = p_Level->GetTerrainHeight(pos.x, pos.z);
			hit_pos.x = pos.x;
			hit_pos.y = t_height;
			hit_pos.z = pos.z;

			if (current_editing > 99 && current_editing < 199 || current_editing > 299 && current_editing < 399)
			{
				btCollisionWorld::ClosestRayResultCallback RayCallback(btVector3(hit_pos.x, hit_pos.y + 10.0f, hit_pos.z), btVector3(hit_pos.x, hit_pos.y - 100.0f, hit_pos.z));

				m_Resources->m_Physics.m_dynamicsWorld->rayTest(btVector3(hit_pos.x, hit_pos.y + 10.0f, hit_pos.z), btVector3(hit_pos.x, hit_pos.y - 100.0f, hit_pos.z), RayCallback);

				if (RayCallback.hasHit())
				{
					pointer_above = RayCallback.m_hitPointWorld.getY() - hit_pos.y;
				}
			}
			
			//m_MO[0]->ResetToPosition(XMFLOAT3(pos.x, t_height + 1.0f, pos.z));
			bTerrainHit = true;
		}
		else
		{
			pos.x = pos.x + dir.x;
			pos.y = pos.y + dir.y;
			pos.z = pos.z + dir.z;
		}
		if (bTerrainHit == true)
			break;

	}



	if (current_editing == 1)
	{
		if (m_Resources->m_PadInput->GetPointerPressed() == false)
		{
			if (bEditing == true)
			{
				p_Level->BakePinch();
				SetStaticsHeightToTerrrain();
			}


			if (bTerrainHit == true)
			{
				//m_Marker->SetPosition(pos.x, pos.y, pos.z);
				
				pos_x = pos.x;
				pos_y = pos.y;
				pos_z = pos.z;

				if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::M) == true)
				{
					if (pos.x != last_pos.x || pos.z != last_pos.z)
					{
						int i;
						float x_dist = (last_pos.x - pos.x)*0.2f;
						float z_dist = (last_pos.z - pos.z)*0.2f;
						p_Level->ClearPlaneUpdate();

						for (i = 0; i < 5; i++)
						{
							p_Level->UpdateGeneratedTerrainPinch(1, 0.0f, pos.z-(z_dist*(float)i), pos.x- (x_dist*(float)i), fixed_height, stretch_width, true);
						}
						//p_Level->UpdateGeneratedTerrainPinch(1, 0.0f, pos.z, pos.x, fixed_height, stretch_width, true);
						p_Level->BakePinch();
						SetStaticsHeightToTerrrain();
					}

				}
				else
				{
					if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::Z) == true)
					{
						if (pos.x != last_pos.x || pos.z != last_pos.z || bColKeyDown == false)
						{
							bColKeyDown = true;
							p_Level->UpdateGeneratedTerrainTex(pos.z, pos.x, stretch_width, 0);

						}
					}
					else
					{
						if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::X) == true)
						{
							if (pos.x != last_pos.x || pos.z != last_pos.z || bColKeyDown == false)
							{
								bColKeyDown = true;
								p_Level->UpdateGeneratedTerrainTex(pos.z, pos.x, stretch_width, 1);

							}
						}
						else
						{
							bColKeyDown = false;
						}
					}
					fixed_height = t_height;
				}
			}


			last_pos = pos;
			stretch_height = 0.0f;
			bEditing = false;
		}
		else
		{
			if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::O) == true)
			{
				stretch_height += 0.1f;
			}

			if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::L) == true)
			{
				stretch_height -= 0.1f;
			}



			p_Level->UpdateGeneratedTerrainPinch(1, 0.0f, pos.z, pos.x, stretch_height, stretch_width, false);
			bEditing = true;
		}
		render_point_x = hit_pos.x;
		render_point_y = hit_pos.y + pointer_above;
		render_point_z = hit_pos.z;
	}



	if (current_editing == 2)
	{
		render_point_x = hit_pos.x;
		render_point_y = hit_pos.y + pointer_above;
		render_point_z = hit_pos.z;
		if (m_Resources->m_PadInput->GetPointerPressed() == true)
		{
			if (bEditing == false && bMouseClicked == false)
			{
				bMouseClicked = true;
				bEditing = true;
				PathPoint ppoint;
				ppoint.pos.x = render_point_x;
				ppoint.pos.y = render_point_y;
				ppoint.pos.z = render_point_z;
				p_Level->m_Paths->AddPoint(0, &ppoint);
			}
		}
		else
		{
			bMouseClicked = false;
			bEditing = false;
		}

	}

	if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::I) == true)
	{
		stretch_width += 0.1f;
	}

	if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::K) == true)
	{
		stretch_width -= 0.1f;
	}


	if (current_editing == -1)
	{
		//m_Marker->SetPosition(hit_pos.x, hit_pos.y + pointer_above, hit_pos.z);
		if (m_Resources->m_PadInput->GetPointerPressed() == true)
		{
			p_Statics->Clear(hit_pos.x, hit_pos.z);
			p_Stuff->Clear(hit_pos.x, hit_pos.z);
		}
	}

	if (current_editing > 99 && current_editing < 399)
	{


		if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::D) == true)
		{
			pointer_above += 0.1f;
		}



		if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::C) == true)
		{
			pointer_above -= 0.1f;
		}


	}


	if (current_editing >99 && current_editing<199)
	{
		int item = current_editing - 100;
		//m_Marker->SetPosition(hit_pos.x, hit_pos.y + pointer_above, hit_pos.z, pointer_rot_x, pointer_rot_y,0.0f, pointer_scale);

		if (m_Resources->m_PadInput->GetPointerPressed() == true)
		{
			p_Statics->CreateOne(hit_pos.x, hit_pos.y + pointer_above, hit_pos.z, pointer_above,1.0f, pointer_rot_x,pointer_rot_y, pointer_rot_z);
		}

		render_point_x = hit_pos.x;
		render_point_y = hit_pos.y + pointer_above;
		render_point_z = hit_pos.z;

	}

	if (current_editing > 299 && current_editing < 399)
	{
		int item = current_editing - 300;
		//m_Marker->SetPosition(hit_pos.x, hit_pos.y + pointer_above + p_Stuff->m_stuff_model[p_Stuff->current_selected_index]->m_meshModels[0]->Extents().MaxY, hit_pos.z, pointer_rot_x, pointer_rot_y, 0.0f, pointer_scale);
		//m_Marker->SetPosition(hit_pos.x, hit_pos.y + pointer_above, hit_pos.z, pointer_rot_x, pointer_rot_y, 0.0f, pointer_scale);
		if (m_Resources->m_PadInput->GetPointerPressed() == true)
		{
			if ((last_place_x != hit_pos.x) && (last_place_z != hit_pos.z))//(bPointerPressed == false)
			{
				last_place_x = hit_pos.x;
				last_place_z = hit_pos.z;

				if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::R) == true)
				{
					p_Stuff->CreateOne(hit_pos.x, hit_pos.y + pointer_above + p_Stuff->m_stuff_model[p_Stuff->current_selected_index]->m_meshModels[0]->Extents().MaxY, hit_pos.z, pointer_above, 0.0f, pointer_rot_x, pointer_rot_y, pointer_rot_z,true);
				}
				else
				{
					p_Stuff->CreateOne(hit_pos.x, hit_pos.y + pointer_above + p_Stuff->m_stuff_model[p_Stuff->current_selected_index]->m_meshModels[0]->Extents().MaxY, hit_pos.z, pointer_above, 0.0f, pointer_rot_x, pointer_rot_y, pointer_rot_z);
				}
				bPointerPressed = true;
			}

			if(bPointerPressed == false)
			{
				last_place_x = 9999.0f;
				last_place_z = 9999.0f;
			}


		}
		else
		{
			bPointerPressed = false;

			render_point_x = hit_pos.x;
			render_point_y = hit_pos.y + pointer_above + p_Stuff->m_stuff_model[p_Stuff->current_selected_index]->m_meshModels[0]->Extents().MaxY;
			render_point_z = hit_pos.z;

		}
	}
}

void LevelEdit::SetEditing(int ed)
{
	current_editing = ed; 
	if (current_editing > 99 && current_editing < 199)
	{
		p_Statics->SetCurrentSelectedindex(current_editing - 100);
	}
	if (current_editing > 299 && current_editing < 399)
	{
		p_Stuff->SetCurrentSelectedindex(current_editing - 300);
	}
}

void LevelEdit::SetStaticsHeightToTerrrain()
{
	int i;

	for (i = 0; i < p_Statics->cur_total; i++)
	{
		if (p_Statics->m_static[i].bActive == 1)
		{
			p_Statics->m_static[i].y = p_Level->GetTerrainHeight(p_Statics->m_static[i].x, p_Statics->m_static[i].z);
			p_Statics->m_static[i].model_matrix._24 = p_Statics->m_static[i].y + p_Statics->m_static[i].height_from_terrain;// = 
			//p_Statics->m_static[i].model_matrix = m_Resources->MakeMatrix(p_Statics->m_static[i].x, p_Statics->m_static[i].y + p_Statics->m_static[i].height_from_terrain, p_Statics->m_static[i].z, p_Statics->m_static[i].rot_x, p_Statics->m_static[i].rot_y, p_Statics->m_static[i].rot_z, p_Statics->m_static[i].scale);
		}
	}

	for (i = 0; i < p_Stuff->cur_total; i++)
	{
		if (p_Stuff->m_stuff[i].bActive == 1)
		{
			p_Stuff->m_stuff[i].info.pos.y = p_Level->GetTerrainHeight(p_Stuff->m_stuff[i].info.pos.x, p_Stuff->m_stuff[i].info.pos.z) + p_Stuff->m_stuff[i].height_from_terrain + p_Stuff->m_stuff_model[p_Stuff->m_stuff[i].model_index]->m_meshModels[0]->Extents().MaxY;
			p_Stuff->m_stuff[i].model_matrix._24 = p_Stuff->m_stuff[i].info.pos.y;// +p_Stuff->m_stuff_model[p_Stuff->m_stuff[i].model_index]->model->box_extents.y;// = m_Resources->MakeMatrix(p_Statics->m_static[i].x, p_Statics->m_static[i].y, p_Statics->m_static[i].z, p_Statics->m_static[i].rot_x, p_Statics->m_static[i].rot_y, p_Statics->m_static[i].rot_z, p_Statics->m_static[i].scale);
		}
	}
}



void LevelEdit::ClearPaths()
{
	bClearPaths = true;
}
void LevelEdit::NewPath()
{
	bNewPath = true;
}

void LevelEdit::MakePath()
{
	// XMLoadFloat3(&XMFLOAT3((float)j*2.0f - left_right_walls + 1.0f, height_map[j][i], (float)i*2.0f - front_back_walls + 1.0f)));
	bMakePath = true;

}

void LevelEdit::Render()
{
	int i;

	if (m_Resources->bDevelopment == false)
		return;

	
	if (current_editing == 1 || current_editing == -1)
	{
		//m_Resources->m_Models->SetRenderBuffers("marker");
		//m_Resources->SetConstantModelBuffer(m_Marker->GetObjectMatrix());
		//m_Resources->SetTexture("dice", 0);
		//m_Resources->m_Camera->UpdateConstantBuffer();
		//m_Resources->m_Models->DrawIndexed("marker");

		m_Resources->m_Camera->SetModelMatrix(&m_Resources->MakeMatrix(render_point_x, render_point_y, render_point_z, 0.0f, 0.0f, 0.0f, stretch_width));
		m_Resources->m_Camera->UpdateConstantBuffer();
		//m_Marker[0]->Render(*m_Resources);
		p_Statics->m_static_model[0]->m_meshModels[0]->Render(*m_Resources);
	}

	if (current_editing == 2)
	{
		//m_Resources->m_Models->SetRenderBuffers("marker");
		//m_Resources->SetConstantModelBuffer(m_Marker->GetObjectMatrix());
		//m_Resources->SetTexture("dice", 0);
		//m_Resources->m_Camera->UpdateConstantBuffer();
		//m_Resources->m_Models->DrawIndexed("marker");

		m_Resources->m_Camera->SetModelMatrix(&m_Resources->MakeMatrix(render_point_x, render_point_y, render_point_z, 0.0f, 0.0f, 0.0f, stretch_width));
		m_Resources->m_Camera->UpdateConstantBuffer();
		//m_Marker[0]->Render(*m_Resources);
		p_Statics->m_static_model[0]->m_meshModels[0]->Render(*m_Resources);

		for (i = 0; i < p_Level->m_Paths->GetTotalPoints(0); i++)
		{
			m_Resources->m_Camera->SetModelMatrix(&m_Resources->MakeMatrix(p_Level->m_Paths->GetPoints(0).at(i).pos.x, p_Level->m_Paths->GetPoints(0).at(i).pos.y, p_Level->m_Paths->GetPoints(0).at(i).pos.z, 0.0f, 0.0f, 0.0f, 1.0f));
			m_Resources->m_Camera->UpdateConstantBuffer();
			//m_Marker[0]->Render(*m_Resources);
			p_Statics->m_static_model[0]->m_meshModels[0]->Render(*m_Resources);
		}
	}


	if (current_editing > 99 && current_editing < 199)
	{
		
		//m_Resources->SetConstantModelBuffer(m_Marker->GetObjectMatrix());
		m_Resources->m_Camera->SetModelMatrix(&m_Resources->MakeMatrix(render_point_x, render_point_y, render_point_z, 
			pointer_rot_x, pointer_rot_y, pointer_rot_z, 1.0f));

		m_Resources->m_Camera->UpdateConstantBuffer();
		p_Statics->RenderPointer();

	}

	if (current_editing > 299 && current_editing < 399)
	{
		m_Resources->m_Camera->SetModelMatrix(&m_Resources->MakeMatrix(render_point_x, render_point_y, render_point_z, pointer_rot_x, pointer_rot_y, pointer_rot_z, 1.0f));
		m_Resources->m_Camera->UpdateConstantBuffer();
		if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::R) == true)
		{
			p_Stuff->RenderPointer(&m_Resources->MakeMatrix(render_point_x, render_point_y, render_point_z, pointer_rot_x, pointer_rot_y, pointer_rot_z, 1.0f), true);
			//m_Marker[0]->Render(*m_Resources);
		}
		else
		{
			p_Stuff->RenderPointer(&m_Resources->MakeMatrix(render_point_x, render_point_y, render_point_z, pointer_rot_x, pointer_rot_y, pointer_rot_z, 1.0f), false);
			//m_Marker[0]->Render(*m_Resources);
		}

	}

	
}


