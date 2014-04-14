#pragma once
#include <Windows.h>
#include "debug.h"
class AApp
{
public:
	AApp()
	{
		_hInstance = NULL;
		::GetModuleFileName(NULL,m_ProgramDirectory,sizeof(m_ProgramDirectory));
		*strrchr(m_ProgramDirectory,'\\') = '\0';
		::SetCurrentDirectory(m_ProgramDirectory);
		debug_out(("Location:%s\n",m_ProgramDirectory));
	}

	void setInstance(HINSTANCE hInstance) {_hInstance = hInstance;}
	const char* getProgramDirectory() const{return m_ProgramDirectory;}
	HINSTANCE getInstance(){return _hInstance;}

	const char* getAppName() const{return "Software Manager";}
	const char* getAppNameAndVersion() const{return "Software Manager v1.0";}
	const char* getAppVersion() const{return "v1.0";}

public:
	operator HINSTANCE() const{return _hInstance;}

private:
	HINSTANCE _hInstance;
	char m_ProgramDirectory[MAX_PATH];
};

