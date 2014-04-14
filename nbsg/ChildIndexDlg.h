#pragma once
#include "Windowbase.h"
#include "SQLite.h"
class AIndexList;

class AChildIndexDlg:public AWindowBase
{
public:
	AChildIndexDlg(AWindowBase* hParentBase,const char* table,const char* windowTitle);
	~AChildIndexDlg();

private:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnNotify(LPNMHDR phdr);
	INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl);
	INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM lParam);
	INT_PTR OnSize(int width,int height);
	INT_PTR OnDestroy();
	INT_PTR OnDropFiles(HDROP hDrop);
	INT_PTR OnShowWindow(BOOL bShow,BOOL bCallFromShowWindow);

private:
	int __cdecl callback(void* pv, int argc, char** argv, char** column);

public:
	struct LPARAM_STRUCT
	{
		AIndexSqlite::SQLITE_INDEX si;
		bool bFileExisted;
		int iItem;			//如果是修改,记录其所在ListView的iItem
	};


private:
	void SetTableName(const char* zTable);
	const char* GetTableName();
	void SearchAll();
	void RemoveAll();
	void UpdateItem(LPARAM_STRUCT* l);
	BOOL GetItem(LPARAM_STRUCT** ppls);
	void AddItem(LPARAM_STRUCT* l);
	void RemoveItem(int iItem);

private:
	char			m_zTableName[32];
	AListView*		m_pIndexList;
	AIndexSqlite*	m_pIndexSqlite;
	AThunk*			m_pSqliteThunk;
	HIMAGELIST		m_hImageList;

private:
	BOOL m_bInitialized;
};
