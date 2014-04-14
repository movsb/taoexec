#include "nbsg.h"
#include "resource.h"
#include "Utils.h"
#include "Button.h"
#include "EditBox.h"
#include "SQLite.h"
#include "Utils.h"
#include "PathLib.h"
#include "reg.h"
#include "plugin.h"

#include "ChildIndexDlg.h"
#include "ChildFileDlg.h"
#include "ChildSettingsDlg.h"


AChildSettingsDlg::AChildSettingsDlg(AWindowBase* hParentBase):
	m_pSettings(new ASettingsSqlite),
	m_pEditWindowTitle(new AEditBox),
	m_pEditIndexList(new AEditBox),
	m_pEditDirList(new AEditBox),
	m_pBtnTopmost(new AButton),
	m_pBtnAutoHide(new AButton),
	m_pBtnAutoRun(new AButton),
	m_pBtnShowDesktop(new AButton),
	m_pEditIcon(new AEditBox),
	m_pBtnIcon(new AButton)
{
	debug_out(("AChildSettingsDlg::AChildSettingsDlg(AWindowBase* hParentBase)\n"));
	//m_AmIADialog = TRUE;
	this->SetParent(hParentBase);
	CreateDialogParam(g_pApp->getInstance(),MAKEINTRESOURCE(IDD_SETTINGS),hParentBase->GetHwnd(),DLGPROC(GetWindowThunk()),0);
}


AChildSettingsDlg::~AChildSettingsDlg(void)
{
	delete m_pSettings;
	delete m_pEditWindowTitle;
	delete m_pEditIndexList;
	delete m_pEditDirList;
	delete m_pEditIcon;
	delete m_pBtnTopmost;
	delete m_pBtnAutoRun;
	delete m_pBtnAutoHide;
	delete m_pBtnShowDesktop;
	delete m_pBtnIcon;
	debug_out(("AChildSettingsDlg::~AChildSettingsDlg()\n"));
}

INT_PTR AChildSettingsDlg::DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR AChildSettingsDlg::OnVScroll(WPARAM wParam,HWND hScrollNull)
{
	int iVertPos;
	SCROLLINFO si = {0};
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	GetScrollInfo(m_hWnd,SB_VERT,&si);
	iVertPos = si.nPos;
	switch(LOWORD(wParam))
	{
	case SB_ENDSCROLL:						break;
	case SB_TOP:		si.nPos=si.nMin;	break;
	case SB_BOTTOM:		si.nPos=si.nMax;	break;
	case SB_LINEUP:		si.nPos--;			break;
	case SB_LINEDOWN:	si.nPos++;			break;
	case SB_PAGEUP:		si.nPos-=si.nPage;	break;
	case SB_PAGEDOWN:	si.nPos+=si.nPage;	break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		si.nPos = si.nTrackPos;break;
	}
	si.fMask = SIF_POS;
	::SetScrollInfo(m_hWnd,SB_VERT,&si,TRUE);
	::GetScrollInfo(m_hWnd,SB_VERT,&si);

	if(si.nPos != iVertPos){
		::ScrollWindow(this->GetHwnd(),0,(iVertPos-si.nPos)*si.nPage,NULL,NULL);
		UpdateWindow(m_hWnd);
	}

	return 0;
}

INT_PTR AChildSettingsDlg::OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl)
{
	if(hWndCtrl == m_pBtnAutoHide->GetHwnd()){
		BOOL bAutoHide = m_pBtnAutoHide->GetCheck()==BST_CHECKED;
		
		ControlMessage cm;
		cm.self = this;
		cm.uMsg = WM_USER+0;
		cm.lParam = bAutoHide;
		this->GetParent()->SendMessage(WM_NULL,0,LPARAM(&cm));
	}else if(hWndCtrl == m_pBtnAutoRun->GetHwnd()){
		bool bRun = m_pBtnAutoRun->GetCheck()==BST_CHECKED;
		AReg run(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run");
		if(bRun){
			if(run){
				char path[260];
				run.SetValue("nbsg",REG_SZ,(BYTE*)path,GetModuleFileName(NULL,path,260)+1);
			}
		}else{
			run.DeleteValue("nbsg");
		}
	}else if(hWndCtrl == m_pBtnTopmost->GetHwnd()){
		bool bTop = m_pBtnTopmost->GetCheck()==BST_CHECKED;
		this->GetParent()->SetWindowPos(bTop?HWND_TOPMOST:HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	}else if(hWndCtrl == m_pBtnIcon->GetHwnd()){
		char file[MAX_PATH]={0};
		if(APathLib::getOpenFileName(
			this->GetHwnd(),
			"选择一个图标资源",
			"图标资源(*.ico;*.exe;*.dll)\x00*.ico;*.exe;*.dll\x00"
			"所有文件(*.*)\x00*.*\x00",
			file)
		)
		{
			string icon(file);
			icon += ",0";
			this->SetAppIcon(icon);
			m_pEditIcon->SetWindowText(icon.c_str());
			m_pEditIcon->SetModify(TRUE);
		}
	}else if(hWndCtrl == m_pEditWindowTitle->GetHwnd()){
		if(codeNotify == EN_KILLFOCUS){
			if(m_pEditWindowTitle->GetModify()){
				this->GetParent()->SetWindowText(m_pEditWindowTitle->GetWindowText().c_str());
			}
		}
	}else if(hWndCtrl == m_pEditIcon->GetHwnd()){
		if(codeNotify == EN_KILLFOCUS){
			if(m_pEditIcon->GetModify()){
				this->SetAppIcon(m_pEditIcon->GetWindowText());
			}
		}
	}
	return 0;
}

INT_PTR AChildSettingsDlg::OnNotify(LPNMHDR phdr)
{
	return 0;
}

INT_PTR AChildSettingsDlg::OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM lParam)
{
	m_hWnd = hWnd;
	this->SetWindowText("设置");
	m_pSettings			->attach(this->GetHwnd(),g_pSqliteBase->getPdb());
	m_pEditWindowTitle	->attach(this,IDC_SETTINGS_TITLE);
	m_pEditIndexList	->attach(this,IDC_SETTINGS_INDEXLIST);
	m_pEditDirList		->attach(this,IDC_SETTINGS_DIRLIST);
	m_pEditIcon			->attach(this,IDC_SETTINGS_ICON);
	m_pBtnTopmost		->attach(this,IDC_SETTINGS_TOPMOST);
	m_pBtnAutoRun		->attach(this,IDC_SETTINGS_AUTORUN);
	m_pBtnAutoHide		->attach(this,IDC_SETTINGS_AUTOHIDE);
	m_pBtnShowDesktop	->attach(this,IDC_SETTINGS_SHOW_DESKTOP);
	m_pBtnIcon			->attach(this,IDC_SETTINGS_ICONBTN);

	m_pBtnIcon			-> SubClass();

	m_pEditWindowTitle	->SubClass();
	m_pEditIcon			->SubClass();
	m_pEditIndexList	->SubClass();
	m_pEditDirList		->SubClass();

	//////////////////////////////////////////////////////////////////////////
	SCROLLINFO si = {0};
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin=0; 
	si.nMax = 99;
	si.nPos=si.nTrackPos=0;
	si.nPage = 10;
	::SetScrollInfo(hWnd,SB_VERT,&si,TRUE);

	//////////////////////////////////////////////////////////////////////////
	int size;

	char* title="";
	if(m_pSettings->getSetting("title",(void**)&title,&size)){
		m_pEditWindowTitle->SetWindowText((const char*)title);
		this->GetParent()->SetWindowText((const char*)title);
		delete[] title;
	}else{
		this->GetParent()->SetWindowText(g_pApp->getAppNameAndVersion());
	}

	char* icon="";
	HICON hIcon;
	if(m_pSettings->getSetting("appicon",(void**)&icon,&size) &&
		(hIcon=APathLib::GetIndexedFileIcon(icon))
	)
	{
		m_pEditIcon->SetWindowText(icon);
		delete[] icon;
		this->GetParent()->SetIcon(ICON_SMALL,hIcon);
	}else{
		m_pEditIcon->SetWindowText(icon);
		this->GetParent()->SetIcon(ICON_SMALL,::LoadIcon(g_pApp->getInstance(),MAKEINTRESOURCE(IDI_ICON1)));
	}

	int* pbTopmost=0;
	if(m_pSettings->getSetting("topmost",(void**)&pbTopmost,&size)){
		m_pBtnTopmost->SetCheck(*pbTopmost?BST_CHECKED:BST_UNCHECKED);
		this->GetParent()->SetWindowPos(*pbTopmost?HWND_TOPMOST:HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		delete[] pbTopmost;
	}

	int* pbAutoHide=0;
	if(m_pSettings->getSetting("autohide",(void**)&pbAutoHide,&size)){
		m_pBtnAutoHide->SetCheck(*pbAutoHide?BST_CHECKED:BST_UNCHECKED);
		ControlMessage cm={0};
		cm.self = this;
		cm.uMsg = WM_USER+0;
		cm.lParam = (LPARAM)*pbAutoHide;
		GetParent()->SendMessage(WM_NULL,0,LPARAM(&cm));
		delete[] pbAutoHide;
	}

	int* pbAutoRun=0;
	if(m_pSettings->getSetting("autorun",(void**)&pbAutoRun,&size)){
		m_pBtnAutoRun->SetCheck(*pbAutoRun?BST_CHECKED:BST_UNCHECKED);
		delete[] pbAutoRun;
	}

	int* bShowDesktop;
	if(m_pSettings->getSetting("show_desktop",(void**)&bShowDesktop,&size)){
		bool bShow = *bShowDesktop!=0;     
		if(bShow){
			std::string desk_user = APathLib::getSpecialFolder(CSIDL_COMMON_DESKTOPDIRECTORY);
			std::string desk_all  = APathLib::getSpecialFolder(CSIDL_DESKTOPDIRECTORY);

			std::string all = desk_user + ";" + desk_all;
			
			AChildFileDlg* pfile = new AChildFileDlg(this->GetParent(),"桌面",all.c_str());

			ControlMessage cm = {0};
			cm.self = NULL;
			cm.uMsg = WM_USER+1;
			cm.wParam = (WPARAM)pfile;
			cm.lParam = LPARAM("桌面");
			GetParent()->SendMessage(WM_NULL,0,LPARAM(&cm));
		}
		m_pBtnShowDesktop->SetCheck(bShow?BST_CHECKED:BST_UNCHECKED);
		delete[] bShowDesktop;
	}else{
		std::string desk_user = APathLib::getSpecialFolder(CSIDL_COMMON_DESKTOPDIRECTORY);
		std::string desk_all  = APathLib::getSpecialFolder(CSIDL_DESKTOPDIRECTORY);

		std::string all = desk_user + ";" + desk_all;

		AChildFileDlg* pfile = new AChildFileDlg(this->GetParent(),"桌面",all.c_str());
		
		ControlMessage cm = {0};
		cm.self = NULL;
		cm.uMsg = WM_USER+1;
		cm.wParam = (WPARAM)pfile;
		cm.lParam = LPARAM("桌面");
		GetParent()->SendMessage(WM_NULL,0,LPARAM(&cm));

		m_pBtnShowDesktop->SetCheck(BST_CHECKED);
	}

	char* index;
	if(m_pSettings->getSetting("index_list",(void**)&index,&size)){
		std::string str(index);
		str += "\r\n";
		m_pEditIndexList->SetWindowText(index);
		try{
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

				AChildIndexDlg* pindex = new AChildIndexDlg(this->GetParent(),data_base.c_str(),data_name.c_str());

				ControlMessage cm = {0};
				cm.self = NULL;
				cm.uMsg = WM_USER+1;
				cm.wParam = (WPARAM)pindex;
				cm.lParam = LPARAM(data_name.c_str());
				GetParent()->SendMessage(WM_NULL,0,LPARAM(&cm));

				last_pos = pos+1;
			}
			delete[] index;
		}
		catch(...){
			AUtils::msgbox(this->GetHwnd(),MB_ICONERROR,g_pApp->getAppName(),"索引列表不正确!");
		}
	}

	char* dir;
	if(m_pSettings->getSetting("dir_list",(void**)&dir,&size)){
		std::string str(dir);
		str += "\r\n";
		m_pEditDirList->SetWindowText(dir);
		try{
			std::string::size_type pos=0,last_pos=0;
			while((pos=str.find_first_of('\n',last_pos))!=std::string::npos){
				if(pos-last_pos==1){
					last_pos = pos+1;
					continue;
				}

				std::string all  = str.substr(last_pos,pos-last_pos);
				all[all.length()-1]='\0';

				std::string::size_type pos_2 = all.find_first_of(',');
				std::string dir_name = all.substr(0,pos_2);
				std::string dir_path = all.substr(pos_2+1);

				AChildFileDlg* pfile = new AChildFileDlg(this->GetParent(),dir_name.c_str(),dir_path.c_str());

				ControlMessage cm = {0};
				cm.self = NULL;
				cm.uMsg = WM_USER+1;
				cm.wParam = (WPARAM)pfile;
				cm.lParam = LPARAM(dir_name.c_str());
				GetParent()->SendMessage(WM_NULL,0,LPARAM(&cm));

				last_pos = pos+1;
			}
			delete[] dir;
		}
		catch(...){
			AUtils::msgbox(this->GetHwnd(),MB_ICONERROR,g_pApp->getAppName(),"目录列表不正确!");
		}
	}

	//加载所有插件从这里开始
	//内存没有释放,怎么办?
	APlugin::PLUGINS plugins;
	APlugin::FindAllPlugins(&plugins);
	if(plugins.size()){
		for(APlugin::PLUGINS_IT it=plugins.begin(); it!=plugins.end(); it++){
			APlugin* pp = new APlugin;
			pp->Load(it->c_str());
			pp->npInit(this->GetParent());
			pp->npAbout();
		}
	}

	return FALSE;

}

INT_PTR AChildSettingsDlg::OnDestroy()
{
	if(m_pEditWindowTitle->GetModify()){
		std::string title = m_pEditWindowTitle->GetWindowText();
		m_pSettings->setSetting("title",(void*)title.c_str(),title.length()+1);
	}
	if(m_pEditIndexList->GetModify()){
		std::string index = m_pEditIndexList->GetWindowText();
		m_pSettings->setSetting("index_list",(void*)index.c_str(),index.length()+1);
	}
	if(m_pEditDirList->GetModify()){
		std::string dir = m_pEditDirList->GetWindowText();
		m_pSettings->setSetting("dir_list",(void*)dir.c_str(),dir.length()+1);
	}
	if(m_pEditIcon->GetModify()){
		std::string icon = m_pEditIcon->GetWindowText();
		m_pSettings->setSetting("appicon",(void*)icon.c_str(),icon.length()+1);
	}

	int bTopmost  = m_pBtnTopmost->GetCheck()==BST_CHECKED;
	m_pSettings->setSetting("topmost",(void*)&bTopmost,4);
	int bAutoHide = m_pBtnAutoHide->GetCheck()==BST_CHECKED;
	m_pSettings->setSetting("autohide",(void*)&bAutoHide,4);
	int bAutoRun  = m_pBtnAutoRun->GetCheck()==BST_CHECKED;
	m_pSettings->setSetting("autorun",(void*)&bAutoRun,4);
	int bShowDesktop = m_pBtnShowDesktop->GetCheck()==BST_CHECKED;
	m_pSettings->setSetting("show_desktop",(void*)&bShowDesktop,4);

	return 0;
}


void AChildSettingsDlg::SetAppIcon(const string& icon)
{
	HICON hIcon=APathLib::GetIndexedFileIcon(icon.c_str());
	if(hIcon){
		this->GetParent()->SetIcon(ICON_SMALL,hIcon);
	}
}

INT_PTR AChildSettingsDlg::OnNull(LPARAM lParam)
{
	ControlMessage* pcm = reinterpret_cast<ControlMessage*>(lParam);
	if(!pcm) return 0;

	if(pcm->uMsg==WM_CHAR){
		if(pcm->wParam == VK_RETURN){
			if(pcm->self == m_pEditWindowTitle){
				GetParent()->SetWindowText(m_pEditWindowTitle->GetWindowText().c_str());
				return 0;
			}else if(pcm->self == m_pEditIcon){
				SetAppIcon(m_pEditIcon->GetWindowText().c_str());
			}else{
				return pcm->self->DoDefault(pcm);
			}
		}else if(pcm->wParam == VK_TAB){
			if(pcm->self == m_pEditWindowTitle
				|| pcm->self == m_pEditIcon
				|| pcm->self == m_pEditIndexList
				|| pcm->self == m_pEditDirList)
			{
				return 0;
			}else{
				return pcm->self->DoDefault(pcm);
			}
		}
	}
	return 0;
}