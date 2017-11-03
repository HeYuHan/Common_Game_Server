#include "Room.h"
#include "ChannelServer.h"
#include <common.h>
static void RoomUpdate(float time, void* arg)
{
	ChannelRoom *room = (ChannelRoom*)arg;
	room->Update(time);
}
ChannelRoom::ChannelRoom():
	uid(0),
	m_State(ROOM_STATE_IDLE),
	m_MaxClient(10)
{
}

ChannelRoom::~ChannelRoom()
{
}

void ChannelRoom::Init()
{
	m_ClientList.clear();
	m_UpdateTimer.Init(gChannelServer.GetEventBase(), 0.02f, RoomUpdate, this,true);
	m_UpdateTimer.Begin();
}

void ChannelRoom::Clean()
{
	m_UpdateTimer.Stop();
}

void ChannelRoom::Update(float time)
{
}

bool ChannelRoom::IsFull()
{
	return m_MaxClient == m_ClientList.size();
}

void ChannelRoom::StartGame()
{
}

