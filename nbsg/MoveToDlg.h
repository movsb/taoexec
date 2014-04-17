#pragma once

#include "ChildIndexDlg.h"

class CMoveToDlg
{
public:
	enum RET_TYPE{
		MOVETO_CANCELED,
		MOVETO_COPYTO,
		MOVETO_MOVETO
	};
public:
	CMoveToDlg(HWND parent,AChildIndexDlg::LPARAM_STRUCT**ppls,size_t count,const char* origTableName);
	~CMoveToDlg(){}
	RET_TYPE GetDlgCode() const {return m_dlgcode;}
	RET_TYPE m_dlgcode;
};
