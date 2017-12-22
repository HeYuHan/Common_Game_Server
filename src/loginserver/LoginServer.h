#pragma once
#ifndef __LOGIN_SERVER_H__
#define __LOGIN_SERVER_H__
#include <BaseServer.h>
#include <HttpListenner.h>
#include <Timer.h>
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
	bool VerifyAccount(const char *id);
private:
	Timer m_UpdateTimer;
};
extern LoginServer gServer;


#endif // !__LOGIN_SERVER_H__
