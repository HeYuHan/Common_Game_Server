#include "Room.h"

ChannelRoom::ChannelRoom():
	uid(0),
	m_State(ROOM_STATE_IDLE)
{
}

ChannelRoom::~ChannelRoom()
{
}

void ChannelRoom::Init()
{
}

void ChannelRoom::Clean()
{
}

void ChannelRoom::Update(float time)
{
}

bool ChannelRoom::IsFull()
{
	return m_ClientCount == 0;
}
