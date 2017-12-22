#include "LoginServer.h"
#include <log.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/buffer.h>
#include <json/json.h>
LoginServer gServer;
static void LoginUpdate(float time,void *arg)
{

}
LoginServer::LoginServer()
{
}

LoginServer::~LoginServer()
{
}

bool LoginServer::Init()
{
	if (!BaseServer::Init())
	{
		return false;
	}
	CreateHttpServer(9300, 10240);


	return true;
}

int LoginServer::Run()
{
	if (Init())
	{
		m_UpdateTimer.Init(0, LoginUpdate, this, true);
		m_UpdateTimer.Begin();
		int ret = BaseServer::Run();
		log_debug("server end result %d", ret);
		return ret;
	}
	return -1;
}

void LoginServer::OnGet(evhttp_request * req, const char * path, const char * query)
{
	NS_MAP::unordered_map<std::string, std::string> result;
	HttpListenner::PasreQuery(query, result);

	int ret_code=0;
	if (strcmp(path, "/verify_id") == 0)
	{
		
	}
	Json::Value root;
	root["ret"] = ret_code;
	root["msg"] = "hello world";

	HttpListenner::WriteData(req, root.toStyledString().c_str());
	HttpListenner::EndWrite(req, HTTP_OK, "ok");
}

void LoginServer::OnPost(evhttp_request * req, const char * path, const char * query, evbuffer * buffer)
{
}

bool LoginServer::VerifyAccount(const char * id)
{
	return false;
}


