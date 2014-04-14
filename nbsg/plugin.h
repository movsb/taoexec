#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <windows.h>
#include <vector>
#include "WindowBase.h"



enum
{
	ENP_INIT=0,
	ENP_ABOUT,

	ENP_MAX_INDEX
};

struct ENTRY_POINT
{
	int entry;
	const char* name;
};

extern ENTRY_POINT m_EntryPoint[];

class APlugin
{
public:
	APlugin(){}
	~APlugin(){}

	typedef std::vector<std::string> PLUGINS;
	typedef PLUGINS::iterator		 PLUGINS_IT;
	
	const char* operator[](int entry_point)
	{
		assert(entry_point < ENP_MAX_INDEX);
		return m_EntryPoint[entry_point].name;
	}

	static void FindAllPlugins(PLUGINS* plugins)
	{
		WIN32_FIND_DATA fd;
		HANDLE hFind = ::FindFirstFile("np*.dll",&fd);
		if(hFind != INVALID_HANDLE_VALUE){
			do{
				if(!(::GetFileAttributes(fd.cFileName)&FILE_ATTRIBUTE_DIRECTORY)){
					plugins->push_back(std::string(fd.cFileName));
				}
			}while(::FindNextFile(hFind,&fd));
			::FindClose(hFind);
		}
	}

	void* GetProcAddress(const char* entry)
	{
		return ::GetProcAddress(m_hInstDll,entry);
	}
	

	BOOL Load(const char* module)
	{
		m_hInstDll = ::LoadLibrary(module);
		if(m_hInstDll){
			npInit = (NPINIT)GetProcAddress((*this)[ENP_INIT]);
			npAbout = (NPABOUT)GetProcAddress((*this)[ENP_ABOUT]);

			return TRUE;
		}else{
			return FALSE;
		}
	}
	BOOL Free()
	{
		::FreeLibrary(m_hInstDll);
	}

public:
	typedef BOOL (*NPINIT)(AWindowBase* parent);
	NPINIT npInit;

	typedef void (*NPABOUT)();
	NPABOUT npAbout;

private:
	HINSTANCE m_hInstMain;
	HINSTANCE m_hInstDll;
};

#endif//!__PLUGIN_H__
