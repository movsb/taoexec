#include "ChildIndexDlg.h"
#include "ListView.h"
#include "EditBox.h"
#include "Button.h"
#include "nbsg.h"
#include "resource.h"
#include "AddDlg.h"
#include <tchar.h>
#include "PathLib.h"
#include "Str.h"
#include "tips.h"
#include "Utils.h"
#include "MoveToDlg.h"

AChildIndexDlg::AChildIndexDlg(AWindowBase* hParentBase,const char* table,const char* windowTitle):
	m_pSqliteThunk(new AThunk),
	m_pIndexList(new AListView),
	m_pIndexSqlite(new AIndexSqlite)
{
	m_hImageList = NULL;
	m_bInitialized = FALSE;

	this->SetParent(hParentBase);
	this->SetTableName(table);

	void* pThunk = m_WndThunk.Stdcall(this,&AChildIndexDlg::WindowProc);
	CreateDialogParam(g_pApp->getInstance(),MAKEINTRESOURCE(IDCD_TEMPLATE),hParentBase->GetHwnd(),DLGPROC(pThunk),0);

	this->SetWindowText(windowTitle);
}

AChildIndexDlg::~AChildIndexDlg()
{
	delete m_pIndexList;
	delete m_pIndexSqlite;
	delete m_pSqliteThunk;
}

void AChildIndexDlg::SetTableName(const char* zTable)
{
	_tcsncpy(m_zTableName,zTable,sizeof(m_zTableName)/sizeof(*m_zTableName));
}

const char* AChildIndexDlg::GetTableName()
{
	return m_zTableName;
}

void AChildIndexDlg::SearchAll()
{
	debug_out(("SearchAll:%s\n",m_zTableName));
	if(this->m_hImageList) ImageList_Destroy(m_hImageList);
	m_hImageList = ImageList_Create(32,32,ILC_COLOR32,0,1);
	m_pIndexList->SetImageList(m_hImageList,LVSIL_NORMAL);
	assert(m_hImageList != NULL);

	RemoveAll();

	void* pThunk = m_pSqliteThunk->Cdeclcall(this,&AChildIndexDlg::callback);
	m_pIndexSqlite->search("*",NULL,(sqlite3_callback)pThunk);
}
//deprecated
void AChildIndexDlg::UpdateItem(LPARAM_STRUCT* l)
{
	AIndexSqlite::SQLITE_INDEX* psi = &l->si;
	int iItem = l->iItem;
	
	m_pIndexList->SetItemText(iItem,0,psi->comment.c_str());
	m_pIndexList->SetItemText(iItem,1,psi->idx.c_str());
	m_pIndexList->SetItemText(iItem,2,psi->index.c_str());
	m_pIndexList->SetItemText(iItem,3,psi->path.c_str());
	m_pIndexList->SetItemText(iItem,4,psi->param.c_str());
	m_pIndexList->SetItemText(iItem,5,psi->times.c_str());
}

BOOL AChildIndexDlg::GetItem(LPARAM_STRUCT** ppls)
{
	LV_ITEM lvi = {0};
	int isel = m_pIndexList->GetNextItem(-1,LVNI_SELECTED);
	if(isel == -1){return FALSE;}
	lvi.mask = LVIF_PARAM;
	lvi.iItem = isel;
	m_pIndexList->GetItem(&lvi);
	*ppls = reinterpret_cast<LPARAM_STRUCT*>(lvi.lParam);
	(*ppls)->iItem = lvi.iItem;
	return TRUE;
}

void AChildIndexDlg::AddItem(LPARAM_STRUCT* l)
{
	LV_ITEM lvi = {0};
	int i=0;
	HICON hIcon;
	AIndexSqlite::SQLITE_INDEX* psi = &l->si;

	if(l->si.path[0] == '{'){
		hIcon = APathLib::GetClsidIcon(l->si.path);
	}else{
		hIcon = APathLib::getFileIcon(l->si.path.c_str());
	}

	if(!hIcon) hIcon=(HICON)this->GetParent()->SendMessage(WM_GETICON,ICON_SMALL);

	lvi.lParam = (LPARAM)l;
	lvi.mask = LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
	lvi.iItem = 0x7FFFFFFF;
	lvi.iSubItem = i++;
	lvi.pszText = (LPTSTR)psi->comment.c_str();
	lvi.iImage = ImageList_AddIcon(m_hImageList,hIcon);
	lvi.iItem = m_pIndexList->InsertItem(&lvi);
	l->iItem = lvi.iItem;

// 	lvi.mask &= ~(LVIF_PARAM|LVIF_IMAGE);
// 
// 	lvi.iSubItem = i++;
// 	lvi.pszText = psi->idx;
// 	ListView_SetItem(_hWnd,&lvi);
// 
// 	lvi.iSubItem = i++;
// 	lvi.pszText = psi->index;
// 	ListView_SetItem(_hWnd,&lvi);
// 
// 	lvi.iSubItem = i++;
// 	lvi.pszText = psi->path;
// 	ListView_SetItem(_hWnd,&lvi);
// 
// 	lvi.iSubItem = i++;
// 	lvi.pszText = psi->param;
// 	ListView_SetItem(_hWnd,&lvi);
// 
// 	lvi.iSubItem = i++;
// 	lvi.pszText = psi->times;
// 	ListView_SetItem(_hWnd,&lvi);
}

void AChildIndexDlg::RemoveItem(int iItem)
{
	int count = m_pIndexList->GetItemCount();
	assert(iItem < count);

	void* p = (void*)m_pIndexList->GetItemLParam(iItem);
	delete p;
	m_pIndexList->DeleteItem(iItem);
}

void AChildIndexDlg::RemoveAll()
{
	int nItems = m_pIndexList->GetItemCount();
	for(int it=0; it<nItems; it++){
		LPARAM_STRUCT* pls = reinterpret_cast<LPARAM_STRUCT*>(m_pIndexList->GetItemLParam(it));
		delete pls;
	}
	m_pIndexList->DeleteAllItems();
}

int __cdecl AChildIndexDlg::callback(void* pv, int argc, char** argv, char** column)
{
	LPARAM_STRUCT* l = new LPARAM_STRUCT;

	l->si.idx    = argv[0];						//idx
	l->si.index  = AStr(argv[1],true).toAnsi();	//index
	l->si.comment= AStr(argv[2],true).toAnsi();	//comment
	l->si.path   = AStr(argv[3],true).toAnsi();	//path
	l->si.param  = AStr(argv[4],true).toAnsi();	//param
	l->si.times  = argv[5];						//times



	if(l->si.path[0] == '{'){//如果是CLS_GUID
		l->bFileExisted = true;
	}else{
		l->bFileExisted = APathLib::isFileExists(l->si.path.c_str());
	}
	AddItem(l);
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 


INT_PTR AChildIndexDlg::DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR AChildIndexDlg::OnNotify(LPNMHDR phdr)
{
	if(phdr->idFrom == m_pIndexList->GetCtrlID())
	{
		switch(phdr->code)
		{
		case NM_CUSTOMDRAW:
			{
				LPNMLVCUSTOMDRAW plcd = (LPNMLVCUSTOMDRAW)phdr;
				switch(plcd->nmcd.dwDrawStage)
				{
				case CDDS_PREPAINT:
					return SetDlgResult(CDRF_NOTIFYITEMDRAW);
				case CDDS_ITEMPREPAINT:
					{
						LPARAM_STRUCT* pls = reinterpret_cast<LPARAM_STRUCT*>(plcd->nmcd.lItemlParam);
						if(pls->bFileExisted){
							plcd->clrText = RGB(0,0,0);
							plcd->clrTextBk = RGB(255,255,255);
						}else{
							plcd->clrText = RGB(255,0,0);
							plcd->clrTextBk = RGB(255,255,255);
						}
						return SetDlgResult(CDRF_NEWFONT);
					}
				}
				return SetDlgResult(CDRF_DODEFAULT);
			}
		case NM_DBLCLK:
		case NM_RETURN:
			{
				LPARAM_STRUCT* pls=NULL; 
				int isel = m_pIndexList->GetNextItem(-1,LVNI_SELECTED);
				if(isel == -1) return 0;
				pls = (LPARAM_STRUCT*)m_pIndexList->GetItemLParam(isel);

				if(APathLib::shellExec(this->GetHwnd(),pls->si.path.c_str(),pls->si.param.c_str(),APathLib::getFileDir(pls->si.path.c_str()).c_str())){
					show_tips((char*)pls->si.comment.c_str());
					
					m_pIndexSqlite->UpdateTimes(pls->si.idx.c_str());
				}
				return 0;
			}
		case NM_RCLICK:
			{
				HMENU hMenu       = NULL;
				HMENU hMenuIndex  = NULL;
				HMENU hMenuView   = NULL;
				HMENU hMenuPlugin = NULL;

				hMenu       = LoadMenu(g_pApp->getInstance(), MAKEINTRESOURCE(IDM_MENU_MAIN));
				hMenu       = GetSubMenu(hMenu, 0);
				hMenuIndex  = GetSubMenu(hMenu,0);
				hMenuView   = GetSubMenu(hMenu,1);
				hMenuPlugin = GetSubMenu(hMenu,2);

				int isel = m_pIndexList->GetNextItem(-1,LVNI_SELECTED)==-1?0:1;
				EnableMenuItem(hMenuIndex,	IDM_INDEX_MODIFY,	isel?MF_ENABLED:MF_GRAYED);
				//EnableMenuItem(hMenuIndex,	IDM_INDEX_REMOVE,	isel?MF_ENABLED:MF_GRAYED);
				EnableMenuItem(hMenuIndex,	IDM_INDEX_MOVETO,	isel?MF_ENABLED:MF_GRAYED);

				EnableMenuItem(hMenuView,	IDM_VIEW_COPYDIR,	isel?MF_ENABLED:MF_GRAYED);
				EnableMenuItem(hMenuView,	IDM_VIEW_COPYPARAM,	isel?MF_ENABLED:MF_GRAYED);
				EnableMenuItem(hMenuView,	IDM_VIEW_DETAIL,	isel?MF_ENABLED:MF_GRAYED);
				EnableMenuItem(hMenuView,	IDM_VIEW_DIR,		isel?MF_ENABLED:MF_GRAYED);

				POINT pt;
				GetCursorPos(&pt);
				TrackPopupMenu(hMenu,TPM_LEFTALIGN,pt.x,pt.y,0,this->GetHwnd(),NULL);
				return 0;
			}
		default:
			return 0;
		}
	}
	return 0;
}

INT_PTR AChildIndexDlg::OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl)
{
	if(!codeNotify && !hWndCtrl){//menu
		switch(ctrlID)
		{
		//Index Options
		case IDM_INDEX_ADD:
			{
				CAddDlg adlg(this->GetParent()->GetHwnd(),this->GetTableName(),CAddDlg::TYPE_NEW,0);
				SearchAll();
				return 0;
			}
		case IDM_INDEX_MODIFY:
			{
				LPARAM_STRUCT* pls = 0;
				if(!m_pIndexList->GetSelectedItemLParam((LPARAM*)&pls)) return 0;
				CAddDlg adlg(this->GetParent()->GetHwnd(),this->GetTableName(),CAddDlg::TYPE_MODIFY,LPARAM(&pls->si));
				SearchAll();
				return 0;
			}

		case IDM_INDEX_REMOVE_MULTI:
			{
				int i,count,count_selected=0;
				//int* p_items = NULL;
				//int iii=0;
				count = m_pIndexList->GetItemCount();
				//p_items = new int[count];

				for(i=-1; (i=m_pIndexList->GetNextItem(i,LVNI_SELECTED))!=-1;){
					count_selected++;
					//p_items[iii++] = i;
				}

				if(count==0 || count_selected==0){
					AUtils::msgbox(this->GetHwnd(),MB_ICONEXCLAMATION,g_pApp->getAppName(),"没有选中任何项!");
					//delete []p_items;
					return 0;						 
				}else{
					int ret;
					ret = AUtils::msgbox(this->GetHwnd(),MB_ICONQUESTION|MB_OKCANCEL,g_pApp->getAppName(),"确定要移除这 %u 项么\?",count_selected);
					if(ret == IDCANCEL) return 0;
					for(i=-1; (i=m_pIndexList->GetNextItem(i,LVNI_SELECTED))!=-1;){
					//for(i=0; i<count_selected; i++){
						LPARAM_STRUCT* pls = reinterpret_cast<LPARAM_STRUCT*>(m_pIndexList->GetItemLParam(i));
						if(m_pIndexSqlite->deleteIndex(pls->si.idx.c_str())){
							m_pIndexList->DeleteItem(i);
							delete pls;
							i--; //删除某项后后面的项会向前面移动,所以从当前删除项的前一项(不包含)开始索引
						}
					}
					//delete[] p_items;
				}
				return 0;
			}
		case IDM_INDEX_MOVETO:
			{
				int count;
				int i;
				LPARAM_STRUCT* *ppls = NULL;
				
				count = m_pIndexList->GetItemCount();
				ppls = new LPARAM_STRUCT*[count];
				
				count = 0;
				for(i=-1; (i=m_pIndexList->GetNextItem(i,LVNI_SELECTED))!=-1;){
					ppls[count++] = reinterpret_cast<LPARAM_STRUCT*>(m_pIndexList->GetItemLParam(i));
				}
				if(count==0){
					delete[] ppls;
					return 0;
				}

				CMoveToDlg moveto(this->GetHwnd(),ppls,count,m_pIndexSqlite->getTableName());

				delete[] ppls;

				if(moveto.GetDlgCode()!=CMoveToDlg::MOVETO_CANCELED) SearchAll();

				return 0;
			}
		case IDM_INDEX_UPDATE:
			{
				RemoveAll();
				SearchAll();
				return 0;
			}
		//View Options
		case IDM_VIEW_DIR:
			{
				LPARAM_STRUCT* pls = NULL;
				if(m_pIndexList->GetSelectedItemLParam((LPARAM*)&pls)){
					if(pls->si.path[0] == '{'){
						if(pls->si.path.size()==38){
							HKEY hKey;
							if(RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Regedit",0,KEY_SET_VALUE,&hKey) == ERROR_SUCCESS){
								string str("HKEY_CLASSES_ROOT\\CLSID\\");
								str += pls->si.path;
								if(RegSetValueEx(hKey,"LastKey",0,REG_SZ,(BYTE*)str.c_str(),str.length()+1) == ERROR_SUCCESS){
									ShellExecute(NULL,"open","regedit",NULL,NULL,SW_SHOWNORMAL);
								}
								RegCloseKey(hKey);
							}
							return 0;
						}else{
							AUtils::msgbox(this->GetHwnd(),MB_ICONEXCLAMATION,NULL,"不合法的GUID!");
							return 0;
						}
					}
					APathLib::showDir(this->GetHwnd(),pls->si.path.c_str());
				}
				return 0;
			}
		case IDM_VIEW_DETAIL:
			{
				string detail(2048,0);
				LPARAM_STRUCT* pls = NULL;
				if(!m_pIndexList->GetSelectedItemLParam((LPARAM*)&pls)) return 0;

				detail = "索引序号: ";
				detail += pls->si.idx;
				detail += "\n索引名称: ";
				detail += pls->si.index;
				detail += "\n索引说明: ";
				detail += pls->si.comment;
				detail += "\n文件路径: ";
				detail += pls->si.path;
				detail += "\n默认参数: ";
				detail += pls->si.param;
				detail += "\n使用次数: ";
				detail += pls->si.times;

				//有'%'要错
				//AUtils::msgbox(this->GetHwnd(),MB_OK,g_pApp->getAppName(),detail.c_str());
				MessageBox(detail.c_str(),g_pApp->getAppName(),MB_OK);
				return 0;
			}
		case IDM_VIEW_COPYPARAM:
		case IDM_VIEW_COPYDIR:
			{
				LPARAM_STRUCT* pls = NULL;
				if(!m_pIndexList->GetSelectedItemLParam((LPARAM*)&pls)) return 0;
				
				if(AUtils::setClipData(ctrlID==IDM_VIEW_COPYPARAM?pls->si.param.c_str():pls->si.path.c_str())){
					AUtils::msgbox(this->GetHwnd(),MB_OK,g_pApp->getAppName(),"%s",ctrlID==IDM_VIEW_COPYPARAM?(pls->si.param.size()?pls->si.param.c_str():"(没有内容)"):pls->si.path.c_str());
				}else{
					AUtils::msgerr(this->GetHwnd(),"失败");
				}
				return 0;
			}
		}
	}else{

	}
	return 0;
}

INT_PTR AChildIndexDlg::OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM lParam)
{
	m_hWnd = hWnd;

	this->ShowWindow(SW_HIDE);

	m_pIndexList->AttachCtrl(this,IDCD_TEMPLATE_LIST);
	m_pIndexList->SetExtendedListViewStyle(m_pIndexList->GetExtendedListViewStyle()|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	m_pIndexList->SetStyle(m_pIndexList->GetStyle()&~LVS_TYPEMASK|LVS_ICON);

	m_pIndexSqlite->setTableName(m_zTableName);
	m_pIndexSqlite->attach(this->GetHwnd(),g_pSqliteBase->getPdb());

	this->DragAcceptFiles(TRUE);
	
	//SearchAll();

	return FALSE;
}

INT_PTR AChildIndexDlg::OnSize(int width,int height)
{
	m_pIndexList->SetWindowPos(0,0,width,height);
	return 0;
}

INT_PTR AChildIndexDlg::OnShowWindow(BOOL bShow,BOOL bCallFromShowWindow)
{
	if(!m_bInitialized && bShow && bCallFromShowWindow){
		m_bInitialized = TRUE;
		SearchAll();
	}
	return SetDlgResult(0);
}

INT_PTR AChildIndexDlg::OnDestroy()
{
	return 0;
}

INT_PTR AChildIndexDlg::OnDropFiles(HDROP hDrop)
{
	//判断是否被置于软件图标上并决定是否用该软件打开该文件
	LVHITTESTINFO hti = {0};
	GetCursorPos(&hti.pt);
	ScreenToClient(m_pIndexList->GetHwnd(),&hti.pt);
	hti.flags = LVHT_ONITEM;
	int iItem = m_pIndexList->HitTest(&hti);

	UINT count = DragQueryFile(hDrop,0xFFFFFFFF,NULL,0);
	char file[MAX_PATH]={0};
	DragQueryFile(hDrop,0,file,sizeof(file));

	if(iItem != -1){//落在了图标上
		LPARAM_STRUCT * param = (LPARAM_STRUCT*)m_pIndexList->GetItemLParam(iItem);
		string str(file);
		if(count > 1) str += " 等多个文件";

		int ret = AUtils::msgbox(this->GetHwnd(),MB_ICONQUESTION|MB_YESNOCANCEL,
			"请选择一个操作~",
			"文件:\t%s\n\n"
			"[是]\t将该文件添加为索引\n"
			"[否]\t用 %s 打开它(不带默认参数)",
			str.c_str(),
			param->si.comment
			);

		if(ret == IDCANCEL) return 0;
		if(ret == IDNO){
			string files("");
			char a[MAX_PATH]={0};
			for(unsigned int x=0; x<count; ++x){
				DragQueryFile(hDrop,x,a,sizeof(a));
				files += a;
				files += " ";
			}
			debug_out(("files:%s\n",files.c_str()));
			APathLib::shellExec(this->GetHwnd(),param->si.path.c_str(),files.c_str(),NULL);
			return 0;
		}else{
			//fallthrough
		}
	}
	
	for(UINT i=0; i<count; i++){
		char file[MAX_PATH]={0};
		DragQueryFile(hDrop,i,file,sizeof(file));
		CAddDlg adlg(this->GetHwnd(),this->GetTableName(),CAddDlg::TYPE_PATH,LPARAM(file));
	}
	SearchAll();
	return 0;
}
