#include "StdAfx.h"

using namespace std;
using namespace DuiLib;

#include "AddDlg.h"
#include "PathLib.h"
#include "SQLite.h"
#include "Mini.h"
#include "tips.h"
#include "Utils.h"
 
#include "res/resource.h"

class CMiniImpl : public WindowImplBase
{
public:
	CMiniImpl(CSQLite* db)
	{
		m_db = db;
	}
protected:
	virtual CDuiString GetSkinFolder()
	{
		return "skin/";
	}
	virtual CDuiString GetSkinFile()
	{
		return "MiniDlg.xml";
	}
	virtual LPCTSTR GetWindowClassName(void) const
	{
		return "女孩不哭";
	}
	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam)
	{
		return FALSE;
	}

	virtual void InitWindow();
	virtual void OnFinalMessage( HWND hWnd )
	{
		__super::OnFinalMessage(hWnd);
		delete this;
	}
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnReturn(TNotifyUI& msg);
	virtual void OnMenu(TNotifyUI& msg);
	virtual void OnTimer(TNotifyUI& msg);

private:
	bool handleCommand(string str);
	bool runShell(string cmd,string arg);
	bool findCmd(const string& cmd,vector<CIndexItem*>* R);

private:
	void updatePosition();

private:
	CSQLite*	m_db;
	CRichEditUI* m_rich;
};

DUI_BEGIN_MESSAGE_MAP(CMiniImpl,WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_RETURN,OnReturn)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_MENU,OnMenu)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_TIMER,OnTimer)
DUI_END_MESSAGE_MAP()

CMini::CMini(CSQLite* db)
{
	CMiniImpl* pMini = new CMiniImpl(db);
	pMini->Create(nullptr,"MINIWND",WS_POPUP,WS_EX_TOPMOST|WS_EX_TOOLWINDOW);
	pMini->ShowWindow(true);
}
CMini::~CMini()
{

}

void CMiniImpl::OnReturn(TNotifyUI& msg)
{
	if(msg.pSender == m_rich){
		CDuiString str = m_rich->GetText();
		if(handleCommand(string(str))){
			m_rich->SetText("");
		}
	}
}

void CMiniImpl::OnTimer(TNotifyUI& msg)
{
	if(msg.pSender == m_rich){
		
	}
}

void CMiniImpl::OnMenu(TNotifyUI& msg)
{
	if(msg.pSender == m_rich){
		HMENU hMenu = ::LoadMenu(CPaintManagerUI::GetInstance(),MAKEINTRESOURCE(IDR_MENU_EDITBOX));
		HMENU hMenuSub = ::GetSubMenu(hMenu,0);
		::ClientToScreen(GetHWND(),&msg.ptMouse);
		UINT id = ::TrackPopupMenu(hMenuSub,TPM_NONOTIFY|TPM_RETURNCMD|TPM_LEFTBUTTON,msg.ptMouse.x,msg.ptMouse.y,0,GetHWND(),nullptr);
		switch(id)
		{
		default:
			break;
		}
		::DestroyMenu(hMenu);
		return;
	}
}

void CMiniImpl::InitWindow()
{
	m_rich = static_cast<CRichEditUI*>(m_PaintManager.FindControl("command"));
	ImmAssociateContext(GetHWND(),NULL);
	updatePosition();
	::RegisterHotKey(GetHWND(),0,MOD_CONTROL|MOD_SHIFT,'Z');
}

bool CMiniImpl::runShell(string cmd,string arg)
{
	int rv = (int)::ShellExecute(GetHWND(),"open",cmd.c_str(),arg.c_str(),nullptr,SW_SHOWNORMAL);
	if(rv >= 32) return true;
	else{
		AUtils::msgerr(GetHWND(),"");
		return false;
	}
}

bool CMiniImpl::handleCommand(string str)
{
	const char* p = str.c_str();
	while(*p && isspace(*p)) p++;

	if(p != str.c_str())
		str = str.substr((unsigned int)p-(unsigned int)str.c_str());

	if(!str.size()){
		return true;
	}

	bool bshellrun=false;
	if(str[0]=='!'){
		str=str.substr(1);
		bshellrun = true;
	}

	bool bmodify=false;
	if(str[0]=='@'){
		str=str.substr(1);
		bmodify = true;
	}

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

	if(bshellrun){
		return runShell(cmd,param);
	}

	if(cmd.find('\'')!=string::npos){
		MessageBox(GetHWND(),"索引名不可以含有单引号!",nullptr,MB_ICONERROR);
		return false;
	}

	if(bshellrun){
		return APathLib::shellExec(GetHWND(),cmd.c_str(),param.c_str(),nullptr);
	}

	vector<CIndexItem*> R;
	bool found=false;
	m_db->QueryIndices(cmd.c_str(),&R,&found);


	if(!found){
		if(R.size()>1){
			ShowTips("索引过多~");
			m_db->FreeVectoredIndexItems(&R);
			return false;
		}else if(R.size()==0){
			ShowTips("未找到该索引~");
			m_db->FreeVectoredIndexItems(&R);
			return false;
		}
	}

	auto item = R[R.size()-1];
	bool ret = false;

	if(param=="") param = item->param;
	if(bviewdir){
		ret = APathLib::showDir(GetHWND(),item->path.c_str());
	}
	else if(bmodify){
		CAddDlg dlg(GetHWND(),CAddDlg::TYPE_MODIFY,item,m_db);
	}
	else{
		ret = APathLib::shellExec(GetHWND(),item->path.c_str(),item->param.c_str(),0);
		ret = ret && m_db->UpdateTimes(item);
		if(ret){
			ShowTips(item->comment.c_str());
		}
	}

	m_db->FreeVectoredIndexItems(&R);

	return ret;
}

void CMiniImpl::updatePosition()
{
	CDuiRect rc;
	if(!::SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0))
		return;

	SIZE sz = m_PaintManager.GetClientSize();
	
	::SetWindowPos(GetHWND(), 0, 
		(rc.GetWidth()-sz.cx)/2, rc.GetHeight()-sz.cy-10,
		0, 0,
		SWP_NOSIZE|SWP_NOZORDER|(::IsWindowVisible(GetHWND())?SWP_SHOWWINDOW:SWP_HIDEWINDOW)
		);
}

LRESULT CMiniImpl::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch(uMsg)
	{
	case WM_DISPLAYCHANGE:
		{
			updatePosition();
			break;
		}
	case WM_TIMER:
		{
			if(wParam == 256){
				ShowWindow(false);
				::KillTimer(GetHWND(),256);
				bHandled = TRUE;
				return 0;
			}
			break;
		}
	case WM_ACTIVATEAPP:
		{
			if(wParam == TRUE){
				::KillTimer(GetHWND(),256);
			}else{
				::SetTimer(GetHWND(),256,5000,nullptr);
			}
			bHandled = TRUE;
			return 0;
		}
	case WM_HOTKEY:
		{
			if(wParam == 0){
				if(::IsWindowVisible(GetHWND())){
					ShowWindow(false);
					::KillTimer(GetHWND(),256);
				}else{
					ShowWindow(true,true);
					m_rich->SetFocus();
				}
				bHandled = TRUE;
				return 0;
			}
			break;
		}
	}
	bHandled = FALSE;
	return 0;
}
