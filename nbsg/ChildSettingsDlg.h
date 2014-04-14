#pragma once
#include "WindowBase.h"

class AChildSettingsDlg:public AWindowBase
{
public:
	AChildSettingsDlg(AWindowBase* hParentBase);
	~AChildSettingsDlg(void);

private:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnNotify(LPNMHDR phdr);
	INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM lParam);
	//INT_PTR OnClose();
	INT_PTR OnDestroy();
	INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl);
	INT_PTR OnVScroll(WPARAM wParam,HWND hScrollNull);
	INT_PTR OnNull(LPARAM lParam);

private:
	void SetAppIcon(const string& icon);

private:
	ASettingsSqlite*	m_pSettings;
	AEditBox*			m_pEditWindowTitle;
	AEditBox*			m_pEditIndexList;
	AEditBox*			m_pEditDirList;
	AEditBox*			m_pEditIcon;
	AButton*			m_pBtnIcon;
	AButton*			m_pBtnTopmost;
	AButton*			m_pBtnAutoHide;
	AButton*			m_pBtnAutoRun;
	AButton*			m_pBtnShowDesktop;
};

