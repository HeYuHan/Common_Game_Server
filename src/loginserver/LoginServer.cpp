#include "LoginServer.h"
#include <log.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/buffer.h>
#include <FileReader.h>
#include <tools.h>
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
	
	Json::Value root;
	bool ret = ReadJson(root, m_Config.data_path);
	if (!ret)
	{
		console_error("read data error path %s", m_Config.data_path);
		return false;
	}
	Json::Value server_config = root["Config"];
	if (!server_config.isNull())
	{
		//log
		ParseJsonValue(server_config, "m_LogName", gLogger.logName, 64);
		ParseJsonValue(server_config, "m_LogPath", gLogger.fileName, 128);
		ParseJsonValue(server_config, "m_LogToConsole", gLogger.m_LogToConsole);
		ParseJsonValue(server_config, "m_LogToFile", gLogger.m_LogToFile);


		ParseJsonValue(server_config, "m_VerifyAccountUrl", m_Config.verify_account_url, 128);
		ParseJsonValue(server_config, "m_ThreadCount", m_Config.thread_count);
		
	}
	m_AccountData = root["m_AccountData"];
	return strlen(m_Config.verify_account_url)!=0 || !m_AccountData.isNull();
}

int LoginServer::Run()
{
	if (Init())
	{
		m_UpdateTimer.Init(0, LoginUpdate, this, true);
		m_UpdateTimer.Begin();
		int ret = CreateHttpServer(m_Config.ip,m_Config.port, 10240, m_Config.thread_count);
		if (ret)
		{
			log_debug("login server run in ip %s port %d", m_Config.ip,m_Config.port);
			ret = BaseServer::Run();
		}
		log_debug("server end result %d", ret);
		return ret;
	}
	return -1;
}

void LoginServer::OnGet(evhttp_request * req, const char * path, const char * query)
{
	NS_MAP::unordered_map<std::string, std::string> result;
	HttpListenner::PasreQuery(query, result);
	Json::Value root;

	if (strcmp(path, "/verify_id") == 0)
	{
		VerifyAccount(result["id"].c_str(), root);
	}
	

	HttpListenner::WriteData(req, root.toStyledString().c_str());
	HttpListenner::EndWrite(req, HTTP_OK, "ok");
}

void LoginServer::OnPost(evhttp_request * req, const char * path, const char * query, evbuffer * buffer)
{
}

bool LoginServer::VerifyAccount(const char * id, Json::Value &ret)
{
	if (NULL == id)
	{
		goto ERR;
	}
	if (strlen(m_Config.verify_account_url) != 0)
	{
		ret["result"] = ERROR_VERIFY_BY_OTHER_URL;
		ret["msg"] = m_Config.verify_account_url;
	}
	else if(!m_AccountData.isNull())
	{
		if (m_AccountData[id].isNull())
		{
			ret["result"] = ERROR_ID_NOT_FOUND;
			return false;
		}
		else
		{
			ret["result"] = ERROR_NONE;
			ret["msg"] = m_AccountData[id];
			return true;
		}
	}


	ERR:
	ret["result"] = ERROR_ID_IS_NULL;
	return false;
}


