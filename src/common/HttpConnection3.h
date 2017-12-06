#pragma once

#include <event2/http.h>

#ifndef __HTTP_CONNECTION_H__
#define __HTTP_CONNECTION_H__
class IHttpInterface
{
	friend class HttpManager;
public:
	IHttpInterface();
	virtual void OnResponse() = 0;
	int GetBufferLength();
	int ReadBuffer(void *data, int size);
	int GetState();
private:
	struct evhttp_connection* connection;
	struct evhttp_request* request;
	int state;
};
class HttpManager
{
public:
	bool Request(const char* url, char* data, int port, int flag,IHttpInterface *user_data);
	bool Get(const char* url, int port, IHttpInterface *user_data);
	bool Get(const char* url, IHttpInterface *user_data = NULL);
	bool Post(const char* url, char* data,int port, IHttpInterface *user_data);
	bool Post(const char* url, char* data, IHttpInterface *user_data = NULL);
	void CleanInterface(IHttpInterface *http);
	void SetInterface(IHttpInterface *http);

};
extern HttpManager gHttpManager;

#endif // !__HTTP_CONNECTION_H__
