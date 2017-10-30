#pragma once
#ifndef __NETWORKCONNECTION_H__
#define __NETWORKCONNECTION_H__
#include "common.h"
class NetworkConnection;
typedef enum
{
	READERROR = 0, WRITEERROR = 1
}NETERR;
typedef enum
{
	NET_NONE=0,NET_CONNECTED,NET_DISCONNECTED
}NetState;
class NetworkStream
{
public:
	NetworkStream(int send_buff_size = 1024 * 512, int read_buff_size = 1024 * 512);
	~NetworkStream();
	virtual void OnMessage()=0;
	virtual void Update();
	void Reset();
	void OnRevcMessage();
public:
	void WirteByte(byte data);
	void WirteByte(char data);
	void WriteShort(short data);
	void WriteUShort(ushort data);
	void WriteInt(int data);
	void WriteUInt(uint data);
	void WriteFloat(float data);
	void WriteLong(long data);
	void WriteULong(ulong data);
	void WriteString(const char* str);
	void WriteData(const void* data, int count);
	void BeginWrite();
	void EndWrite();
	//////////////////////////////////////////////////////////////
	//read data
public:
	void ReadByte(byte &data);
	void ReadByte(char &data);
	void ReadShort(short &data);
	void ReadUShort(ushort &data);
	void ReadInt(int &data);
	void ReadUInt(uint &data);
	void ReadFloat(float &data);
	void ReadLong(long &data);
	void ReadULong(ulong &data);
	int ReadString(char* str, int size);
	void ReadData(void* data, int count);
public:
	NetworkConnection* connection;
protected:
	char* write_buff;
	char* write_position;
	char* write_end;
	char* write_buff_end;

	char* read_offset;
	char* read_buff;
	char* read_end;
	char* read_position;
	char* read_buff_end;

private:

};


class NetworkConnection
{
public:
	NetworkConnection();
	~NetworkConnection();
	virtual void Update() = 0;
	virtual int Read(void* data, int size) = 0;
	virtual int Send(void* data, int size) = 0;
	virtual void OnConnected() = 0;
	virtual void OnDisconnected() = 0;
	virtual void DisConnect()=0;
public:
	NetworkStream* stream;

};
#endif
