#ifndef __UTILS_H__
#define __UTILS_H__

#include <windows.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

class AUtils
{
public:
	static int __cdecl msgbox(HWND hWnd,UINT msgicon, const char* caption, const char* fmt, ...);
	static void msgerr(HWND hWnd,char* prefix);
	static void myassert(void* pv,char* str);
	static bool setClipData(const char* str);
	static bool isObjOnStack(void* pObjAddr);
};

#endif//!__UTILS_H__
