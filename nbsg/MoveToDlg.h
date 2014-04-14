#pragma once
//#include "Window.h"
#include "ChildIndexDlg.h"


class AMoveToDlg:public AWindowBase
{
public:
	enum RET_TYPE{
		MOVETO_CANCELED,
		MOVETO_COPYTO,
		MOVETO_MOVETO
	};
public:
	AMoveToDlg(AWindowBase* parent,AChildIndexDlg::LPARAM_STRUCT**ppls,size_t count,const char* origTableName);
	~AMoveToDlg();

public:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl);
	INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam);
	INT_PTR OnClose();
private:
	AButton *pBtnMoveTo,*pBtnCopyTo,*pBtnCancel;
	ASettingsSqlite* pSettings;
	const char* tableName;
	AChildIndexDlg::LPARAM_STRUCT* *m_ppls;
	size_t m_count;
};
