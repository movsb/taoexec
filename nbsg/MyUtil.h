#pragma once

#include <Windows.h>
#include <vector>
#include <string>

class CDropFiles
{
public:
	CDropFiles(HDROP hDrop,std::vector<std::string>* files);
};

class CCharset
{
public:
	static bool Acp2Unicode(std::string& str,std::wstring* wstr);
};