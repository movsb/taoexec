#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "SQLite.h"

using namespace std;

class AMini:public AWindowBase
{
public:
	AMini(AWindowBase* parent);
	~AMini(void);

private:
	struct CALLBACK_RESULT_ENTRY{
		AIndexSqlite::SQLITE_INDEX index;
		string strDatabase;
	};

	struct CALLBACK_RESULT{
		vector<CALLBACK_RESULT_ENTRY*> result;
		bool found;
		string findstr;
	};

private:
	bool create();
	void BuildPaths();
	bool parseCommandString(std::string& str);
	void updateMiniPos();
	bool RunShell(const string& cmd,const string& arg);

public:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl);
	INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam);
	INT_PTR OnSize(int width,int height);

	INT_PTR OnDestroy();
	INT_PTR OnNcDestroy();

	INT_PTR OnHotKey(WPARAM id);
	INT_PTR OnDisplayChange(WPARAM imageDepth,int cxScreen,int cyScreen);
	INT_PTR OnActivateApp(bool bActivate,DWORD dwThreadID);

	INT_PTR OnNull(LPARAM lParam);

	INT_PTR OnDropFiles(HDROP hDrop);

	INT_PTR OnTimer(int nID,VOID (CALLBACK* TimerProc)(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime));


private:
	int __cdecl SqliteCallback(void* pv,int argc,char** argv,char** column);

private:
	AEditBox*			m_pEdit;
	AThunk				m_SqliteThunk;
	AIndexSqlite*		m_pIndex;

	vector<string>		m_dbs;

	bool				m_bStandalone;	//是否独立于主窗口,根据parent窗口是否为NULL来判断
	HIMC				m_hImc;
};
