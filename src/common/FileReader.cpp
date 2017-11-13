#include "FileReader.h"
#include <iostream>
#include <fstream>
#include<json/reader.h>
using namespace std;
std::string ReadText(const std::string & path)
{
	return std::string();
}

bool ReadJson(Json::Value &root, const std::string & path)
{
	ifstream file_in(path,ios::in | ios::binary);
	if (file_in.is_open())
	{

		Json::Reader reader;
		bool ret = reader.parse(file_in, root);
		file_in.close();
		return ret;
	}
	
	return false;
}
