#pragma once
#include <Windows.h>
#include <WindowsX.h>
#include <CommCtrl.h>
#include <iostream>
#include <cassert>
#include <vector>

#include "Thunk.h"
#include "debug.h"
#include "WindowManager.h"

class AWindowBase;
struct ControlMessage
{
	AWindowBase* self;
	UINT uMsg;
	WPARAM wParam;
	LPARAM lParam;
};


class AWindowBase
{
public:
	AWindowBase();
	virtual ~AWindowBase();
public:
	
	void DragAcceptFiles(BOOL bAccept);
	void EnableWindow(BOOL bEnable);
	HWND SetFocus();
	HICON SetIcon(WPARAM iconId,HICON hIcon);
	HICON GetIcon(WPARAM iconId);
	void GetWindowRect(RECT* pRc);
	void GetClientRect(RECT* pRc);
	void SetWindowPos(int x,int y,int width=-1,int height=-1);
	void SetWindowPos(HWND hWndInsertAfter,int x,int y,int width,int height,UINT uFlags);
	void ShowWindow(int nCmdShow);
	void SetWindowText(const char* szText);
	BOOL GetWindowText(char* dst,int cb);
	std::string GetWindowText();
	int GetWindowTextLength() const;
	void DestroyWindow();
	BOOL EndDialog(INT_PTR Result);
	BOOL SetWindowForeground();
	void CenterWindow(HWND hWndOwner);
	DWORD GetStyle() const;
	void SetStyle(DWORD dwStyle) const;


public:
	virtual void attach(AWindowBase* hParent,int ctrlID)
	{
		AttachCtrl(hParent,ctrlID);
	}
	virtual BOOL AttachCtrl(AWindowBase* pParent,int ctrlID)
	{
		this->SetParent(pParent);
		assert(pParent != NULL);
		this->m_hWnd = GetDlgItem(pParent->GetHwnd(),ctrlID);
		assert(this->m_hWnd != NULL);
		this->m_CtrlID = ctrlID;
		return TRUE;
	}

public:

	HWND GetHwnd() const
	{
		assert(m_hWnd != NULL && "AWindowBase::GetHwnd()");
		return m_hWnd;
	}
	UINT GetCtrlID() const
	{
		return m_CtrlID;
	}
	AWindowBase* GetParent() const
	{
		//assert(m_Parent!=NULL && "AWindowBase::GetParent()");
		return m_Parent;
	}
	void SetParent(AWindowBase* parent)
	{
		m_Parent = parent;
	}

public://当前的窗口消息
	UINT m_uMsg;
	WPARAM m_wParam;
	LPARAM m_lParam;

public://窗口消息处理过程
#define DODEFAULT DoDefault(m_uMsg,m_wParam,m_lParam)
	INT_PTR CALLBACK WindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	void SubClass();
	WNDPROC GetWindowThunk();

	int GetDlgCode() {return m_ModalDialogResultCode;}
	INT_PTR NotifyParent(ControlMessage* pcm=NULL)
	{
		if(pcm){
			return this->GetParent()->SendMessage(WM_NULL,0,LPARAM(pcm));
		}else{
			ControlMessage cm;
			cm.self = this;
			cm.uMsg = m_uMsg;
			cm.wParam = m_wParam;
			cm.lParam = m_lParam;
			return ::SendMessage(this->GetParent()->GetHwnd(),WM_NULL,0,LPARAM(&cm));
		}
	}
	INT_PTR SetDlgResult(INT_PTR Result)
	{
		SetWindowLong(this->GetHwnd(),DWL_MSGRESULT,LONG(Result));
		return TRUE;
	}
	//INT_PTR	DialogModal();

	INT_PTR SendMessage(UINT uMsg,WPARAM wParam=0,LPARAM lParam=0){return ::SendMessage(this->GetHwnd(),uMsg,wParam,lParam);}
	INT_PTR MessageBox(LPCTSTR lpText,LPCTSTR lpCaption=TEXT(""),UINT uType=MB_OK){return ::MessageBox(this->GetHwnd(),lpText,lpCaption,uType);}

	virtual INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam){m_hWnd=hWnd;return DODEFAULT;}
	virtual INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl){return DODEFAULT;}
	virtual INT_PTR OnNotify(LPNMHDR phdr){return DODEFAULT;}
	virtual INT_PTR OnClose(){return DODEFAULT;}
	virtual INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam){MessageBox(TEXT("请务必重载DoDefault!"),NULL,MB_OK);return 0;}
	virtual INT_PTR DoDefault(ControlMessage* pcm){return DoDefault(pcm->uMsg,pcm->wParam,pcm->lParam);}
	virtual INT_PTR OnDestroy(){return DODEFAULT;}
	virtual INT_PTR OnNcDestroy(){return DODEFAULT;}
	virtual INT_PTR OnHotKey(WPARAM id){return DODEFAULT;}
	virtual INT_PTR OnDisplayChange(WPARAM imageDepth,int cxScreen,int cyScreen){return DODEFAULT;}
	virtual INT_PTR OnActivateApp(bool bActivate,DWORD dwThreadID){return DODEFAULT;}
	virtual INT_PTR OnUser(UINT uMsg,WPARAM wParam,LPARAM lParam){return DODEFAULT;}
	virtual INT_PTR OnApp(UINT uMsg,WPARAM wParam,LPARAM lParam){return DODEFAULT;}
	virtual INT_PTR OnSize(int width,int height){return DODEFAULT;}
	virtual INT_PTR OnSizing(WPARAM wParam,LPRECT pRect){return DODEFAULT;}
	virtual INT_PTR OnDropFiles(HDROP hDrop){return DODEFAULT;}
	virtual INT_PTR OnVScroll(WPARAM wParam,HWND hScrollNull){return DODEFAULT;}
	virtual INT_PTR OnContextMenu(HWND hWnd,int x,int y){return DODEFAULT;}
	virtual INT_PTR OnMouseWheel(int delta,int key,int x,int y){return DODEFAULT;}
	virtual INT_PTR OnMouseMove(int key,int x,int y){return DODEFAULT;}
	virtual INT_PTR OnNull(LPARAM lParam){return DODEFAULT;}
	virtual INT_PTR OnLButtonDown(int key,int x,int y){return DODEFAULT;}
	virtual INT_PTR OnLButtonUp(int key,int x,int y){return DODEFAULT;}
	virtual INT_PTR OnKillFocus(HWND hWndFocus){return DODEFAULT;}
	virtual INT_PTR OnChar(int key){return DODEFAULT;}
	virtual INT_PTR OnPaint(){return DODEFAULT;}
	virtual INT_PTR OnShowWindow(BOOL bShow,BOOL bCallFromShowWindow){return DODEFAULT;}
	virtual INT_PTR OnTimer(int nID,VOID (CALLBACK* TimerProc)(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)){return DODEFAULT;}

protected://窗口的相关成员
	HWND m_hWnd;
	UINT m_CtrlID;
	AWindowBase* m_Parent;
	AThunk m_WndThunk;
	WNDPROC m_WndProcOrig;
	WNDPROC m_WndProc;
	//BOOL m_AmIADialog;
	int m_ModalDialogResultCode;

public:
	void AddWindow(AWindowBase* pw)
	{
		m_Windows.push_back(pw);
	}
	void DeleteWindow(AWindowBase* pw)
	{
		for(std::vector<AWindowBase*>::iterator i=m_Windows.begin(); i!=m_Windows.end(); ++i){
			if(*i==pw){
				m_Windows.erase(i);
				break;
			}
		}
		if(m_Windows.size()==0){
			PostQuitMessage(0);
		}
	}
private:
	static std::vector<AWindowBase*> m_Windows;
};


class AWindowBase;
	class AChildIndexDlg;
	class AChildDesktopDlg;
	class AChildFileDlg;
	class AChildSysDlg;
	class AAddDlg;
	class ASysIconDlg;
	class AChildSettingsDlg;
	class AMoveToDlg;
	
	class AMainDlg;
	class AMini;

	class AListView;
	

	class AEditBox;
	class AButton;
	class ATabCtrl;
