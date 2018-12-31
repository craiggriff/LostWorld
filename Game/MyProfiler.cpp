#include "pch.h"
#include "MyProfiler.h"

using namespace Game;
using namespace DirectX;


MyProfiler::MyProfiler(AllResources* p_Resources)
{

	m_Resources = p_Resources;

}


MyProfiler::~MyProfiler()
{

}


void MyProfiler::Start()
{
	tick_seconds = m_Resources->p_Timer->GetElapsedTicks();
}


void MyProfiler::Stop()
{
	tick_seconds = m_Resources->p_Timer->GetElapsedTicks() -tick_seconds;

	
}

void MyProfiler::Display()
{
	m_Resources->m_uiControl->SetTimer(tick_seconds*10000.0f);
	m_Resources->m_uiControl->SetFPS(m_Resources->p_Timer->GetFramesPerSecond());
}