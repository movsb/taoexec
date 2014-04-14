#pragma once
#include "Windowbase.h"
#include "ListView.h"

class AAddDlg:public AWindowBase
{
public:
	enum{TYPE_PATH,TYPE_MODIFY,TYPE_NEW};
	AAddDlg(AWindowBase* parent,const char* table,int type,LPARAM lParam);
	~AAddDlg();
public:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl);
	INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM lParam);
	INT_PTR OnClose();
	INT_PTR OnDropFiles(HDROP hDrop);
	INT_PTR OnNcDestroy();
	INT_PTR OnNull(LPARAM lParam);

private:
	void MakeIndex(int type,LPARAM lParam);
	void MakeTables();

private:
	AEditBox *m_pEditIndex,*m_pEditComment,*m_pEditPath,*m_pEditParam,*m_pEditTimes;
	AButton  *m_pBtnBrowse,*m_pBtnBatch,*m_pBtnNew,*m_pBtnSave,*m_pBtnClose;
	
private:
	int m_type;
	LPARAM m_lParam;
	char* m_zTables;
	const char* m_Table;
	BOOL bNeedFree;
};

