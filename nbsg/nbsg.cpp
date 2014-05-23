#include "StdAfx.h"

#include "App.h"
#include "nbsg.h"
#include "PathLib.h"
#include "res/resource.h"
#include "SQLite.h"

using namespace DuiLib;

#include "MainDlg.h"
#include "Mini.h"
#include "InputBox.h"

AApp* g_pApp;

std::fstream __debug_file;

int CALLBACK WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{

	g_pApp = new AApp;
	g_pApp->setInstance(hInstance);

	CPaintManagerUI::SetInstance(GetModuleHandle(NULL));
	CPaintManagerUI::StartupGdiPlus();

	__debug_file.open(
		CPaintManagerUI::GetInstancePath()+"debug.txt",
		std::ios_base::binary|std::ios_base::app
		// open prop.
		);
	if(!__debug_file.is_open()){
		::MessageBox(nullptr,"请不要多次运行程序!",nullptr,MB_ICONINFORMATION);
		return 1;
	}

	CSQLite* db = new CSQLite;
	SMART_ENSURE(db->Open("./data.db"),==true);
	CMainDlg* dlg = new CMainDlg(db);
	//CMini* mini = new CMini(db);

	CPaintManagerUI::MessageLoop();

	delete dlg;

	SMART_ENSURE(db->Close(),==true);
	delete db;

	CPaintManagerUI::ShutdownGdiPlus();

	delete g_pApp;

	MessageBeep(MB_OK);

	return 0;
}
