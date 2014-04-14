#ifndef __ENCODE_H__
#define __ENCODE_H__

#include <windows.h>
#include <cstring>

class AEncode
{
public:
	static int utf8_to_ansi(const char* UTF8Str, char* pAnsi);
	static int ansi_to_utf8(const char* AnsiStr, char* pDst);
	static int ansi_to_wchar(const char* Ansi, wchar_t* pDst);
};

#endif//!__ENCODE_H__
