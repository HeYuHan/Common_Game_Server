#pragma once
#ifndef __CHANNEL_ROOM_H__

class ChannelRoom
{
public:
	ChannelRoom();
	~ChannelRoom();
	void Update(float time);
public:
	unsigned int uid;
};


#define __CHANNEL_ROOM_H__
#endif