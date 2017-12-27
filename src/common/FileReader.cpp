#include "FileReader.h"
#include <iostream>
#include <fstream>
#include<json/reader.h>

using namespace std;
std::string ReadText(const std::string & path)
{
	return std::string();
}

bool ReadJson(Json::Value &root, const char* path)
{
	ifstream file_in(path,ios::in|ios::binary);
	bool ret = false;
	if (!file_in.fail())
	{

		Json::Reader reader;
		ret = reader.parse(file_in, root);
		
	}
	file_in.close();
	return ret;
}
