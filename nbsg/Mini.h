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
	bool parsePathString(std::string& str);
	void updateMiniPos();

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
private:
	bool				m_bStandalone;	//是否独立于主窗口,根据parent窗口是否为NULL来判断
	std::string			m_index;
	std::vector<std::string>	m_Paths;
	int							m_PathIndex;
	HIMC				m_hImc;
};
