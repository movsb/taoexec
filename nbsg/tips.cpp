#include <Windows.h>
#include <cstdarg>
#include <cstdio>
#include "tips.h"

#include <UIlib.h>

using namespace DuiLib;

class CTipsDlg : public WindowImplBase
{
public:
	CTipsDlg(const CDuiString& str)
	{
		m_str = str;
	}

protected:
	virtual CDuiString GetSkinFolder()
	{
		return "skin/";
	}
	virtual CDuiString GetSkinFile()
	{
		return "Tips.xml";
	}
	virtual LPCTSTR GetWindowClassName(void) const
	{
		return "Å®º¢²»¿Þ";
	}
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if(uMsg == WM_TIMER){
			if(wParam==0){
				::KillTimer(GetHWND(),0);
				Close();
				bHandled = TRUE;
				return 0;
			}
		}
		bHandled = FALSE;
		return 0;
	}
	virtual void OnFinalMessage( HWND hWnd )
	{
		__super::OnFinalMessage(hWnd);
		delete this;
	}
	virtual void InitWindow()
	{
		::SetWindowLongPtr(GetHWND(),GWL_EXSTYLE,::GetWindowLongPtr(GetHWND(),GWL_EXSTYLE)|WS_EX_TOOLWINDOW);
		m_pRichEdit = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("rich")));
		m_pRichEdit->SetText(m_str);
		::SetTimer(GetHWND(),0,3000,NULL);
	}

private:
	CDuiString m_str;
	CRichEditUI* m_pRichEdit;
};

void __cdecl ShowTips(const char* fmt,...)
{
	char buf[4096];
	va_list va;
	va_start(va,fmt);
	_vsnprintf(buf,sizeof(buf),fmt,va);
	va_end(va);

	CTipsDlg* pTips = new CTipsDlg(buf);
	pTips->Create(NULL,NULL,WS_VISIBLE|WS_POPUP,0);
	//pTips->CenterWindow();
	//::SetWindowPos(pTips->GetHWND(),HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW|SWP_NOACTIVATE);
	//:: 
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);

	RECT rc;
	::GetWindowRect(pTips->GetHWND(),&rc);
	int w=rc.right-rc.left,h=rc.bottom-rc.top;

	::SetWindowPos(pTips->GetHWND(),HWND_TOPMOST,(width-w)/2,int(height*2/9.0),0,0,SWP_NOSIZE);
	::ShowWindow(pTips->GetHWND(),SW_SHOWNOACTIVATE);
	//pTips->ShowWindow(true,false);
}

