#ifndef __PATHLIB_H__
#define __PATHLIB_H__

using namespace std;

class APathLib
{
public:
	static bool isFileExists(const char* pszFile,bool bExpandEnvVar=true);
	static HICON getFileIcon(const char* filespec);
	static HICON GetIndexedFileIcon(const char* filespec);
	static string getSpecialFolder(int csidl);
	static string getFileDir(const char* src);
	static char* trim(char* src);
	static bool showDir(HWND hWnd,const char* filespec);
	static char* toLowerCase(char* src);
	static bool getOpenFileName(HWND hwnd,char* title, char* filter,char* buffer);
	static bool shellExec(HWND hWnd,const char* exec,const char* param,const char* dir);
	static string expandEnvString(const char* src);
	static bool ParseLnk(const wchar_t* lnk,string& path,string& args,string& description);
	static bool getFileDescription(const char* file,string& description);
	static bool getNameString(const char* file,string& name);
	static bool IsLink(const char* file);
	static HICON GetClsidIcon(string guid_str);
private:
	APathLib(){}
};

#endif//!__PATHLIB_H__
