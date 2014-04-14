#include "tips.h"
#include "debug.h"
#include "resource.h"
#include <assert.h>
static char strtoshow[1024];
static HWND hWndTips;
static HFONT hFont;
static HBRUSH hDlgBackgroudBrush;
static POINT initMousePos;

#define TIPS_TEXT_COLOR		RGB(255,255,255)
#define TIPS_TEXT_BKCOLOR	RGB(163,73,164)
#define TIPS_DLG_BKCOLOR	RGB(0,0,0)
//#define TIPS_ROUND_COLOR	TIPS_TEXT_BKCOLOR


void update_tips_window_pos(int cx, int cy)
{
	int cxScreen,cyScreen;
	int cxClient,cyClient;

	cxClient = cx;
	cyClient = cy;

	cxClient += 20;
	cyClient += 20;

	cxScreen=GetSystemMetrics(SM_CXSCREEN);
	cyScreen=GetSystemMetrics(SM_CYSCREEN);

	SetWindowPos(hWndTips,HWND_TOPMOST,(cxScreen-cxClient)/2,(int)((cyScreen-cyClient)/2*(0.25)),cxClient,cyClient,SWP_NOACTIVATE|SWP_SHOWWINDOW);
}

int CALLBACK TipsDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_TIMER:
		{
			int xx;
			RECT rc;
			POINT pt;
			GetWindowRect(hwndDlg,&rc);
			GetCursorPos(&pt);
			if((pt.x>=rc.left && pt.x<=rc.right)
				&& (pt.y>=rc.top && pt.y<=rc.bottom)
				&& (initMousePos.x!=pt.x && initMousePos.y != pt.y))
			{
				SetTimer(hwndDlg,0,500,NULL);
				return 0;
			}
			KillTimer(hwndDlg, 0);
			for(;;){
				for(xx=255; xx>0; xx-=8){
					SetLayeredWindowAttributes(hwndDlg,TIPS_DLG_BKCOLOR,(unsigned char)xx,LWA_COLORKEY|LWA_ALPHA);
					Sleep(10);
				}
				break;
			}
			DestroyWindow(hwndDlg);
			return 0;
		}
	case WM_LBUTTONDOWN:
		SendMessage(hwndDlg,WM_NCLBUTTONDOWN,HTCAPTION,0);
		return 0;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			RECT rc;
			SIZE sz;

			int cch;

			if(!*strtoshow)
				break;

			cch = strlen(strtoshow);
			if(cch == 0) break;

			hdc = BeginPaint(hwndDlg, &ps);
			SelectObject(hdc,(HGDIOBJ)hFont);
			SetBkColor(hdc, TIPS_TEXT_BKCOLOR);
			SetTextColor(hdc, TIPS_TEXT_COLOR);

			
			GetTextExtentPoint32(hdc,strtoshow,strlen(strtoshow),&sz);

			update_tips_window_pos(sz.cx,sz.cy);

			GetClientRect(hwndDlg,&rc);
			DrawText(hdc,strtoshow,strlen(strtoshow),&rc,DT_CENTER|DT_HIDEPREFIX|DT_SINGLELINE|DT_VCENTER);

			*strtoshow = '\0';

			EndPaint(hwndDlg, &ps);
			return 0;	
		}
	case WM_CTLCOLORDLG:
		return (INT_PTR)hDlgBackgroudBrush;
	case WM_INITDIALOG:
		ShowWindow(hwndDlg,SW_HIDE);
		hFont = CreateFont(
			30,12, /*Height,Width*/
			0,0, /*escapement,orientation*/
			FW_NORMAL,FALSE,FALSE,FALSE, /*weight, italic, underline, strikeout*/
			ANSI_CHARSET,OUT_DEVICE_PRECIS,CLIP_MASK, /*charset, precision, clipping*/
			DEFAULT_QUALITY, DEFAULT_PITCH, /*quality, and pitch*/
			//Config.Get(CFG_FONT)); /*font name*/
			//TODO:
			"Segoe UI");
		hDlgBackgroudBrush = CreateSolidBrush(TIPS_TEXT_BKCOLOR);
		GetCursorPos(&initMousePos);
		//SetWindowPos(hwndDlg, HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE/*|SWP_SHOWWINDOW*/);
		SetWindowLong(hwndDlg,GWL_EXSTYLE,GetWindowLong(hwndDlg,GWL_EXSTYLE)|WS_EX_LAYERED);
		SetLayeredWindowAttributes(hwndDlg,TIPS_DLG_BKCOLOR,255,LWA_COLORKEY|LWA_ALPHA);
		//ShowWindow(hwndDlg,SW_SHOWNOACTIVATE);
		return 0;
	case WM_DESTROY:
		DeleteObject(hDlgBackgroudBrush);
		DeleteObject(hFont);
		hWndTips = NULL;
		break;
	}
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	return 0;
}

void __cdecl show_tips(char* fmt,...)
{
	va_list va;
	va_start(va, fmt);
	_vsnprintf(strtoshow, sizeof(strtoshow), fmt, va);
	va_end(va);

	if(IsWindow(hWndTips)){
		RECT rc;
		debug_out(("IsWindow==TRUE\n"));
		KillTimer(hWndTips,0);
		GetClientRect(hWndTips,&rc);
		InvalidateRect(hWndTips,&rc,TRUE);
	}else{
		debug_out(("CreateNewWindow\n"));
		hWndTips = CreateDialogParam(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_TIP),NULL,TipsDialogProc,0);
		assert(hWndTips!=NULL);
		if(hWndTips==NULL) return;
	}
	debug_out(("SetTimer\n"));
	SetTimer(hWndTips,0,3000,NULL);
}
