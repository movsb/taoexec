#include "Str.h"
#include "Encode.h"

AStr::AStr(const char* str,bool bUtf8)
{
	if(bUtf8 == false){
		size_t len = strlen(str)+sizeof(char);
		_ansi = new char[len];
		::memcpy(_ansi,str,len);
		_utf8 = new char[len*3];
		AEncode::ansi_to_utf8(_ansi,_utf8);
		_wchar = new wchar_t[len*2];
		AEncode::ansi_to_wchar(_ansi,_wchar);
	}else{
		size_t len = strlen(str)+sizeof(char);
		_utf8 = new char[len];
		::memcpy(_utf8,str,len);
		_ansi = new char[len];
		AEncode::utf8_to_ansi(_utf8,_ansi);
		_wchar = new wchar_t[len*3];
		AEncode::ansi_to_wchar(_ansi,_wchar);
	}
}

AStr::~AStr()
{
	if(_ansi) delete[] _ansi;
	if(_utf8) delete[] _utf8;
}

const char* AStr::toUtf8() const
{
	return _utf8;
}

const char* AStr::toAnsi() const
{
	return _ansi;
}

const wchar_t* AStr::toWchar() const
{
	return _wchar;
}
