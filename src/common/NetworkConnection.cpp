#include "NetworkConnection.h"
#include<string.h>
NetworkStream::NetworkStream(int send_buff_size, int read_buff_size)
{
	read_buff = new char[read_buff_size];
	write_buff = new char[send_buff_size];

	write_position = write_buff;
	write_buff_end = write_buff + send_buff_size;
	write_end = write_buff;

	read_offset = read_buff;
	read_end = read_buff;
	read_position = read_buff;
	read_buff_end = read_buff + send_buff_size;

}


NetworkStream::~NetworkStream()
{
	delete[] read_buff;
	delete[] write_buff;
	read_buff = NULL;
	write_buff = NULL;
}


void NetworkStream::OnRevcMessage()
{
	if (NULL == connection)return;
	try
	{
		int empty_size = read_buff_end - read_offset;
		int revc_size = connection->Read(read_offset, empty_size);
		if (revc_size <= 0)return;
		read_offset += revc_size;
		int size = read_offset - read_position;
		while (size > 4)
		{
			int data_len = 0;
			memcpy(&data_len, read_position, 4);
			if (size - 4 < data_len)break;
			read_position += 4;
			read_end = read_position + data_len;
			OnMessage();
			read_position = read_end;
			size = read_offset - read_position;
		}
		size = read_offset - read_position;
		if ((read_position - read_buff) > 0 && size > 0)
		{
			memcpy(read_buff, read_position, size);
		}
		read_offset = read_buff + size;
		read_position = read_end = read_buff;
	}
	catch (...)
	{
		if (connection)connection->DisConnect();
	}
}

void NetworkStream::Update()
{
	if (NULL != connection)
	{
		connection->Update();
	}
}

void NetworkStream::Reset()
{
	write_position = write_buff;
	write_end = write_buff;

	read_offset = read_buff;
	read_end = read_buff;
	read_position = read_buff;
}




//////////////////////////////////////////////////////////////
//write data
void NetworkStream::WriteByte(byte data)
{
	WriteData(&data, sizeof(byte));
}
void NetworkStream::WriteByte(char data)
{
	WriteData(&data, sizeof(char));
}
void NetworkStream::WriteShort(short data)
{
	WriteData(&data, sizeof(short));
}
void NetworkStream::WriteUShort(ushort data)
{
	WriteData(&data, sizeof(ushort));
}
void NetworkStream::WriteInt(int data)
{
	WriteData(&data, sizeof(int));
}
void NetworkStream::WriteUInt(uint data)
{
	WriteData(&data, sizeof(uint));
}
void NetworkStream::WriteFloat(float data)
{
	WriteData(&data, sizeof(float));
}
void NetworkStream::WriteLong(long data)
{
	WriteData(&data, sizeof(long));
}
void NetworkStream::WriteULong(ulong data)
{
	WriteData(&data, sizeof(ulong));
}
void NetworkStream::WriteString(const char* str)
{
	int len = strlen(str);
	WriteInt(len);
	WriteData(str, len);
}
void NetworkStream::WriteData(const void* data, int count)
{
	if (write_buff == NULL || count < 0 || write_end + count > write_buff_end)
	{
		throw WRITEERROR;
	}
	if (count > 0)
	{
		memcpy(write_end, data, count);
		write_end += count;
	}

}
void NetworkStream::BeginWrite()
{
	write_position = write_buff;
	if (connection->m_Type == UDP_SOCKET)
	{
		write_end = write_buff + 5;
	}
	else
	{
		write_end = write_buff + 4;
	}

}
void NetworkStream::EndWrite()
{
	int head_len = connection->m_Type == UDP_SOCKET ? 5 : 4;
	int data_len = write_end - write_position - head_len;
	if (connection->m_Type == UDP_SOCKET)
	{
		write_position[0] = 254;
		memcpy(write_position+1, &data_len, 4);
	}
	else
	{
		memcpy(write_position, &data_len, 4);
	}
	if (connection)connection->Send(write_position, data_len+head_len);
}
//////////////////////////////////////////////////////////////
//read data
void NetworkStream::ReadByte(byte &data)
{
	ReadData(&data, sizeof(byte));
}
void NetworkStream::ReadByte(char &data)
{
	ReadData(&data, sizeof(char));
}
void NetworkStream::ReadShort(short &data)
{
	ReadData(&data, sizeof(short));
}
void NetworkStream::ReadUShort(ushort &data)
{
	ReadData(&data, sizeof(ushort));
}
void NetworkStream::ReadInt(int &data)
{
	ReadData(&data, sizeof(int));
}
void NetworkStream::ReadUInt(uint &data)
{
	ReadData(&data, sizeof(uint));
}
void NetworkStream::ReadFloat(float &data)
{
	ReadData(&data, sizeof(float));
}
void NetworkStream::ReadLong(long &data)
{
	ReadData(&data, sizeof(long));
}
void NetworkStream::ReadULong(ulong &data)
{
	ReadData(&data, sizeof(ulong));
}
int NetworkStream::ReadString(char* str, int size)
{
	int len = 0;
	ReadInt(len);
	if (len < 0 || size<len)
	{
		throw READERROR;

	}
	str[len] = 0;
	ReadData(str, len);
	return len;
}
void NetworkStream::ReadData(void* data, int count)
{
	if (read_buff == NULL || count < 0 || read_position + count > read_end)
	{
		throw READERROR;
	}
	if (count > 0)
	{
		memcpy(data, read_position, count);
		read_position += count;
	}
}















NetworkConnection::NetworkConnection():
	stream(NULL)
{
}

NetworkConnection::~NetworkConnection()
{
	stream = NULL;
}

