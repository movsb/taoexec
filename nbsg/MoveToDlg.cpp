#include <string>

#include "Button.h"
#include "nbsg.h"
#include "resource.h"
#include "Utils.h"

#include "ChildIndexDlg.h"
#include "SQLite.h"
#include "MoveToDlg.h"

AMoveToDlg::AMoveToDlg(AWindowBase* parent,AChildIndexDlg::LPARAM_STRUCT* *ppls,size_t count,const char* origTableName)
{
	pSettings = new ASettingsSqlite;
	pBtnMoveTo = new AButton;
	pBtnCopyTo = new AButton;
	pBtnCancel = new AButton;
	
	assert(parent != NULL);	//parent 一定不能为NULL,因为表名是指针, 不是数组
	this->SetParent(parent);
	tableName = origTableName;
	m_count = count;
	m_ppls = ppls;

	m_ModalDialogResultCode = DialogBoxParam(g_pApp->getInstance(),MAKEINTRESOURCE(IDD_MOVE_TO_GROUP),this->GetParent()->GetHwnd(),DLGPROC(GetWindowThunk()),0);
}

AMoveToDlg::~AMoveToDlg()
{
	delete pBtnMoveTo;
	delete pBtnCopyTo;
	delete pBtnCancel;
	delete pSettings;
}

INT_PTR AMoveToDlg::DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR AMoveToDlg::OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl)
{
	if(codeNotify == BN_CLICKED){
		if(ctrlID == IDC_MOVETO_CANCEL){
			this->EndDialog(MOVETO_CANCELED);
		}else if(ctrlID==IDC_MOVETO_MOVETO || ctrlID==IDC_MOVETO_COPYTO){
			HWND hList = GetDlgItem(this->GetHwnd(),IDC_LIST_MOVETO);
			int iSel = ListView_GetNextItem(hList,-1,LVNI_SELECTED);
			if(iSel == -1){
				AUtils::msgbox(this->GetHwnd(),MB_ICONEXCLAMATION,"","请先择一个表名项");
				return 0;
			}

			LV_ITEM lvi = {0};
			char table[64]={0};
			lvi.mask = LVIF_TEXT;
			lvi.cchTextMax = 64;
			lvi.pszText = table;
			lvi.iItem = iSel;
			lvi.iSubItem = 0;
			ListView_GetItem(hList,&lvi);
			//AUtils::msgbox(hList,0,"",lvi.pszText);

			AIndexSqlite* is = new AIndexSqlite;
			is->setTableName(table);
			is->attach(this->GetHwnd(),g_pSqliteBase->getPdb());

			AIndexSqlite* ois = new AIndexSqlite;
			ois->setTableName(tableName);
			ois->attach(this->GetHwnd(),g_pSqliteBase->getPdb());

			for(size_t n=0; n<m_count; n++){
				//移动或复制过去算是新增了
				//添加成功会,idx会被修改改新的
				//char* pidx = new char[sizeof(m_ppls[n]->si.idx)];
				//memcpy(pidx,m_ppls[n]->si.idx,sizeof(m_ppls[n]->si.idx));
				//m_ppls[n]->si.idx[0] = '.';
				AIndexSqlite::SQLITE_INDEX si={0};
				si = m_ppls[n]->si;
				si.idx[0]='.';
				if(is->add(&si)){
					if(ctrlID == IDC_MOVETO_MOVETO){
						ois->deleteIndex((char*)m_ppls[n]->si.idx.c_str());
					}
				}else{
					AUtils::msgbox(this->GetHwnd(),MB_ICONERROR,NULL,"失败!");
				}
			}

			delete is;
			delete ois;

			this->EndDialog(ctrlID==IDC_MOVETO_MOVETO?MOVETO_MOVETO:MOVETO_COPYTO);
		}
		return 0;
	}
	return 0;
}

INT_PTR AMoveToDlg::OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam)
{
	m_hWnd = hWnd;

	pSettings->attach(hWnd,g_pSqliteBase->getPdb());

	LVCOLUMN lvc = {0};
	RECT rc;
	HWND hList;
	int listWidth=0;

	char str[128]={0};
	sprintf(str,"将要移动/复制选中的 %d 项 ...",m_count);
	this->SetWindowText(str);

	hList = GetDlgItem(hWnd,IDC_LIST_MOVETO);
	ListView_SetExtendedListViewStyle(hList,ListView_GetExtendedListViewStyle(hList)|LVS_EX_FULLROWSELECT);
	::GetWindowRect(hList,&rc);
	listWidth = rc.right-rc.left-10;

	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	lvc.cchTextMax = 256;

	lvc.iSubItem = 0;
	lvc.pszText = "数据库表名";
	lvc.cx = int(listWidth*0.4);
	ListView_InsertColumn(hList,0,&lvc);

	lvc.iSubItem++;
	lvc.pszText = "标签页名";
	lvc.cx = int(listWidth*0.6);
	ListView_InsertColumn(hList,1,&lvc);
	

	char* index;
	int size;
	if(pSettings->getSetting("index_list",(void**)&index,&size)){
		std::string str(index);
		str += "\r\n";

		LV_ITEM lvi = {0};
		lvi.mask = LVIF_TEXT;
		lvi.cchTextMax = 256;
		lvi.iItem = -1;
		std::string::size_type pos=0,last_pos=0;
		while((pos=str.find_first_of('\n',last_pos))!=std::string::npos){
			if(pos-last_pos==1){
				last_pos = pos+1;
				continue;
			}

			std::string all  = str.substr(last_pos,pos-last_pos);
			all[all.length()-1]='\0';

			std::string::size_type pos_2 = all.find_first_of(',');
			std::string data_base = all.substr(0,pos_2);
			std::string data_name = all.substr(pos_2+1);

			//忽略当前表
			if(strcmp(data_base.c_str(),tableName)!=0){
				lvi.iItem ++;
				lvi.iSubItem = 0;
				lvi.pszText = (char*)data_base.c_str();
				ListView_InsertItem(hList,&lvi);
				lvi.iSubItem = 1;
				lvi.pszText = (char*)data_name.c_str();
				ListView_SetItem(hList,&lvi);
			}

			last_pos = pos+1;
		}
		delete[] index;
	}

	this->CenterWindow(GetParent()->GetHwnd());

	return 0;
}

INT_PTR AMoveToDlg::OnClose()
{
	this->EndDialog(MOVETO_CANCELED);
	return 0;
}
