#include "WindowBase.h"
#include "Utils.h"

std::vector<AWindowBase*> AWindowBase::m_Windows;

AWindowBase::AWindowBase()
{
	m_hWnd = NULL;
	m_CtrlID = 0xFFFF;
	m_Parent = NULL;

	m_uMsg = 
	m_wParam = 
	m_lParam = 0;

	m_WndProcOrig = 
	m_WndProc = NULL;

	m_ModalDialogResultCode = 0;
}

AWindowBase::~AWindowBase()
{

}

INT_PTR CALLBACK AWindowBase::WindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	m_uMsg = uMsg;
	m_wParam = wParam;
	m_lParam = lParam;

	switch(uMsg)
	{
	case WM_PAINT:				return OnPaint();
	case WM_COMMAND:			return OnCommand(HIWORD(wParam),LOWORD(wParam),HWND(lParam));
	case WM_CLOSE:				return OnClose();
	case WM_NOTIFY:				return OnNotify(LPNMHDR(lParam));
		
	case WM_DESTROY:			return OnDestroy();
	case WM_INITDIALOG:			return OnInitDialog(hWnd,HWND(wParam),lParam);
	case WM_HOTKEY:				return OnHotKey(wParam);
	case WM_NCDESTROY:			return OnNcDestroy();
	case WM_DISPLAYCHANGE:		return OnDisplayChange(wParam,LOWORD(lParam),HIWORD(lParam));
	case WM_ACTIVATEAPP:		return OnActivateApp(wParam==TRUE,DWORD(lParam));
		
	case WM_USER:				return OnUser(uMsg,wParam,lParam);
	case WM_APP:				return OnApp(uMsg,wParam,lParam);
		
	case WM_SIZE:				return OnSize(LOWORD(lParam),HIWORD(lParam));
	case WM_SIZING:				return OnSizing(wParam,LPRECT(lParam));
		
	case WM_DROPFILES:			return OnDropFiles(HDROP(wParam));
		
	case WM_VSCROLL:			return OnVScroll(wParam,HWND(lParam));
	case WM_CONTEXTMENU:		return OnContextMenu(HWND(wParam),GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
		
	case WM_MOUSEWHEEL:			return OnMouseWheel((int)short(HIWORD(wParam)),LOWORD(wParam),LOWORD(lParam),HIWORD(lParam));
	case WM_MOUSEMOVE:			return OnMouseMove(wParam,GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
	case WM_LBUTTONDOWN:		return OnLButtonDown(wParam,GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
	case WM_LBUTTONUP:			return OnLButtonUp(wParam,GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
	case WM_KILLFOCUS:			return OnKillFocus(HWND(wParam));
		
	case WM_NULL:				return OnNull(lParam);
		
	case WM_CHAR:				return OnChar(wParam);

	case WM_SHOWWINDOW:			return OnShowWindow((BOOL)wParam,(BOOL)(lParam==0));

	case WM_TIMER:				
		{
			typedef VOID (CALLBACK* TP)(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);
			return OnTimer(int(wParam),(TP)lParam);
		}
		
	default:					return this->DoDefault(uMsg,wParam,lParam);
	}
}

void AWindowBase::SubClass()
{
	void* proc = m_WndThunk.Stdcall(this,&AWindowBase::WindowProc);
	m_WndProcOrig = (WNDPROC)SetWindowLongPtr(this->GetHwnd(),GWLP_WNDPROC,LONG(proc));
	assert(m_WndProcOrig != NULL);
}

WNDPROC AWindowBase::GetWindowThunk()
{
	return (WNDPROC)m_WndThunk.Stdcall(this,&AWindowBase::WindowProc);
}

// INT_PTR	AWindowBase::DialogModal()
// {
// 	void* proc = m_WndThunk.Stdcall(this,&AWindowBase::WindowProc);
// 	return DialogBoxParam()
// }


void AWindowBase::DragAcceptFiles(BOOL bAccept)
{
	::DragAcceptFiles(this->GetHwnd(),bAccept);
}

void AWindowBase::EnableWindow(BOOL bEnable)
{
	::EnableWindow(this->GetHwnd(),bEnable);
}

HWND AWindowBase::SetFocus()
{
	return ::SetFocus(this->GetHwnd());
}

HICON AWindowBase::SetIcon(WPARAM iconId,HICON hIcon)
{
	return (HICON)::SendMessage(this->GetHwnd(),WM_SETICON,iconId,LPARAM(hIcon));
}

HICON AWindowBase::GetIcon(WPARAM iconId)
{
	return (HICON)::SendMessage(this->GetHwnd(),WM_GETICON,iconId,0);
}

void AWindowBase::GetWindowRect(RECT* pRc)
{
	::GetWindowRect(this->GetHwnd(),pRc);
}

void AWindowBase::GetClientRect(RECT* pRc)
{
	::GetClientRect(this->GetHwnd(),pRc);
}

void AWindowBase::SetWindowPos(int x,int y,int width/* =-1 */,int height/* =-1 */)
{
	if(width==-1 && height==-1){
		SetWindowPos(NULL,x,y,0,0,SWP_NOZORDER|SWP_NOSIZE);
	}else{
		SetWindowPos(NULL,x,y,width,height,SWP_NOZORDER);
	}
}
void AWindowBase::SetWindowPos(HWND hWndInsertAfter,int x,int y,int width,int height,UINT uFlags)
{
	::SetWindowPos(this->GetHwnd(),hWndInsertAfter,x,y,width,height,uFlags);
}

void AWindowBase::ShowWindow(int nCmdShow)
{
	::ShowWindow(this->GetHwnd(),nCmdShow);
}
void AWindowBase::SetWindowText(const char* szText)
{
	::SetWindowText(this->GetHwnd(),szText);
}

int AWindowBase::GetWindowTextLength() const
{
	return ::GetWindowTextLength(this->GetHwnd());
}

BOOL AWindowBase::GetWindowText(char* dst,int cb)
{
	int len = GetWindowTextLength();
	if(cb < len){ 
		*dst = '\0';
		return FALSE;
	}
	return ::GetWindowText(this->GetHwnd(),dst,cb)>0;
}

std::string AWindowBase::GetWindowText()
{
	int len = GetWindowTextLength();
	if(len == 0) return "";
	else{
		char* s = new char[len+1];
		memset(s,0,len+1);
		GetWindowText(s,len+1);
		std::string str = s;
		delete s;
		return str;
	}
}

void AWindowBase::DestroyWindow()
{
	::DestroyWindow(this->GetHwnd());
}

BOOL AWindowBase::EndDialog(INT_PTR Result)
{
	return ::EndDialog(this->GetHwnd(),Result);
}

/***********************************************************************
名称:setWindowForeground
描述:设置到前台窗口
参数:
返回:
说明:
***********************************************************************/
BOOL AWindowBase::SetWindowForeground()
{
	HWND hForeWnd = GetForegroundWindow();
	if(hForeWnd != this->GetHwnd()){
		DWORD dwForeID,dwCurrID;
		dwCurrID = GetCurrentThreadId();
		dwForeID = GetWindowThreadProcessId(hForeWnd,NULL);
		if(AttachThreadInput(dwCurrID,dwForeID,TRUE)){
			ShowWindow(SW_SHOWNORMAL);
			::SetForegroundWindow(this->GetHwnd());
			AttachThreadInput(dwCurrID,dwForeID,FALSE);
			return true;
		}else{
			return false;
		}
	}
	return true;
}

/**************************************************
函  数:centerWindow@8
功  能:把指定窗口居中于指定的另一窗口
参  数:
		hWndOwner - 参考窗口句柄
返回值:
说  明:若居中于屏幕,置hWndOwner为NULL
	只考虑了任务栏在屏幕下方的情况
**************************************************/
void AWindowBase::CenterWindow(HWND hWndOwner)
{
	RECT rchWnd,rchWndOwner,rcTaskBar;
	HWND hTaskBar;
	int width,height;
	int scrWidth,scrHeight;
	int x,y;

	//if(!IsWindow(_hWnd)) return;
	GetWindowRect(&rchWnd);

	scrWidth = GetSystemMetrics(SM_CXSCREEN);
	scrHeight = GetSystemMetrics(SM_CYSCREEN);

	if(!hWndOwner||!IsWindow(hWndOwner)){
		SetRect(&rchWndOwner,0,0,scrWidth,scrHeight);
	}else{
		::GetWindowRect(hWndOwner,&rchWndOwner);
	}

	width = rchWnd.right-rchWnd.left;
	height = rchWnd.bottom-rchWnd.top;
	
	x = (rchWndOwner.right-rchWndOwner.left-width)/2+rchWndOwner.left;
	y = (rchWndOwner.bottom-rchWndOwner.top-height)/2+rchWndOwner.top;

	if(x > scrWidth) x -= 20;
	if(y > scrHeight){
		y = scrHeight - 10;
		hTaskBar = FindWindow("Shell_TrayWnd",NULL);
		if(hTaskBar){
			::GetWindowRect(hTaskBar,&rcTaskBar);
			y -= rcTaskBar.bottom-rcTaskBar.top;
		}
	}
	MoveWindow(this->GetHwnd(),x,y,width,height,TRUE);
}

DWORD AWindowBase::GetStyle() const
{
	return ::GetWindowLongPtr(this->GetHwnd(),GWL_STYLE);
}

void AWindowBase::SetStyle(DWORD dwStyle) const
{
	::SetWindowLongPtr(this->GetHwnd(),GWL_STYLE,(LONG)dwStyle);
}
