#ifndef __TIPS_H__
#define __TIPS_H__

#if _MSC_VER == 1200
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <windows.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void __cdecl show_tips(char* fmt,...);

#ifdef __cplusplus
}
#endif

#endif//!__TIPS_H__
