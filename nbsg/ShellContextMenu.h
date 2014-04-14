#ifndef __SHELL_CONTEXTMENU_H__
#define __SHELL_CONTEXTMENU_H__

#include <windows.h>
#include <ShObjIdl.h>
#include <ShlObj.h>
#include <malloc.h>

class AShellContextMenu
{
public:
	AShellContextMenu();
	~AShellContextMenu();
	void SetObjects(const wchar_t* strObjects);
	UINT ShowContextMenu(HWND hWnd);
	HMENU GetMenu();

private:
	
	void InvokeCommand(HWND hWnd,LPCONTEXTMENU pContextMenu,UINT idCommand);
	BOOL GetContextMenu(void** ppContextMenu,int* piMenuType);
	HRESULT BindToParentEx(LPCITEMIDLIST pidl,REFIID riid,void** ppv,LPCITEMIDLIST* ppidlLast);
	static LRESULT CALLBACK WindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	void FreePIDLArray(LPITEMIDLIST* pidlArray);
	LPITEMIDLIST CopyPIDL(LPCITEMIDLIST pidl,int cb=-1);
	UINT GetPIDLSize(LPCITEMIDLIST pdil);
	LPBYTE GetPIDLPos(LPCITEMIDLIST pidl,int nPos);
	int GetPIDLCount(LPCITEMIDLIST pidl);
private:
	int nItems;
	BOOL bDelete;
	HMENU m_Menu;
	IShellFolder * m_psfFolder;
	LPITEMIDLIST * m_pidlArray;	
};


#endif//!__SHELL_CONTEXTMENU_H__
