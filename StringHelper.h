#pragma once
#include <string>



class StringHelper 
{
public: 
	static std::wstring StringToWide(std::string str);
	static std::string GetDirectoryFromFilePath(const std::string& filepath);
	static std::string GetFileExtension(const std::string& filename);
};