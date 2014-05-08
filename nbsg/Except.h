#pragma once

#include <string>
#include <vector>

class CExcept
{
public:
	CExcept(const char* desc,const char* func)
	{
		this->desc = desc;
		this->stack.push_back(func);
	}
	std::string desc;
	std::vector<std::string> stack;
};
