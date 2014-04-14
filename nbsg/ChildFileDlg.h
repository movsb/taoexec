#pragma once
#include "windowbase.h"

class AChildFileDlg:public AWindowBase
{
public:
	AChildFileDlg(AWindowBase* hParentBase,const char* windowTitle,const char* zDir);
	~AChildFileDlg(void);

public:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnNotify(LPNMHDR phdr);
	INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM lParam);
	INT_PTR OnDestroy();
	INT_PTR OnSize(int width,int height);
	INT_PTR OnShowWindow(BOOL bShow,BOOL bCallFromShowWindow);

private:
	struct LPARAM_STRUCT
	{
		char itemPath[MAX_PATH];
		char* pFileName;
	};

private:
	unsigned int __stdcall RefreshProc(void* pv);
	void SetDirectories(const char* zDirs);
	void Refresh();
	void RemoveAll();
	void AddItem(LPARAM_STRUCT* p);

private:
	HANDLE		m_Thread;
	HANDLE		m_Event;
	AListView*	m_pFileList;
	AThunk		m_ThreadThunk;

	char		m_Dirs[MAX_PATH*4];
	HIMAGELIST	m_hImageList;

private:
	BOOL m_bInitialized;
};
