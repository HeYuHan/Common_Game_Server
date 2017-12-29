#pragma once
#include<string>
#include <event2/http.h>
#ifndef __HTTP_CONNECTION4_H__
#define __HTTP_CONNECTION4_H__
class UriParser
{
public:
	UriParser();
	bool Parse(const char* url);
public:
	int port;
	char host[64];

};

int HttpGet(const char* url, std::string &ret);

int HttpPost(const char * url, const char* data, std::string & content);
#endif
