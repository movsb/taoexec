#include "ShellContextMenu.h"


#define MENU_ID_MIN		1
#define MENU_ID_MAX		10000

IContextMenu2* g_IContextMenu2 = NULL;
IContextMenu3* g_IContextMenu3 = NULL;

WNDPROC OldWndProc = NULL;

AShellContextMenu::AShellContextMenu()
{
	m_psfFolder = NULL;
	m_pidlArray = NULL;
	m_Menu = NULL;
}

AShellContextMenu::~AShellContextMenu()
{
	if(m_psfFolder && bDelete){
		m_psfFolder->Release();
	}
	m_psfFolder = NULL;
	FreePIDLArray(m_pidlArray);
	m_pidlArray = NULL;

	if(m_Menu) DestroyMenu(m_Menu);
}

HMENU AShellContextMenu::GetMenu()
{
	if(!m_Menu) m_Menu = ::CreatePopupMenu();
	return m_Menu;
}

BOOL AShellContextMenu::GetContextMenu(void** ppContextMenu,int* piMenuType)
{
	*ppContextMenu = NULL;
	LPCONTEXTMENU pcm = NULL;

	m_psfFolder->GetUIObjectOf(NULL,nItems,(LPCITEMIDLIST*)m_pidlArray,IID_IContextMenu,NULL,(void**)&pcm);

	if(pcm){
		if(pcm->QueryInterface(IID_IContextMenu3,ppContextMenu) == NOERROR){
			*piMenuType = 3;
		}else if(pcm->QueryInterface(IID_IContextMenu2,ppContextMenu) == NOERROR){
			*piMenuType = 2;
		}

		if(*ppContextMenu){
			pcm->Release();
		}else{
			*piMenuType = 1;
			*ppContextMenu = pcm;
		}
		return TRUE;
	}else{
		return FALSE;
	}
}

LRESULT CALLBACK AShellContextMenu::WindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_MENUCHAR:
		if(g_IContextMenu3){
			LRESULT result = 0;
			g_IContextMenu3->HandleMenuMsg2(uMsg,wParam,lParam,&result);
			return result;
		}
		break;
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
		if(wParam){
			break;
		}
		//fallthrough

	case WM_INITMENUPOPUP:
		if(g_IContextMenu2){
			g_IContextMenu2->HandleMenuMsg(uMsg,wParam,lParam);
		}else{
			g_IContextMenu3->HandleMenuMsg(uMsg,wParam,lParam);
		}
		return uMsg==WM_INITMENUPOPUP?0:TRUE;
	default:
		break;
	}
	return ::CallWindowProc(OldWndProc,hWnd,uMsg,wParam,lParam);
}

UINT AShellContextMenu::ShowContextMenu(HWND hWnd)
{
	int iMenuType = 0;
	LPCONTEXTMENU pContextMenu = NULL;

	if(!GetContextMenu((void**)&pContextMenu,&iMenuType))
		return 0;

	if(!m_Menu) m_Menu = ::CreatePopupMenu();

	pContextMenu->QueryContextMenu(m_Menu,2,MENU_ID_MIN,MENU_ID_MAX,CMF_NORMAL|CMF_EXPLORE);

	if(iMenuType > 1){
		OldWndProc = (WNDPROC)SetWindowLong(hWnd,GWL_WNDPROC,(LONG)WindowProc);
		if(iMenuType == 2){
			g_IContextMenu2 = (LPCONTEXTMENU2)pContextMenu;
		}else{
			g_IContextMenu3 = (LPCONTEXTMENU3)pContextMenu;
		}
	}else{
		OldWndProc = 0;
	}

	POINT pt;
	GetCursorPos(&pt);

	UINT idCommand = ::TrackPopupMenu(m_Menu,TPM_RETURNCMD|TPM_LEFTALIGN,pt.x,pt.y,0,hWnd,NULL);

	if(OldWndProc) SetWindowLong(hWnd,GWL_WNDPROC,(LONG)OldWndProc);

	if(idCommand>=MENU_ID_MIN && idCommand<= MENU_ID_MAX){
		InvokeCommand(hWnd,pContextMenu,idCommand-MENU_ID_MIN);
		idCommand = 0;
	}

	pContextMenu->Release();
	g_IContextMenu2 = NULL;
	g_IContextMenu3 = NULL;

	return idCommand;
}

void AShellContextMenu::InvokeCommand(HWND hWnd,LPCONTEXTMENU pContextMenu,UINT idCommand)
{
	CMINVOKECOMMANDINFO cmi = {0};
	cmi.cbSize = sizeof(cmi);
	cmi.hwnd = hWnd;
	cmi.lpVerb = (LPCSTR)MAKEINTRESOURCE(idCommand);
	cmi.nShow = SW_SHOWNORMAL;

	HRESULT h = pContextMenu->InvokeCommand(&cmi);
}

void AShellContextMenu::SetObjects(const wchar_t* strObjects)
{
	if(m_psfFolder && bDelete){
		m_psfFolder->Release();
	}		
	m_psfFolder = NULL;
	FreePIDLArray(m_pidlArray);
	m_pidlArray = NULL;

	IShellFolder* psfDesktop = NULL;
	SHGetDesktopFolder(&psfDesktop);

	LPITEMIDLIST pidl = NULL;

	psfDesktop->ParseDisplayName(NULL,0,(LPWSTR)strObjects,NULL,&pidl,NULL);

	//LPITEMIDLIST pidlItem = NULL;
	BindToParentEx(pidl,IID_IShellFolder,(void**)&m_psfFolder,NULL);
	//free(pidlItem);

	LPMALLOC lpMalloc = NULL;
	SHGetMalloc(&lpMalloc);
	lpMalloc->Free(pidl);

	IShellFolder* psfFolder = NULL;
	//nItems = 1;

	LPITEMIDLIST pidlItem = NULL;

	nItems = 0;
	for(const wchar_t* p=strObjects;*p!=L'\0';p++){
		nItems ++;
		psfDesktop->ParseDisplayName(NULL,0,(LPWSTR)p,NULL,&pidl,NULL);
		m_pidlArray = (LPITEMIDLIST*)realloc(m_pidlArray,nItems*sizeof(LPITEMIDLIST));
		BindToParentEx(pidl,IID_IShellFolder,(void**)&psfFolder,(LPCITEMIDLIST*)&pidlItem);
		m_pidlArray[nItems-1] = CopyPIDL(pidlItem);
		free(pidlItem);
		lpMalloc->Free(pidl);
		psfFolder->Release();

		while(*p) p++;
	}

	lpMalloc->Release();
	psfDesktop->Release();

	bDelete = TRUE;
}

void AShellContextMenu::FreePIDLArray(LPITEMIDLIST* pidlArray)
{
	if(!pidlArray) return;

	int iSize = _msize(pidlArray)/sizeof(LPITEMIDLIST);

	for(int i=0; i<iSize; i++){
		free(pidlArray[i]);
	}
	free(pidlArray);
}

LPITEMIDLIST AShellContextMenu::CopyPIDL(LPCITEMIDLIST pidl,int cb/* =-1 */)
{
	if(cb == -1) cb = GetPIDLSize(pidl);

	LPITEMIDLIST pidlRet = (LPITEMIDLIST)calloc(cb + sizeof(USHORT),sizeof(BYTE));
	if(pidlRet) memcpy(pidlRet,pidl,cb);

	return pidlRet;
}

UINT AShellContextMenu::GetPIDLSize(LPCITEMIDLIST pidl)
{  
	if (!pidl) 
		return 0;
	int nSize = 0;
	LPITEMIDLIST pidlTemp = (LPITEMIDLIST) pidl;
	while (pidlTemp->mkid.cb)
	{
		nSize += pidlTemp->mkid.cb;
		pidlTemp = (LPITEMIDLIST) (((LPBYTE) pidlTemp) + pidlTemp->mkid.cb);
	}
	return nSize;
}

HRESULT AShellContextMenu::BindToParentEx(LPCITEMIDLIST pidl,REFIID riid,void** ppv,LPCITEMIDLIST* ppidlLast)
{
	HRESULT hr = 0;
	if (!pidl || !ppv)
		return E_POINTER;

	int nCount = GetPIDLCount (pidl);
	if (nCount == 0)	// desktop pidl of invalid pidl
		return E_POINTER;

	IShellFolder * psfDesktop = NULL;
	SHGetDesktopFolder (&psfDesktop);
	if (nCount == 1)	// desktop pidl
	{
		if ((hr = psfDesktop->QueryInterface(riid, ppv)) == S_OK)
		{
			if (ppidlLast) 
				*ppidlLast = CopyPIDL(pidl);
		}
		psfDesktop->Release ();
		return hr;
	}

	LPBYTE pRel = GetPIDLPos (pidl, nCount - 1);
	LPITEMIDLIST pidlParent = NULL;
	pidlParent = CopyPIDL(pidl, pRel - (LPBYTE) pidl);
	IShellFolder * psfFolder = NULL;

	if ((hr = psfDesktop->BindToObject (pidlParent, NULL, __uuidof (psfFolder), (void **) &psfFolder)) != S_OK)
	{
		free (pidlParent);
		psfDesktop->Release ();
		return hr;
	}
	if ((hr = psfFolder->QueryInterface (riid, ppv)) == S_OK)
	{
		if (ppidlLast)
			*ppidlLast = CopyPIDL ((LPCITEMIDLIST) pRel);
	}
	free (pidlParent);
	psfFolder->Release ();
	psfDesktop->Release ();
	return hr;
}

LPBYTE AShellContextMenu::GetPIDLPos (LPCITEMIDLIST pidl, int nPos)
{
	if (!pidl)
		return 0;
	int nCount = 0;

	BYTE * pCur = (BYTE *) pidl;
	while (((LPCITEMIDLIST) pCur)->mkid.cb)
	{
		if (nCount == nPos)
			return pCur;
		nCount++;
		pCur += ((LPCITEMIDLIST) pCur)->mkid.cb;	// + sizeof(pidl->mkid.cb);
	}
	if (nCount == nPos) 
		return pCur;
	return NULL;
}

int AShellContextMenu::GetPIDLCount (LPCITEMIDLIST pidl)
{
	if (!pidl)
		return 0;

	int nCount = 0;
	BYTE*  pCur = (BYTE *) pidl;
	while (((LPCITEMIDLIST) pCur)->mkid.cb)
	{
		nCount++;
		pCur += ((LPCITEMIDLIST) pCur)->mkid.cb;
	}
	return nCount;
}
