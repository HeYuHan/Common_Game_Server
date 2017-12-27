#pragma once
#ifndef __LOGIN_SERVER_H__
#define __LOGIN_SERVER_H__
#include <BaseServer.h>
#include <HttpListenner.h>
#include <Timer.h>
#include <json/json.h>
#include <vector>
#include <HttpConnection3.h>

struct Config
{
	char addr[128];
	char data_path[128];
	int thread_count;
	char verify_account_url[128];
};
typedef enum
{
	ERROR_NONE = 0,
	ERROR_VERIFY_OFF,
	ERROR_VERIFY_BY_OTHER_URL,
	ERROR_ID_NOT_FOUND=1001,
	ERROR_ID_IS_NULL,
	ERROR_NO_SERVER,
	ERROR_TIME_OUT,
	
}LoginResult;
class LoginServer:public BaseServer,public HttpListenner
{
public:
	LoginServer();
	~LoginServer();

public:
	virtual bool Init();
	virtual int Run();
	virtual void OnGet(HttpTask *task, const char* path, const char* query);
	virtual void OnPost(HttpTask *task, const char* path, const char* query, struct evbuffer *buffer);
	void Response(HttpTask* task, Json::Value &ret);
private:
	void VerifyAccount(HttpTask *task, const char *id);
	void GetServerList(HttpTask *task);
	
private:
	Timer m_UpdateTimer;
	bool m_AccountVerifyOn;
public:
	Config m_Config;
	NS_MAP::unordered_map<std::string, std::string> m_AccountData;
	NS_VECTOR::vector<std::string> m_ServerList;
};
extern LoginServer gServer;


#endif // !__LOGIN_SERVER_H__
