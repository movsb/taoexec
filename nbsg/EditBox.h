#pragma once
#include "WindowBase.h"
class AEditBox:public AWindowBase
{
public:
	AEditBox(void);
	~AEditBox(void);

public:
	BOOL GetModify();
	void SetModify(BOOL bModified);
	DWORD GetSel(DWORD* dwStart,DWORD* dwEnd);
	BOOL CanUnDo();
	void DoCopy();
	void DoCut();
	void DoPaste();
	void DoDelete();
	void DoSelAll();
	void DoUndo();

public:
	INT_PTR OnContextMenu(HWND hWnd,int x,int y);
	INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl);
	INT_PTR OnDropFiles(HDROP hDrop);
	INT_PTR OnChar(int key);
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
	{
		return CallWindowProc(m_WndProcOrig,this->GetHwnd(),uMsg,wParam,lParam);
	}
};
