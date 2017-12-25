#pragma once
#include <json/json.h>
#include <event2/event.h>
#ifndef __TOOLS_H__
#define __TOOLS_H__
/*
** return a random integer in the interval
** [a, b]
*/
int RandomRange(int a, int b);
void ParseJsonValue(Json::Value json, const char* key, int &value);
void ParseJsonValue(Json::Value json, const char* key, float &value);
void ParseJsonValue(Json::Value json, const char* key, bool &value);
void ParseJsonValue(Json::Value json, const char* key, char* str, int len);
bool ParseSockAddr(sockaddr_in & addr, const char * str, bool by_name);
#endif // !__TOOLS_H__
