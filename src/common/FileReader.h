#pragma once
#ifndef __FILE_READER_H__
#define __FILE_READER_H__

#include <string>
#include<json/json.h>
std::string ReadText(const std::string &path);
bool ReadJson(Json::Value &root, const char* path);
#endif // !__FILE_READER_H__
