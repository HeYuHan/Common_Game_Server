#include "LoginServer.h"
#include <log.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/buffer.h>
#include <FileReader.h>
#include <tools.h>
#include <HttpConnection4.h>
LoginServer gServer;

static void LoginUpdate(float time,void *arg)
{

}
LoginServer::LoginServer():
	m_AccountVerifyOn(true)
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
		log_lasterror();
		return false;
	}
	Json::Value server_config = root["Config"];
	if (!server_config.isNull())
	{
		//log
		//ParseJsonValue(server_config, "m_LogToConsole", gLogger.m_LogToConsole);
		//ParseJsonValue(server_config, "m_LogToFile", gLogger.m_LogToFile);

		ParseJsonValue(server_config, "m_AccountVerifyOn", m_AccountVerifyOn);
		ParseJsonValue(server_config, "m_VerifyAccountUrl", m_Config.verify_account_url, 128);
		ParseJsonValue(server_config, "m_ThreadCount", m_Config.thread_count);
		
	}
	Json::Value account = root["m_AccountData"];
	if (!account.isNull())
	{
		NS_VECTOR::vector<std::string> members = account.getMemberNames();
		for (std::string::size_type i = 0; i < members.size(); i++)
		{
			Json::FastWriter writer;
			m_AccountData[members[i]] = writer.write(account[members[i]]);
		}

	}
	Json::Value serverlist = root["m_ServerList"];
	if (!serverlist.isNull())
	{
		for (Json::ValueIterator it = serverlist.begin(); it != serverlist.end(); it++)
		{
			m_ServerList.push_back((*it).asString());
		}
	}
	return strlen(m_Config.verify_account_url)!=0 || !account.isNull();
}

int LoginServer::Run()
{
	if (Init())
	{
		m_UpdateTimer.Init(0, LoginUpdate, this, true);
		m_UpdateTimer.Begin();
		int ret = CreateHttpServer(m_Config.addr, 10240, m_Config.thread_count);
		if (ret)
		{
			log_debug("login server run in %s", m_Config.addr);
			ret = BaseServer::Run();
		}
		else
		{
			log_error("create login server error ret %d", ret);
		}
		StopServer();
		return ret;
	}
	return -1;
}

void LoginServer::OnGet(HttpTask *task, const char * path, const char * query)
{
	
	if (strcmp(path, "/verify_id") == 0)
	{
		VerifyAccount(task, query);
	}
	if (strcmp(path, "/server_list") == 0)
	{
		GetServerList(task);
	}
}

void LoginServer::OnPost(HttpTask *task, const char * path, const char * query, evbuffer * buffer)
{
}

void LoginServer::VerifyAccount(HttpTask *task, const char * query)
{
	NS_MAP::unordered_map<std::string, std::string> query_map;
	HttpListenner::PasreQuery(query, query_map);
	const char* id = query_map["id"].c_str();
	Json::Value ret;
	if (strlen(id)==0)
	{
		ret["result"] = ERROR_ID_IS_NULL;
		goto RES;
	}
	if (!m_AccountVerifyOn)
	{
		ret["result"] = ERROR_NONE;
		ret["msg"] = "server verify off";
	}
	else if (strlen(m_Config.verify_account_url) != 0)
	{
		ret["result"] = ERROR_VERIFY_BY_OTHER_URL;
		char url[512] = { 0 };
		sprintf(url, "%s?%s", m_Config.verify_account_url, query);
		std::string content;
		ret["state"] = HttpGet(url, content);
		ret["msg"] = content;
	}
	else
	{
		if (m_AccountData[id].empty())
		{
			ret["result"] = ERROR_ID_NOT_FOUND;
		}
		else
		{
			ret["result"] = ERROR_NONE;
			ret["msg"] = m_AccountData[id];
		}
	}
	RES:
	Response(task, ret);
}

void LoginServer::GetServerList(HttpTask *task)
{
	Json::Value ret;
	if (m_ServerList.empty())
	{
		ret["result"] = ERROR_NO_SERVER;
		return;
	}
	else
	{
		ret["result"] = ERROR_NONE;
		Json::Value list;
		for (int i = 0; i < m_ServerList.size(); i++)
		{
			list[i] = m_ServerList[i];
		}
		ret["msg"] = list;
	}
	Response(task, ret);
}

void LoginServer::Response(HttpTask * task, Json::Value &ret)
{
	Json::FastWriter writer;
	HttpListenner::WriteData(task->request, writer.write(ret).c_str());
	HttpListenner::EndWrite(task->request, HTTP_OK, "ok");
}


