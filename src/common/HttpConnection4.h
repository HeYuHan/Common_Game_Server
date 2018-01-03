#pragma once
#include<string>
#include <event2/http.h>
#ifndef __HTTP_CONNECTION4_H__
#define __HTTP_CONNECTION4_H__
class UriParser
{
public:
	UriParser();
	~UriParser();
	bool Parse(const char* url);
	char* Decode(const char* url);
private:
	void Clean();
	
public:
	int port;
	char host[64];
	char* m_DecodeData;

};

int HttpGet(const char* url, std::string &ret);

int HttpPost(const char * url, const char* data, std::string & content);
#endif
