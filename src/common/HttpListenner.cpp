
#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/buffer.h>
#include "tools.h"
#include "HttpListenner.h"
#include "log.h"
static void OnClientRequest(struct evhttp_request *req, void *arg)
{
	HttpTask* task = static_cast<HttpTask*>(arg);
	if (task && task->listenner)
	{
		task->request = req;
		task->listenner->OnRequest(task);
	}
}
static int BindSocket(const char* addr_str, int count)
{
	int r;
	int nfd;
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	if (!ParseSockAddr(addr, addr_str, false))return -1;
	nfd = socket(AF_INET, SOCK_STREAM, 0);
	if (nfd < 0) return -1;
	r = evutil_make_listen_socket_reuseable(nfd);

	r = bind(nfd, (struct sockaddr*)&addr, sizeof(addr));
	if (r < 0) return -1;
	r = listen(nfd, count);
	if (r < 0) return -1;
	if (evutil_make_socket_nonblocking(nfd) < 0)
	{
		evutil_closesocket(nfd);
		nfd = -1;
	}
	return nfd;
}
HttpListenner::HttpListenner()
{
}

HttpListenner::~HttpListenner()
{
	m_TaskPool.Terminate();
	m_ThreadPool.Stop();
}

bool HttpListenner::CreateHttpServer(const char* addr, int listen_count,int thread_count)
{
	if (!m_TaskPool.Initialize(thread_count))return false;
	m_Socket = BindSocket(addr, listen_count);
	if (m_Socket < 0)return false;
	if (!m_ThreadPool.Start(thread_count, thread_count))return false;
	for (int i = 0; i < thread_count; i++)
	{
		HttpTask *t = m_TaskPool.Allocate();
		t->listenner = this;
		m_ThreadPool.AddTask(t);
	}
	return true;
}

void HttpListenner::StopServer()
{
	if (m_Socket > 0)
	{
		evutil_closesocket(m_Socket);
		m_Socket = 0;
	}
	m_ThreadPool.Stop();
}

void HttpListenner::OnRequest(HttpTask *task)
{
	evhttp_request* req = task->request;
	evhttp_cmd_type type = evhttp_request_get_command(req);
	const struct evhttp_uri* uri = evhttp_request_get_evhttp_uri(req);
	const char* path = evhttp_uri_get_path(uri);
	const char* query = evhttp_uri_get_query(uri);
	
	if (type == EVHTTP_REQ_GET)
	{
		OnGet(task, path, query);
	}
	else if (type == EVHTTP_REQ_POST)
	{
		OnPost(task, path, query, evhttp_request_get_input_buffer(req));
	}
	else
	{
		evhttp_send_error(req, HTTP_BADREQUEST, "request type error");
	}
	
}

int HttpListenner::ReadData(evhttp_request * req, char * input, int size)
{
	struct evbuffer* buffer = evhttp_request_get_input_buffer(req);
	if (NULL == buffer)return 0;
	int buff_len = evbuffer_get_length(buffer);
	if (buff_len > 0)
	{
		return evbuffer_remove(buffer, input, MIN(buff_len, size));
	}
	return 0;
}

int HttpListenner::WriteData(evhttp_request * req, const char * output)
{
	struct evbuffer *buffer = evhttp_request_get_output_buffer(req);
	if (NULL == buffer)return 0;
	return evbuffer_add(buffer, output, strlen(output));
}

void HttpListenner::EndWrite(struct evhttp_request *req, int code, const char* reason)
{
	evhttp_send_reply(req, code, reason, evhttp_request_get_output_buffer(req));
}

void HttpListenner::PasreQuery(const char * query, NS_MAP::unordered_map<std::string, std::string>& result)
{
	if (NULL == query)return;
	std::string str = query;
	
	int index = 0;
	while ((index = str.find('&', 0)) > 0)
	{
		
		std::string key = str.substr(0,index);
		int index2 = str.find('=', 0);
		if (index2 > 0)
		{
			result[key.substr(0, index2)] = key.substr(index2+1);
		}
		str = str.substr(index+1);
	}
	index = str.find('=', 0);
	if (index > 0)
	{
		result[str.substr(0, index)] = str.substr(index + 1);
	}
	
}


HttpTask::HttpTask():
	evbase(NULL),
	request(NULL),
	listenner(NULL)
{
}

HttpTask::~HttpTask()
{
}

void HttpTask::Process()
{
	if (NULL == listenner)return;
	evbase = event_base_new();
	if (evbase == NULL)
	{
		return;
	}
	struct evhttp *httpd = evhttp_new(evbase);
	if (httpd == NULL)
	{
		return;
	}
	int ret = evhttp_accept_socket(httpd, listenner->m_Socket);
	if (ret != 0)return;
	evhttp_set_gencb(httpd, OnClientRequest, this);
	event_base_dispatch(evbase);
	evhttp_free(httpd);
	event_base_free(evbase);
	evbase = NULL;
}
