#pragma once
enum ClientMessage
{
	CM_MESSAGE_START = 0,
	CM_PLAYER_INFO,
	CM_REQUEST_JOIN_GAME,
	CM_ROOM_START_GAME,
	CM_READY_IN_GAME,
	CM_INGAME_MOVE_DATA,
	CM_INGAME_SHOOT,
        //Ìí¼ÓÐÂµÄÎäÆ÷
};
enum ServerMessage
{
	SM_MESSAGE_START = 0,
	SM_CAN_ENTER_GAME,
	SM_ROOM_INFO,
	SM_ROOM_ENTER,
	SM_ROOM_LEAVE,
	SM_GAME_LOGADING,
	SM_INGAME_JOIN,
	SM_INGAME_ROOM_INFO,
	SM_INGAME_GAME_TIME,
	SM_INGAME_BRITH,
	SM_INGAME_MOVE_DATA,
	SM_INGAME_SHOOT,
	SM_INGAME_STATE_CHANGE,
};
typedef enum 
{
	ERROR_NONE=0,
	ERROR_CREATE_ROOM,
}ChannelError;
struct PlayerSkillState
{
public:
	static const int None = 0;
	static const int Shield = 1 << 1;
};
