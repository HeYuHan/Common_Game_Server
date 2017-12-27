#include "HttpConnection3.h"
#include <event2/event.h> 
#include <event2/http_struct.h>
#include <event2/buffer.h>
#include <string.h>
#include "Timer.h"
#include "log.h"
HttpManager gHttpManager;
void http_request_done(struct evhttp_request *req, void *arg)
{
	/*fprintf(stderr, "< HTTP/1.1 %d\n", evhttp_request_get_response_code(req));
	struct evkeyvalq* headers = evhttp_request_get_input_headers(req);
	struct evkeyval* header = headers->tqh_first;

	while (header)
	{
		fprintf(stderr, "< %s: %s\n", header->key, header->value);
		auto n = header->next;
		header = n.tqe_next;
	}

	fprintf(stderr, "< \n");
	printf("send request ok...\n");
	size_t len = evbuffer_get_length(req->input_buffer);
	unsigned char * str = evbuffer_pullup(req->input_buffer, len);
	char buf[102400] = { 0 };
	memcpy(buf, str, len);
	if (str == NULL)
	{
		printf("len = %d, str == NULL\n", len);
	}
	else
	{
		printf("len = %d, str = %s\n", len, buf);
	}*/
	struct evhttp_connection* connection = NULL;
	IHttpInterface * http = static_cast<IHttpInterface*>(arg);
	if (NULL != http)
	{
		gHttpManager.SetInterface(http);
		http->OnResponse();
		gHttpManager.CleanInterface(http);
	}
	else
	{
		connection = static_cast<struct evhttp_connection*>(arg);
		if (NULL != connection)
		{
			evhttp_connection_free(connection);
		}
		evhttp_request_free(req);
	}
}




bool HttpManager::Request(const char * url, char * data, int port, int flag, IHttpInterface * user_data)
{
	struct evhttp_uri *uri = evhttp_uri_parse(url);
	const char* host = evhttp_uri_get_host(uri);
	int url_port = evhttp_uri_get_port(uri);
	if (url_port > 0)port = url_port;
	struct event_base* base = (NULL == user_data) ? Timer::GetEventBase() : user_data->GetEventBase();
	struct evhttp_connection* connection = evhttp_connection_base_new(base, NULL, evhttp_uri_get_host(uri), port);
	struct evhttp_request* req = evhttp_request_new(http_request_done, (user_data == NULL) ? connection : (void*)user_data);
	evhttp_add_header(req->output_headers, "Host", host);
	evhttp_connection_set_timeout(connection, 10000);
	if (NULL != user_data)
	{
		user_data->connection = connection;
		user_data->request = req;
	}
	char query_path[512] = { 0 };
	int index = 0;
	const char* path = evhttp_uri_get_path(uri);
	if (strlen(path) == 0) {
		strcpy(&query_path[index++], "/");
	}
	else
	{
		strcpy(&query_path[index], path);
		index += strlen(path);
	}
	const char* query = evhttp_uri_get_query(uri);
	if (NULL != query)
	{
		sprintf(&query_path[index], "?%s", query);
	}
	if (flag == EVHTTP_REQ_POST)
	{
		evbuffer_add(req->output_buffer, data, strlen(data));
	}
	int ret = evhttp_make_request(connection, req, (evhttp_cmd_type)flag, query_path);
	evhttp_uri_free(uri);
	return ret == 0;
}

bool HttpManager::Get(const char * url, int port, IHttpInterface * user_data)
{
	return Request(url, NULL, port, EVHTTP_REQ_GET, user_data);
}

bool HttpManager::Get(const char * url, IHttpInterface * user_data)
{
	return Get(url, 80, user_data);
}

bool HttpManager::Post(const char * url, char * data, int port, IHttpInterface * user_data)
{
	return Request(url, NULL, port, EVHTTP_REQ_POST, user_data);
}

bool HttpManager::Post(const char * url, char * data, IHttpInterface * user_data)
{
	return Post(url, data, 80, user_data);
}

void HttpManager::CleanInterface(IHttpInterface * http)
{
	http->state = 0;
	if(http->connection)evhttp_connection_free(http->connection);
	if(http->request)evhttp_request_free(http->request);
	http->connection = NULL;
	http->request = NULL;
}

void HttpManager::SetInterface(IHttpInterface * http)
{
	http->state = evhttp_request_get_response_code(http->request);
}

IHttpInterface::IHttpInterface():connection(NULL),request(NULL),state(0)
{

}

int IHttpInterface::GetBufferLength()
{
	if (NULL != request && state>0)
	{
		return evbuffer_get_length(request->input_buffer);
	}
	return 0;
}

int IHttpInterface::ReadBuffer(void * data, int size)
{
	if (request != NULL)
	{
		return evbuffer_remove(request->input_buffer, data, size);
	}
	return 0;
}

int IHttpInterface::GetState()
{
	return state;
}

event_base * IHttpInterface::GetEventBase()
{

	return Timer::GetEventBase();
}
