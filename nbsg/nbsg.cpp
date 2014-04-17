#define NBSG_CPP
#include "nbsg.h"
#include "SQLite.h"
#include "MainDlg.h"
#include "Mini.h"
#include "WindowManager.h"

#include <uilib.h>

using namespace DuiLib;

//#pragma comment(lib,"imm32.lib")

ASqliteBase* g_pSqliteBase;
AApp* g_pApp;
AWindowManager* g_pWindowManager;

int CALLBACK WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{
#ifdef _DEBUG
	AllocConsole();
	debug_out(("Program Starting...\n"));
#endif
	g_pApp = new AApp;
	g_pSqliteBase = new ASqliteBase;
	g_pWindowManager = new AWindowManagerOutside;

	g_pApp->setInstance(hInstance);

	g_pSqliteBase->sethParent(NULL);
	if(!g_pSqliteBase->open("nbs.db")){
		return 1;
	}

//	ImmDisableIME(GetCurrentThreadId());

	AMini* mini = new AMini(NULL);
	mini->SetWindowPos(HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

//	AMainDlg* maindlg = new AMainDlg(lpCmdLine,nShowCmd!=SW_HIDE);
// 	AMainDlg* maindlg2 = new AMainDlg(lpCmdLine,nShowCmd!=SW_HIDE);
// 	AMainDlg* maindlg3 = new AMainDlg(lpCmdLine,nShowCmd!=SW_HIDE);
// 	
// 	

// 	HINSTANCE hInst = LoadLibrary("memo.dll");
// 
// 	typedef void (*INIT)(AWindowBase* parent);
// 	INIT init = (INIT)GetProcAddress(hInst,"?init@@YAXPAVAWindowBase@@@Z");
// 	if(init){
// 		init(maindlg);
// 	}
// 	
	CPaintManagerUI::SetInstance(GetModuleHandle(NULL));
	//CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath()+"skin/");

	CPaintManagerUI::MessageLoop();

	g_pSqliteBase->close();

	delete g_pSqliteBase;
	delete g_pApp;
	delete g_pWindowManager;

	MessageBeep(MB_OK);

#ifdef _DEBUG
	debug_out(("Program terminated...\nPress a key to continue...\n"));
	char tmp;
	DWORD dwRead;
	ReadConsole(GetStdHandle(STD_INPUT_HANDLE),&tmp,1,&dwRead,NULL);
	FreeConsole();
#endif // _DEBUG

	return 0;
}
