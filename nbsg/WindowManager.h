#pragma once
#include <windows.h>
#include <vector>

class AWindowBase;

class AWindowManager
{
public:
	void insert(AWindowBase* pWindow);
	virtual void remove(AWindowBase* pWindow);
	bool empty();

public:
	std::vector<AWindowBase*> m_Windows;
};

class AWindowManagerOutside:public AWindowManager
{
public:
	void remove(AWindowBase* pWindow)
	{
		this->AWindowManager::remove(pWindow);
		if(m_Windows.size()==0){
			::PostQuitMessage(0);
		}
	}
};
