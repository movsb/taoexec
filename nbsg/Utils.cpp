#include "Utils.h"
#include <cassert>
#include <cstdio>

/***************************************************
函  数:msgbox
功  能:显示消息框
参  数:
	msgicon:消息光标
	caption:对话框标题
	fmt:格式字符串
	...:变参
返回值:
	用户点击的按钮对应的值(MessageBox)
说  明:
***************************************************/
int __cdecl AUtils::msgbox(HWND hWnd,UINT msgicon, const char* caption, const char* fmt, ...)
{
	assert(hWnd==NULL || ::IsWindow(hWnd));
	va_list va;
	char smsg[1024]={0};
	va_start(va, fmt);
	_vsnprintf(smsg, sizeof(smsg)/sizeof(*smsg), fmt, va);
	va_end(va);
	return MessageBox(hWnd, smsg, caption, msgicon);
}

/***************************************************
函  数:msgerr
功  能:显示带prefix前缀的系统错误消息
参  数:prefix-前缀字符串
返回值:(无)
说  明:
***************************************************/
void AUtils::msgerr(HWND hWnd,char* prefix)
{
	char* buffer = NULL;
	if(!prefix) prefix = "";
	if(FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,GetLastError(),MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),(LPSTR)&buffer,0,NULL)) 
	{
		msgbox(hWnd,MB_ICONHAND, NULL, "%s:%s", prefix, buffer);
		LocalFree(buffer);
	}
	else
	{
		msgbox(hWnd,MB_ICONSTOP,NULL,"msg err");
	}
}


/***********************************************************************
名称:Assert
描述:Debug
参数:pv-任何表达式,str-提示
返回:
说明:
***********************************************************************/
void AUtils::myassert(void* pv,char* str)
{
	if(!pv){
		msgbox(NULL,MB_ICONERROR,NULL,"Debug Error:%s\n\n"
			"应用程序遇到内部错误,请报告错误!"
			"请重新启动应用程序后再试!",str==NULL?"<null>":str);
	}
}

/***************************************************
函  数:set_clip_data
功  能:复制str指向的字符串到剪贴板
参  数:
	str:字符串,以0结尾
返回值:
	成功:非零
	失败:零
说  明:
***************************************************/
bool AUtils::setClipData(const char* str)
{
	HGLOBAL hGlobalMem = NULL;
	char* pMem = NULL;
	int lenstr;

	if(str == NULL) return false;
	if(!OpenClipboard(NULL)) return false;

	lenstr = strlen(str)+sizeof(char);//Makes it null-terminated
	hGlobalMem = GlobalAlloc(GHND, lenstr);
	if(!hGlobalMem) return false;
	pMem = (char*)GlobalLock(hGlobalMem);
	EmptyClipboard();
	memcpy(pMem, str, lenstr);
	SetClipboardData(CF_TEXT, hGlobalMem);
	CloseClipboard();
	GlobalFree(hGlobalMem);
	return true;
}

/**************************************************
函  数:
功  能:判断一个类对象是否在栈上
参  数:
返  回:
说  明:不能判断全局对象
**************************************************/
bool AUtils::isObjOnStack(void* pObjAddr)
{
	bool bOnStack = true;
	MEMORY_BASIC_INFORMATION mbi = {0};
	if(VirtualQuery(&bOnStack,&mbi,sizeof(mbi))){
		bOnStack = pObjAddr>=mbi.BaseAddress && (DWORD)pObjAddr<=(DWORD)mbi.BaseAddress+mbi.RegionSize;
	}else{
		
	}
	return bOnStack;
}