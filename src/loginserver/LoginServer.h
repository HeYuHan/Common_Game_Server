#pragma once
#ifndef __LOGIN_SERVER_H__
#define __LOGIN_SERVER_H__
#include <BaseServer.h>
#include <HttpListenner.h>
#include <Timer.h>
#include <json/json.h>
struct Config
{
	char ip[128];
	int port;
	char data_path[128];
	int thread_count;
	char verify_account_url[128];
};
typedef enum
{
	ERROR_NONE = 0,
	ERROR_ID_NOT_FOUND=1001,
	ERROR_ID_IS_NULL,
	ERROR_VERIFY_BY_OTHER_URL
}LoginResult;
class LoginServer:public BaseServer,public HttpListenner
{
public:
	LoginServer();
	~LoginServer();

public:
	virtual bool Init();
	virtual int Run();
	virtual void OnGet(struct evhttp_request *req, const char* path, const char* query);
	virtual void OnPost(struct evhttp_request *req, const char* path, const char* query, struct evbuffer *buffer);
private:
	bool VerifyAccount(const char *id,Json::Value &ret);
private:
	Timer m_UpdateTimer;
public:
	Config m_Config;
	Json::Value m_AccountData;
};
extern LoginServer gServer;


#endif // !__LOGIN_SERVER_H__
