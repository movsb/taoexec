#if _MSC_VER == 1200
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif

#include "SQLite.h"
#include "nbsg.h"
#include "resource.h"
#include "EditBox.h"
#include "Utils.h"
#include "PathLib.h"
#include "tips.h"
#include "MainDlg.h"
#include "Mini.h"

using namespace std;

AMini::AMini(AWindowBase* parent):
	m_bStandalone(parent==NULL),
	m_hImc(0),
	m_pEdit(new AEditBox),
	m_pIndex(new AIndexSqlite)

{
	if(m_bStandalone) AWindowBase::AddWindow(this);
	this->SetParent(parent);
	create();
}

AMini::~AMini(void)
{
	delete m_pEdit;
	delete m_pIndex;
}

void AMini::updateMiniPos()
{
	RECT rc,rcTaskBar={0};
	int cxScreen,cyScreen;
	HWND hTaskBar;
	int task_bar_height=0;
	cxScreen=GetSystemMetrics(SM_CXSCREEN);
	cyScreen=GetSystemMetrics(SM_CYSCREEN);
	this->GetWindowRect(&rc);
	hTaskBar=FindWindow("Shell_TrayWnd",NULL);
	if(hTaskBar){
		::GetWindowRect(hTaskBar,&rcTaskBar);
		if(rcTaskBar.left==0 && rcTaskBar.top!=0){//在下边
			task_bar_height = rcTaskBar.bottom-rcTaskBar.top;
		}
	}
	this->SetWindowPos(HWND_TOPMOST,(cxScreen-(rc.right-rc.left))/2,cyScreen-(rc.bottom-rc.top)-task_bar_height-5,0,0,SWP_NOSIZE);
}

void AMini::BuildPaths()
{
	m_Paths.clear();

	string dir,str;
	str = m_pEdit->GetWindowText();
	dir = str.substr(0,str.find_last_of('\\')+1);

	WIN32_FIND_DATA fd;
	string s = str + "*";

	HANDLE hFile = ::FindFirstFile(s.c_str(),&fd);
	if(hFile != INVALID_HANDLE_VALUE){
		do{
			string file = dir + fd.cFileName;
			if(!(::GetFileAttributes(file.c_str()) & FILE_ATTRIBUTE_HIDDEN)
				&& strcmp(fd.cFileName,".")
				&& strcmp(fd.cFileName,"..")
			){
				m_Paths.push_back(file);
			}
		}while(::FindNextFile(hFile,&fd));
		::FindClose(hFile);
	}
}

bool AMini::parsePathString(std::string& str)
{
	if(str.size()>=3
		&& str[1]==':')
	{
		if(m_pEdit->GetModify()){
			BuildPaths();
			m_PathIndex=m_Paths.size()?0:-1;
			if(m_PathIndex!=-1){
				m_pEdit->SetWindowText(m_Paths[m_PathIndex++].c_str());
				m_pEdit->SetModify(FALSE);
			}
		}else{
			if(m_PathIndex!=-1){
				if(m_PathIndex>m_Paths.size()-1) m_PathIndex=0;
				m_pEdit->SetWindowText(m_Paths[m_PathIndex++].c_str());
			}
		}
		Edit_SetSel(m_pEdit->GetHwnd(),m_pEdit->GetWindowTextLength(),m_pEdit->GetWindowTextLength());
		return true;
	}
	return false;
}

bool AMini::parseCommandString(std::string& str)
{
	const char* p = str.c_str();
	while(*p && isspace(*p)) p++;
	str = str.substr((unsigned int)p-(unsigned int)str.c_str());
	if(!str.size()){
		m_pEdit->SetWindowText("");
		return true;
	}


	if(str == "\\"){
		APathLib::showDir(this->GetHwnd(),g_pApp->getProgramDirectory());
		return true;
	}

	if(parsePathString(str))
		return true;

	string cmd,param;
	string::size_type nspace,nslash;
	bool bviewdir = false;

	nspace = str.find_first_of(' ');
	if(nspace == string::npos){
		nslash = str.find_first_of('\\');
		if(nslash == string::npos){
			cmd = str;
			param = "";
		}else{
			if(nslash == str.length()-1){
				cmd = str.substr(0,str.length()-1);
				bviewdir = true;
				param = "";
			}else{
				cmd = param = "";
			}
		}
	}else{
		cmd = str.substr(0,nspace);
		param = str.substr(nspace+1);
	}

	if(!cmd.size()) return false;

	if(cmd.find('\'')!=string::npos){
		AUtils::msgbox(this->GetHwnd(),MB_ICONEXCLAMATION,NULL,"索引名不可以含有单引号!");
		return false;
	}


	CALLBACK_RESULT cr;
	cr.findstr = cmd;
	cr.found = false;
	for(const char* p=m_index.c_str(); *p;)
	{
		m_pIndex->setTableName(p);
		m_pIndex->search(
			cmd.c_str(),
			reinterpret_cast<void*>(&cr),
			(sqlite3_callback)m_SqliteThunk.Cdeclcall(this,&AMini::SqliteCallback)
			);

		if(cr.found/* || cr.result.size()==1*/){
			break;
		}else{
			while(*p++){
			}
			if(!*p){
				//
				if(cr.result.size()>1){
					show_tips("索引过多~");
				}else{
					show_tips("不是有效的命令或索引名!");
				}
			}
		}
	}

	if(cr.found || cr.result.size()==1){
		bool ret = false;
		//AIndexSqlite::SQLITE_INDEX* p = cr.result[cr.result.size()-1];
		CALLBACK_RESULT_ENTRY* pEntry = cr.result[cr.result.size()-1];
		m_pIndex->setTableName(pEntry->strDatabase.c_str());
		if(param=="") param = pEntry->index.param;
		if(bviewdir){
			ret=APathLib::showDir(this->GetHwnd(),pEntry->index.path.c_str());
		}else{
			ret=APathLib::shellExec(this->GetHwnd(),pEntry->index.path.c_str(),param.c_str(),APathLib::getFileDir(pEntry->index.path.c_str()).c_str());
			ret = m_pIndex->UpdateTimes(pEntry->index.idx.c_str());
		}
		if(ret){
			show_tips((char*)pEntry->index.comment.c_str());
			m_pEdit->SetWindowText("");
			if(m_bStandalone) this->ShowWindow(SW_HIDE);
		}
	}
	
	for(auto it=cr.result.begin();
		it!=cr.result.end();
		++it
		)
	{
		delete *it;
	}
	return true;
}

bool AMini::create()
{
	void* pThunk = m_WndThunk.Stdcall(this,&AMini::WindowProc);
	HWND h=CreateDialogParam(g_pApp->getInstance(),MAKEINTRESOURCE(IDD_MINI),this->GetParent()?this->GetParent()->GetHwnd():NULL,(DLGPROC)pThunk,0);
	assert(h!=NULL);
	//this->ShowWindow(SW_SHOW);
	ShowWindow(m_bStandalone?SW_HIDE:SW_SHOW);
	return true;
}

int __cdecl AMini::SqliteCallback(void* pv,int argc,char** argv,char** column)
{
	CALLBACK_RESULT* presult = reinterpret_cast<CALLBACK_RESULT*>(pv);
	CALLBACK_RESULT_ENTRY* pEntry = new CALLBACK_RESULT_ENTRY;
	m_pIndex->makeIndex(&pEntry->index,const_cast<const char**>(argv));
	pEntry->strDatabase = m_pIndex->getTableName();
	presult->result.push_back(pEntry);
	//if(_stricmp(presult->zIndex,p->index.c_str())==0){
	//if(presult->findstr == pEntry->index.index){
	if(_stricmp(presult->findstr.c_str(),pEntry->index.index.c_str())==0){
		presult->found = true;
		return 1;
	}else{
		return 0;
	}
}

INT_PTR AMini::DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR AMini::OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl)
{
	if(!codeNotify && !hWndCtrl){
		switch(ctrlID)
		{
		case IDM_MINI_MAINDLG:
			{
				new AMainDlg(NULL,true);
				return 0;
			}
		case IDM_MINI_EXIT:
			{
// 				if(g_pWindowManager->m_Windows.size()>1){
// 					AUtils::msgbox(this->GetHwnd(),MB_ICONEXCLAMATION,"提示","你必须关闭所有主窗口才能退出程序!");
// 					return 0;
// 				}
				this->DestroyWindow();
				return 0;
			}
		}
		return 0;
	}
	return 0;
}

INT_PTR AMini::OnNull(LPARAM lParam)
{
	ControlMessage* pcm = reinterpret_cast<ControlMessage*>(lParam);
	if(!pcm) return 0;

	if(pcm->self == m_pEdit){
		if(pcm->uMsg==WM_CHAR){
			if(pcm->wParam==VK_RETURN){
				m_pEdit->EnableWindow(false);
				std::string str = m_pEdit->GetWindowText();
				if(parseCommandString(str)){

				}
				m_pEdit->EnableWindow(true);
				m_pEdit->SetFocus();
				return 0;
			}else if(pcm->wParam == VK_TAB){
				std::string str=m_pEdit->GetWindowText();
				parsePathString(str);
				return 0;
			}
		}else if(pcm->uMsg == WM_CONTEXTMENU){
			if(m_bStandalone){
				HMENU hMain = (HMENU)pcm->lParam;
				AppendMenu(hMain,MF_SEPARATOR,0,NULL);
				AppendMenu(hMain,MF_STRING,2,"切换输入法");
				AppendMenu(hMain,MF_SEPARATOR,0,NULL);
				AppendMenu(hMain,MF_STRING,0,"新建主窗口");
				AppendMenu(hMain,MF_STRING,1,"关闭小窗口");
				return SetDlgResult(TRUE);
			}else{
				HMENU hMain = (HMENU)pcm->lParam;
				AppendMenu(hMain,MF_SEPARATOR,0,NULL);
				AppendMenu(hMain,MF_STRING,0,"新建小窗口");
				return SetDlgResult(TRUE);
			}
		}
		return 0;
	}

	if(pcm->uMsg == WM_CONTEXTMENU){
		if(m_bStandalone){
			if(pcm->lParam == 0){
				AMainDlg* pmain = new AMainDlg(NULL,SW_SHOW==SW_SHOW);
				return 0;
			}else if(pcm->lParam == 1){
				this->DestroyWindow();
				//TODO:delete this;
			}else if(pcm->lParam == 2){
				if(m_hImc){
					ImmAssociateContext(m_pEdit->GetHwnd(),m_hImc);
					m_hImc=0;
				}else{
					m_hImc = ImmAssociateContext(m_pEdit->GetHwnd(),NULL);
				}
			}
		}else{
			if(pcm->lParam == 0){
				AMini* pmini = new AMini(NULL);
				return 0;
			}
		}
		return 0;
	}
	return 0;
}

INT_PTR AMini::OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM lParam)
{
	m_hWnd = hWnd;
	if(m_bStandalone) {
		this->updateMiniPos();
		::SetWindowLongPtr(hWnd,GWL_EXSTYLE,(LONG)(::GetWindowLongPtr(hWnd,GWL_EXSTYLE)|WS_EX_LAYERED));
		::SetLayeredWindowAttributes(hWnd,0,125,LWA_ALPHA);
		::RegisterHotKey(hWnd,0,MOD_CONTROL|MOD_SHIFT,0x5A);
		//TODO:取消注册
	}
	if(m_bStandalone) this->SetStyle(this->GetStyle()&~WS_CHILD|WS_POPUP);
	else			  this->SetStyle(this->GetStyle()&~WS_POPUP|WS_CHILD);

	if(m_bStandalone){

	}else{
		this->SetStyle(this->GetStyle()&~WS_POPUP);
		this->SetStyle(this->GetStyle()|WS_CHILD);
		::SetParent(this->GetHwnd(),this->GetParent()->GetHwnd());
	}

	m_pEdit->attach(this,IDC_MINI_EDIT);
	m_pEdit->SubClass();

	if(m_bStandalone) m_hImc=ImmAssociateContext(m_pEdit->GetHwnd(),NULL);

	m_pIndex->setTableName("nbs");//只是为了不报错误
	m_pIndex->attach(this->GetHwnd(),g_pSqliteBase->getPdb());

	ASettingsSqlite settings;
	char* index=0;
	int  size;
	settings.attach(hWnd,g_pSqliteBase->getPdb());
	if(settings.getSetting("index_list",(void**)&index,&size)){
		try{
			std::string::size_type pos=0,last_pos=0;
			std::string str(index);
			while((pos=str.find_first_of('\n',last_pos))!=std::string::npos){
				if(pos-last_pos==1){
					last_pos = pos+1;
					continue;
				}

				std::string all  = str.substr(last_pos,pos-last_pos);
				std::string data_base = all.substr(0,all.find_first_of(','));

				m_index += data_base;
				m_index += '\0';

				last_pos = pos+1;
			}
			m_index += '\0';//以两个'\0\0'结束


			delete[] index;
		}
		catch(...){
			AUtils::msgbox(this->GetHwnd(),MB_ICONERROR,g_pApp->getAppName(),"索引列表不正确!");
		}
	}

	this->DragAcceptFiles(TRUE);
	m_pEdit->SetFocus();

	return FALSE;
}

INT_PTR AMini::OnDropFiles(HDROP hDrop)
{
	char file[MAX_PATH]={0};
	string str = m_pEdit->GetWindowText();
	str += " ";
	DragQueryFile(hDrop,0,file,sizeof(file));
	str += file;
	m_pEdit->SetWindowText(str.c_str());
	return 0;
}

INT_PTR AMini::OnDestroy()
{
	return 0;
}

INT_PTR AMini::OnNcDestroy()
{
	if(m_bStandalone) AWindowBase::DeleteWindow(this);
	return 0;
}

INT_PTR AMini::OnHotKey(WPARAM id)
{
	switch(id)
	{
	case 0:
		{
			if(::IsWindowVisible(this->GetHwnd())){
				this->ShowWindow(SW_HIDE);
			}else{
				this->ShowWindow(SW_SHOWNORMAL);
				if(::GetForegroundWindow()!=this->GetHwnd()){
					this->SetWindowForeground();
				}
				SetActiveWindow(m_pEdit->GetHwnd());
			}
			return 0;
		}
	}
	return 0;
}

INT_PTR AMini::OnDisplayChange(WPARAM imageDepth,int cxScreen,int cyScreen)
{
	if(m_bStandalone){
		this->updateMiniPos();
	}
	return 0;
}

INT_PTR AMini::OnActivateApp(bool bActivate,DWORD dwThreadID)
{
	if(m_bStandalone){
		if(bActivate){
			KillTimer(this->GetHwnd(),0);
			return 0;
		}else{
			//this->ShowWindow(SW_HIDE);
			//如果5秒钟内没有再次激活窗口的话就自动隐藏
			SetTimer(this->GetHwnd(),0,5000,NULL);
			return 0;
		}
	}
	return 0;
}

INT_PTR AMini::OnSize(int width,int height)
{
	m_pEdit->SetWindowPos(0,0,width,height);
	return 0;
}

INT_PTR AMini::OnTimer(int nID,VOID (CALLBACK* TimerProc)(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime))
{
	if(m_bStandalone){
		if(nID==0){
			this->ShowWindow(SW_HIDE);
			KillTimer(this->GetHwnd(),0);
			return 0;
		}
	}
	return 0;
}
