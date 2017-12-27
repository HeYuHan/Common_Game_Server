#pragma once
#include<string>
#ifndef __HTTP_CONNECTION4_H__
#define __HTTP_CONNECTION4_H__

int HttpGet(const char* url, std::string &ret);

int HttpPost(const char * url, const char* data, std::string & content);
#endif
