#if _MSC_VER == 1200
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif

#include "ChildFileDlg.h"
#include "nbsg.h"
#include "resource.h"
#include "ListView.h"
#include "Utils.h"
#include "PathLib.h"
#include "Str.h"
#include "AddDlg.h"

#include <process.h>
#include <tchar.h>
#include "tips.h"

#include "ShellContextMenu.h"

AChildFileDlg::AChildFileDlg(AWindowBase* hParentBase,const char* windowTitle,const char* zDir):
	m_pFileList(new AListView)
{
	m_hImageList = NULL;
	m_bInitialized = FALSE;

	this->SetParent(hParentBase);
	this->SetDirectories(zDir);

	void* pThunk = m_WndThunk.Stdcall(this,&AWindowBase::WindowProc);
	CreateDialogParam(g_pApp->getInstance(),MAKEINTRESOURCE(IDCD_TEMPLATE),hParentBase->GetHwnd(),DLGPROC(pThunk),0);

	this->SetWindowText(windowTitle);
}


AChildFileDlg::~AChildFileDlg(void)
{
	delete m_pFileList;
}

void AChildFileDlg::SetDirectories(const char* zDirs)
{
	assert(zDirs != NULL);
	strncpy(m_Dirs,zDirs,sizeof(m_Dirs)/sizeof(*m_Dirs));

	memset(m_Dirs,0,sizeof(m_Dirs));
	_tcsncpy(m_Dirs,zDirs,sizeof(m_Dirs)/sizeof(*m_Dirs)-1);//必须保证以两个'\0'结束
	for(char* p=m_Dirs; *p ; p++){
		if(*p == ';'){
			*p = '\0';
		}
	}
}

//不知道为什么,刷新的时候listview闪烁严重,所以先隐藏
void AChildFileDlg::Refresh()
{
	debug_out(("DirRefresh:%s\n",m_Dirs));

	ShowWindow(SW_HIDE);

	RemoveAll();
	if(m_hImageList != NULL){
		ImageList_Destroy(m_hImageList);
	}
	m_hImageList = ImageList_Create(32,32,ILC_COLOR32,0,1);
	m_pFileList->SetImageList(m_hImageList,LVSIL_NORMAL);

	//遍历每一个目录
	for(const char* one_dir=m_Dirs;*one_dir;){
		debug_out(("dir:%s\n",one_dir));
		if(APathLib::isFileExists(one_dir) == false){
			AUtils::msgbox(this->GetHwnd(),MB_ICONASTERISK,"Sorry...","目录未设定或不存在!\nm_Directory:%s",one_dir);
			return;
		}

		WIN32_FIND_DATA fd;
		string findstr;
		string directory = one_dir;
		HANDLE hFind = INVALID_HANDLE_VALUE;

		if(directory[directory.size()-1] != '\\') directory += '\\';
		findstr =  directory + "*.*";
		hFind = FindFirstFile(findstr.c_str(),&fd);
		if(hFind != INVALID_HANDLE_VALUE){
			do{
				string a = directory + fd.cFileName;
				DWORD attr = GetFileAttributes(a.c_str());
				if(attr & FILE_ATTRIBUTE_DIRECTORY){
					if(*fd.cFileName == '.'){//. or ..
						continue;
					}
					if(attr & FILE_ATTRIBUTE_HIDDEN){
						continue;
					}
				}else if(attr & FILE_ATTRIBUTE_HIDDEN){
					continue;
				}

				LPARAM_STRUCT* p = new LPARAM_STRUCT;
				strcpy(p->itemPath,a.c_str());
				p->pFileName = const_cast<char*>(strrchr(p->itemPath,'\\')+1);
				AddItem(p);
			}while(FindNextFile(hFind,&fd));
			FindClose(hFind);
		}

		//for 3rd
		while(*one_dir){
			one_dir++;
		}
		//定位到下一个字符串的开始
		one_dir++;
	}
	ShowWindow(SW_SHOW);
	//扯淡,为什么隐藏/显示后,listview的内容不自动显示????
	//RECT rc;
	//m_pFileList->GetClientRect(&rc);
	//GetClientRect(&rc);
	//m_pFileList->SetWindowPos(0,0,0,rc.right-rc.left-1,rc.bottom-rc.top-1,SWP_NOZORDER|SWP_NOMOVE);
	//m_pFileList->SetWindowPos(0,0,0,rc.right-rc.left,rc.bottom-rc.top,SWP_NOZORDER|SWP_NOMOVE);
	
}

void AChildFileDlg::RemoveAll()
{
	int count = m_pFileList->GetItemCount();
	for(int i=0; i<count; i++){
		LPARAM_STRUCT* p = reinterpret_cast<LPARAM_STRUCT*>(m_pFileList->GetItemLParam(i));
		delete p;
	}
	m_pFileList->DeleteAllItems();
}

unsigned int __stdcall AChildFileDlg::RefreshProc(void* pv)
{
	//Refresh();

	//get dir count
	int count_of_dirs=0;
	const char** dir_list = NULL;
	HANDLE* handles = NULL;

	const char* dirs = m_Dirs;
	for(const char* p=dirs;*p;){
		count_of_dirs++;
		while(*p) p++;
		p++;
	}

	dir_list = new const char*[count_of_dirs];
	handles  = new HANDLE[count_of_dirs+1];

	int i=0;
	for(const char* pp=dirs; *pp; ){
		dir_list[i++] = pp;
		while(*pp) pp++;
		pp++;
	}

	handles[0] = m_Event;

	unsigned int sehcode = 0;
	__try{
		for(int i=0; i<count_of_dirs; i++){
			handles[1+i] = FindFirstChangeNotification(
				dir_list[i],
				FALSE,
				FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME
				);
			if(handles[1+i] == INVALID_HANDLE_VALUE){
				sehcode = 1;
				__leave;
			}
		}

		do{
			DWORD dwRet = WaitForMultipleObjects(count_of_dirs+1,handles,FALSE,INFINITE);
			if(dwRet == WAIT_FAILED){
				AUtils::msgerr(this->GetHwnd(),"WaitForMultiObjects(...) failed");
				__leave;
			}else if(dwRet == WAIT_OBJECT_0+0){
				debug_out(("AChildFileDlg::RefreshProc:WAIT_OBJECT_0+0\n"));
				for(int i=0; i<count_of_dirs; i++){
					FindCloseChangeNotification(handles[1+i]);
				}
				__leave;
			}else{
				debug_out(("AChildFileDlg::RefreshProc:WAIT_OBJECT_0+\?\n"));
				Refresh();
				Sleep(3000);
				FindNextChangeNotification(handles[dwRet-WAIT_OBJECT_0]);
				//为防止刷新过于频繁 - 貌似并不管用
			}
		}while(true);
	}
	__finally{
		delete[] dir_list;
		delete[] handles;
	}
	return sehcode;
}

void AChildFileDlg::AddItem(LPARAM_STRUCT* l)
{
	LV_ITEM lvi = {0};
	HICON hIcon;
	lvi.mask = LVIF_IMAGE|LVIF_PARAM|LVIF_TEXT;
	lvi.iItem = 0x7FFFFFFF;
	lvi.pszText = l->pFileName;//如果是快捷方式的话, 我们只需要显示其名称(不包含.lnk),在下面进行替换
	string real_name;

	if(::GetFileAttributes(l->itemPath) & FILE_ATTRIBUTE_DIRECTORY){
		hIcon = APathLib::getFileIcon(l->itemPath);
	}else{
		bool use_abs = false;
		string lcase(l->itemPath);
		for(string::size_type st=0; st<lcase.length(); st++){
			if(lcase[st]>='A' && lcase[st]<='Z'){
				lcase[st] |= 0x20;
			}
		}
		if(strstr(lcase.c_str(),".lnk")){
			string absolute;
			string desc,args;
			if(APathLib::ParseLnk(AStr(l->itemPath,false).toWchar(),absolute,args,desc)){
				hIcon = APathLib::getFileIcon(absolute.c_str());
				use_abs = true;
			}
			real_name = l->pFileName;
			real_name = real_name.substr(0,real_name.find_last_of('.'));
			lvi.pszText = const_cast<char*>(real_name.c_str());
		}
		if(!use_abs){
			hIcon = APathLib::getFileIcon(l->itemPath);
		}
	}

	
	lvi.iImage = ImageList_AddIcon(m_hImageList,hIcon);
	lvi.lParam = reinterpret_cast<LPARAM>(l);

	m_pFileList->InsertItem(&lvi);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 


INT_PTR AChildFileDlg::DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR AChildFileDlg::OnNotify(LPNMHDR phdr)
{
	if(phdr->idFrom == m_pFileList->GetCtrlID()){
		switch (phdr->code)
		{
		case NM_DBLCLK:
		case NM_RETURN:
			{
				LPARAM_STRUCT* p;
				int isel = m_pFileList->GetNextItem(-1,LVNI_SELECTED);
				if(isel == -1){
					for(const char* one_dir=m_Dirs; *one_dir;){
						APathLib::showDir(this->GetHwnd(),one_dir);
						while(*one_dir){
							one_dir++;
						}
						one_dir++;
					}
					return 0;
				}
				p = (LPARAM_STRUCT*)m_pFileList->GetItemLParam(isel);
				if(p){
					if(APathLib::shellExec(this->GetHwnd(),p->itemPath,NULL,NULL)){
						//TODO:HIDE
						show_tips(p->pFileName);
					}
				}
				return 0;
			}
		case NM_RCLICK:
			{
				LPARAM_STRUCT* p = NULL;
				size_t sitem;

				size_t count=0;
				
				sitem = sizeof(p->itemPath);

				for(int ii=-1; (ii=m_pFileList->GetNextItem(ii,LVNI_SELECTED))!=-1;){
					count ++;
				}

				if(count==0) return 0;

				char* parray = new char[sitem*count+1];
				char* ppp = parray;

				memset(parray,0,sitem*count+1);

				for(int i=-1,k=0; (i=m_pFileList->GetNextItem(i,LVNI_SELECTED))!=-1;){
					p = (LPARAM_STRUCT*)m_pFileList->GetItemLParam(i);
					size_t s = strlen(p->itemPath);
					memcpy(ppp,p->itemPath,s);
					ppp += s;
					*ppp++ = '|';
				}
				*ppp++ = '\0';

				AStr s(parray,false);
				const wchar_t* ws = s.toWchar();
				for(wchar_t* pw=(wchar_t*)ws;*pw;pw++){
					if(*pw == L'|'){
						*pw = L'\0';
					}
				}

				for(char* pa=parray;*pa;pa++){
					if(*pa=='|') *pa = '\0';
				}

				AShellContextMenu scm;
				scm.SetObjects(ws);

				HMENU hMenu = scm.GetMenu();
				AppendMenu(hMenu,MF_BYPOSITION|MF_STRING,10001,"添加到索引(&I)");
				AppendMenu(hMenu,MF_SEPARATOR,0,NULL);

				UINT idCommand = scm.ShowContextMenu(this->GetParent()->GetHwnd());
				if(idCommand){
					if(idCommand == 10001){
						CAddDlg add(this->GetHwnd(),NULL,CAddDlg::TYPE_PATH,LPARAM(parray));
					}
				}

				RemoveMenu(hMenu,0,MF_BYPOSITION);

				delete[] parray;

				return 0;
			}
		}
	}
	return 0;
}

INT_PTR AChildFileDlg::OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM lParam)
{
	m_hWnd = hWnd;

	this->ShowWindow(SW_HIDE);

	m_pFileList->attach(this,IDCD_TEMPLATE_LIST);

	RECT rc;
	this->GetClientRect(&rc);
	m_pFileList->SetWindowPos(rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top);

	m_Event = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_Thread = (HANDLE)_beginthreadex(NULL,0,
		(unsigned int (__stdcall*)(void*))m_ThreadThunk.Stdcall(this,&AChildFileDlg::RefreshProc),
		NULL,0,NULL);
	assert(m_Event!=NULL && m_Thread!=NULL);
	return FALSE;
}

INT_PTR AChildFileDlg::OnDestroy()
{
	::SetEvent(m_Event);
	::WaitForSingleObject(m_Thread,1500);
	::CloseHandle(m_Event);
	::CloseHandle(m_Thread);
	RemoveAll();
	return 0;
}

INT_PTR AChildFileDlg::OnSize(int width,int height)
{
	m_pFileList->SetWindowPos(0,0,width,height);
	return 0;
}

INT_PTR AChildFileDlg::OnShowWindow(BOOL bShow,BOOL bCallFromShowWindow)
{
	if(!m_bInitialized && bShow && bCallFromShowWindow){
		m_bInitialized = TRUE;
		Refresh();
	}
	return SetDlgResult(0);
}
