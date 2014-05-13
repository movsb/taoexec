#include "StdAfx.h"
#include "MyUtil.h"

CDropFiles::CDropFiles(HDROP hDrop,std::vector<std::string>* files)
{
	files->empty();
	TCHAR a[MAX_PATH];
	UINT c = ::DragQueryFile(hDrop,-1,nullptr,0);
	for(UINT i=0; i<c; ++i){
		DragQueryFile(hDrop, i, a, MAX_PATH);
		files->push_back(a);
	}
	::DragFinish(hDrop);
}

bool CCharset::Acp2Unicode(std::string& str,std::wstring* wstr)
{
	int rv;
	wchar_t* pws = nullptr;
	rv = ::MultiByteToWideChar(CP_ACP,0,str.c_str(),-1,nullptr,0);
	if(rv <= 0)	return false;
	pws = new wchar_t[rv*2];
	MultiByteToWideChar(CP_ACP,0,str.c_str(),-1,pws,rv);
	*wstr = pws;
	delete[] pws;
	return true;
}
