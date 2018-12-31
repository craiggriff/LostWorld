
#include "pch.h"


#include "GameLogic.h"
#include "BasicLoader.h"
//#include "cpprest/http_client.h"
//#include "cpprest/filestream.h
//#include "cpprest/json.h"

using namespace Game;
using namespace DirectX;



//using namespace utility;
//using namespace web::http;
//using namespace web::http::client;
///using namespace concurrency;
//using namespace web::json;
//using namespace std;
//using namespace web;
//using namespace utility;
//using namespace http;
//using namespace http::client;

/*
// Creates an HTTP request and prints the length of the response stream.
concurrency::task<void> GameLogic::HTTPStreamingAsync()
{
		return create_task([this]{
		try
		{
			char url[200] = { "http://localhost:34568/blackjack/dealer" };

			btVector3* pos = m_Scene->m_Bert->GetPosition();
			utility::ostringstream_t b_url;
			b_url << url;
			
			http_client client(b_url.str());
			// Make the request and asynchronously process the response. 
			frame_pos++;

			//for (int i = 0; i < 1000; i++)
			//{
				utility::ostringstream_t buf;
				buf << "";

				buf << "?posx=";
				buf << pos->getX();
				buf << "?posy=";
				buf << pos->getY();
				buf << "?posz=";
				buf << pos->getZ();
				buf << "?frame=";
				buf << frame_pos;
				client.request(methods::GET, buf.str()).then([this](http_response response)
				{

					if (response.status_code() != status_codes::OK)
					{
						btVector3* pos = m_Scene->m_Bert->GetPosition();
						// Handle error cases... 
						//return pplx::task_from_result();
					}
					else
					{
						auto bodyStream = response.body();

						auto stat = response.status_code();
					}
					bStreamComplete = true;

				}).wait();
			//}
		}
		catch (const exception& e)
		{
			bStreamComplete = true;
			// wcout << L"Caught exception." << endl;
		}
	});
		
}
*/

GameLogic::GameLogic(AllResources* p_Resources, GameScene* pp_GameScene):
	m_Scene(pp_GameScene), m_Resources(p_Resources)
{
	current_player_state = 1;
	SetNewPlayerState(current_player_state);
	 
	bChangePressed = false;
	bInCarPressed = false;

	Windows::Storage::StorageFolder^ folder = Windows::Storage::ApplicationData::Current->LocalFolder;

	Platform::String^ fname = L"sample.txt";

	frame_pos = 0;


	//player_y_view = 0.0f;
	//bStreamComplete = false;
	//HTTPStreamingAsync();

	//}
	//catch(const exception& e)
	//{
		
	//	wcout << L"Caught exception." << endl;
	//}
	


	//HTTPStreamingAsync();


	// In C++ and CX, the system resources used by httpClient object are released 
	// when the object falls out of scope or by the destructor (delete operator)


	// Once your app is done using the HttpClient object call close to 
	// free up system resources (the underlying socket and memory used for the object)
	// httpClient.close();


	//Windows::Foundation::IAsyncOperation::StorageFile^ outputFileName = folder->CreateFileAsync(fname,
	//	Windows::Storage::CreationCollisionOption::ReplaceExisting).done();
	
	//http_client client(U("http://www.google.co.uk/"));
	//const http_response resp = client.request(methods::GET, uri_builder(U("/search")).append_query(U("q"), "cars").to_string()).wait();
	//resp.get()

	//const string_t searchTerm = L"cars";
	//Windows::Storage::StorageFile^ outputFileName = new Windows::Storage::StorageFolder("cool.txt");

	/*
	// Open a stream to the file to write the HTTP response body into.
	auto fileBuffer = std::make_shared<concurrency::streams::streambuf<uint8_t>>();
	concurrency::streams::file_buffer<uint8_t>::open(outputFileName, std::ios::out).then([=](concurrency::streams::streambuf<uint8_t> outFile) -> pplx::task<http_response>
	{
		*fileBuffer = outFile;

		// Create an HTTP request.
		// Encode the URI query since it could contain special characters like spaces.
		http_client client(U("http://www.bing.com/"));
		return client.request(methods::GET, uri_builder(U("/search")).append_query(U("q"), searchTerm).to_string());
	})

		// Write the response body into the file buffer.
		.then([=](http_response response) -> pplx::task<size_t>
	{
		printf("Response status code %u returned.\n", response.status_code());

		return response.body().read_to_end(*fileBuffer);
	})

		// Close the file buffer.
		.then([=](size_t)
	{
		return fileBuffer->close();
	})

		// Wait for the entire response body to be written into the file.
		.wait();
		*/
	/*
	http_client client(U("http://www.bing.com/"));
	concurrency::task::http::http_response resp = client.request(methods::GET, uri_builder(U("/search")).append_query(U("q"), searchTerm).to_string()).then([=](http_response response)
	{

	}
	*/
}


void GameLogic::Update(DX::StepTimer const& timer)
{
	//ucout << "Black Jack"; 
	
	if (bStreamComplete == true)
	{
		//bStreamComplete = false;
		//HTTPStreamingAsync();

		
	}
	

	if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::C) == true)
	{
		if(bInCarPressed==false)
		{
			bInCarPressed = true;
			if (m_Scene->m_Buggy->bInCarView == true)
				m_Scene->m_Buggy->bInCarView = false;
			else
				m_Scene->m_Buggy->bInCarView = true;
		}
	}
	else
	{
		bInCarPressed = false;
	}

	if (m_Resources->m_PadInput->KeyState(Windows::System::VirtualKey::P) == true || m_Resources->m_PadInput->ButtonBwas()==true)
	{
		if (bChangePressed == false)
		{
			bChangePressed = true;
			if (current_player_state == 0)
			{
				SetNewPlayerState(1);
			}
			else
			{
				SetNewPlayerState(0);
			}
		}
	}
	else
	{
		bChangePressed = false;
	}
	//Deal with all physics collisions
	//PhysicsCollisions();
}

void GameLogic::PhysicsCollisions()
{

	int numManifolds = m_Resources->m_Physics.m_dynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; i++)
	{
		btPersistentManifold* contactManifold = m_Resources->m_Physics.m_dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = (btCollisionObject*)(contactManifold->getBody0());
		btCollisionObject* obB = (btCollisionObject*)(contactManifold->getBody1());
		//obA->
		//contactManifold->
		int numContacts = contactManifold->getNumContacts();
		for (int j = 0; j < numContacts; j++)
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if (pt.getDistance() < 0.f)
			{
				for (int k = 0; k < 4; k++)
				{
					if (obA == m_Scene->m_Buggy->wheel[k]->m_rigidbody)
					{
						btTransform wheelTran;
						m_Scene->m_Buggy->wheel[k]->getWorldTransform(wheelTran);

						btVector3 ptA = pt.getPositionWorldOnA();
						btVector3 ptB = pt.getPositionWorldOnB();
						btVector3 normalOnB = pt.m_normalWorldOnB;

						if (pt.m_appliedImpulse > 0.00001f)
						{
							float num_create = wheel_last_contact[k].distance(ptA)*8.0f;
							if (num_create > 10.0f)
							{
								num_create = 10.0f;
							}
							num_create *= 2.0f;
							float step_create = 1.0f / num_create;
							for (int m=0; m < (int)num_create; m++)
							{
								btVector3 ac_pos = lerp(wheel_last_contact[k], ptA,(float)m*step_create);
								float randx = (((rand() % 10)*0.03f) - 0.05f);
								float randy = (((rand() % 10)*0.03f) );
								float randz = (((rand() % 10)*0.03f) - 0.05f);
								m_Scene->m_steam->CreateOne(ac_pos.getX()+ randx, ac_pos.getY()+ randy, ac_pos.getZ()+ randz, pt.m_lateralFrictionDir1.getX(), pt.m_lateralFrictionDir1.getY(), pt.m_lateralFrictionDir1.getZ(), XMFLOAT4(m_Resources->m_LevelInfo.dust_col.x, m_Resources->m_LevelInfo.dust_col.y, m_Resources->m_LevelInfo.dust_col.z, 0.5f), 1.0f);
							}
						}
						wheel_last_contact[k] = ptA;
					}
				}

				for (int k = 0; k < m_Scene->m_Stuff->cur_phy; k++)
				{
					if (obA == m_Scene->m_Stuff->m_Motion[k]->m_rigidbody ||
						obB == m_Scene->m_Stuff->m_Motion[k]->m_rigidbody)
					{
						if (pt.m_appliedImpulse > 1.0f)
						{
							m_Resources->m_audio.PlaySoundEffect(SFXDiceClick);
						}
					}
				}

				//pt.
			}
		}
	}


	
	
	std::vector<concurrency::task<void>> tasks;

	int num_threads = m_Resources->m_Physics.m_dynamicsWorld->getNumCollisionObjects()/4;

	for(int z=0;z<4;z++)
	{
		tasks.push_back(concurrency::create_task([this, z,num_threads]
		{
		// Water boyancy
			for (int i = z * num_threads - 1; i >= 0; i--)
			{
				btCollisionObject* obj = m_Resources->m_Physics.m_dynamicsWorld->getCollisionObjectArray()[i];
				btRigidBody* body = btRigidBody::upcast(obj);
				const btBroadphaseProxy* proxy = body->getBroadphaseHandle();
				if (body->getActivationState()!=ISLAND_SLEEPING && body->getWorldTransform().getOrigin().getY() < m_Scene->m_Level->GetWaterHeight(body->getWorldTransform().getOrigin().getX(), body->getWorldTransform().getOrigin().getZ()) + 0.1f)
				{
					if (proxy->m_collisionFilterGroup == (COL_CARBODY | COL_RAY) ||
						proxy->m_collisionFilterGroup == (COL_WHEEL | COL_RAY))
					{
						float innert = 0.99f - body->getInvMass()*0.004f;
						float vx = body->getLinearVelocity().getX()*innert;
						float vy = (body->getLinearVelocity().getY() + ((body->getInvMass() - 0.5f)*0.04f))*innert;
						float vz = body->getLinearVelocity().getZ()*innert;

						body->setLinearVelocity(btVector3(vx, vy, vz));
						body->setAngularVelocity(body->getAngularVelocity()*0.99f);
					}
					else
					{
						float innert = 0.99f - body->getInvMass()*0.004f;
						float vx = body->getLinearVelocity().getX()*innert;
						float vy = (body->getLinearVelocity().getY() + (body->getInvMass()*0.04f))*innert;
						float vz = body->getLinearVelocity().getZ()*innert;

						body->setLinearVelocity(btVector3(vx, vy, vz));
						body->setAngularVelocity(body->getAngularVelocity()*0.99f);


						/*
					if (body->getActivationState() == ISLAND_SLEEPING)
						{
							btTransform tran = body->getWorldTransform();
							tran.getOrigin().setY(m_Scene->m_Level->GetWaterHeight(body->getWorldTransform().getOrigin().getX(), body->getWorldTransform().getOrigin().getZ()));

							body->setWorldTransform(tran);
						}
						*/
					}
				}
			}
		}));


	}

	auto joinTask = when_all(begin(tasks), end(tasks));

	joinTask.wait();

}



void GameLogic::SetNewPlayerState(int _state)
{
	switch (_state)
	{
	case 0:
			if (current_player_state != 0)
			{
				m_Scene->m_Bert->SetPosition(m_Scene->m_Buggy->player_x + 3.0f, m_Scene->m_Buggy->player_y, m_Scene->m_Buggy->player_z);
			}
			m_Scene->m_Bert->bPlayerActive = true;
			m_Scene->m_Buggy->bPlayerActive = false;
//			m_Scene->m_Buggy->SetWheelActivation(false);
			m_Resources->m_uiControl->CarControlsVisible(false);
			m_Resources->m_uiControl->PlayerControlsVisible(true);
			current_player_state = 0;
			break;
	case 1:
			m_Scene->m_Bert->bPlayerActive = false;
			m_Scene->m_Buggy->bPlayerActive = true;
	//		m_Scene->m_Buggy->SetWheelActivation(true);
			m_Resources->m_uiControl->CarControlsVisible(true);
			m_Resources->m_uiControl->PlayerControlsVisible(false);
			current_player_state = 1;
			break;
	}
}

void GameLogic::SetPlayerState()
{
	int new_state;
	if (current_player_state == 1)
	{
		new_state = 0;
	}
	else
	{
		new_state = 1;
	}
	SetNewPlayerState(new_state);

}

