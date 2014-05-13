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

int CALLBACK WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{

	g_pApp = new AApp;

	g_pApp->setInstance(hInstance);

	CPaintManagerUI::SetInstance(GetModuleHandle(NULL));

#ifdef _DEBUG

#endif

	CSQLite* db = new CSQLite;
	db->Open("./data.db");
	
	CMainDlg* dlg = new CMainDlg(db);
	//CMini* mini = new CMini(db);

	CPaintManagerUI::MessageLoop();

	delete dlg;

	db->Close();
	delete db;

	delete g_pApp;

	MessageBeep(MB_OK);

	return 0;
}
