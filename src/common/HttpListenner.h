#pragma once
#ifndef __HTTP_LISTENNER_H__
#define __HTTP_LISTENNER_H__
#include "common.h"
#include "ThreadPool.h"
#include "objectpool.h"
#include <string.h>
#include <vector>
#ifdef LINUX
#include <tr1/unordered_map>
#else
#include <unordered_map>
#endif
class HttpListenner;
struct evhttp_request;
struct event_base;
class HttpTask :public ThreadTask
{
public:
	HttpTask();
	~HttpTask();
public:
	
	uint uid;
	struct event_base* evbase;
	struct evhttp_request* request;
	HttpListenner* listenner;
	virtual void Process();
};
class HttpListenner
{
friend class HttpTask;
public:
	HttpListenner();
	~HttpListenner();
public:
	bool CreateHttpServer(const char* addr,int listen_count,int thread_count);
	void StopServer();
	void OnRequest(HttpTask *task);
	virtual void OnGet(HttpTask *task,const char* path,const char* query)=0;
	virtual void OnPost(HttpTask *task, const char* path, const char* query, struct evbuffer *buffer)=0;
	static int ReadData(struct evhttp_request *req,char* input, int size);
	static int WriteData(struct evhttp_request *req, const char* output);
	static void EndWrite(struct evhttp_request *req,int code,const char* reason);
	static void PasreQuery(const char* query, NS_MAP::unordered_map<std::string, std::string> &result);
private:
	ObjectPool<HttpTask> m_TaskPool;
	ThreadPool m_ThreadPool;
	int m_Socket;
};
#endif // !__HTTP_LISTENNER_H__



