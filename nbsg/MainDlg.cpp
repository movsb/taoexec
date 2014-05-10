#include <string>
#include <vector>
#include <sstream>
#include <windows.h>

using namespace std;

#include <UIlib.h>

using namespace DuiLib;

#include "res/resource.h"

#include "Utils.h"
#include "SQLite.h"
#include "PathLib.h"
#include "Except.h"

#include "AddDlg.h"
#include "MainDlg.h"
#include "InputBox.h"

class CMouseWheelOptionUI : public COptionUI
{
public:
	virtual LPCTSTR GetClass() const override
	{
		return _T("MouseWheelOptionUI");
	}
	virtual LPVOID GetInterface(LPCTSTR pstrName) override
	{
		if( _tcscmp(pstrName, _T("MouseWheelOption")) == 0 ) return this;
		return __super::GetInterface(pstrName);
	}
	virtual void DoEvent(TEventUI& event) override
	{
		if(IsMouseEnabled() && event.Type>UIEVENT__MOUSEBEGIN && event.Type<UIEVENT__MOUSEEND)
		{
			if(event.Type == UIEVENT_SCROLLWHEEL){
				TNotifyUI msg;
				msg.pSender = this;
				msg.sType = DUI_MSGTYPE_SCROLL;
				msg.wParam = event.wParam;
				msg.lParam = event.lParam;
				GetManager()->SendNotify(msg);
				return;
			}
		}
		return __super::DoEvent(event);
	}
};

class CIconButtonUI : public CButtonUI
{
public:
	CIconButtonUI():
		m_hIcon(0)
	{

	}
	~CIconButtonUI()
	{
		::DestroyIcon(m_hIcon);
	}

	virtual LPCTSTR GetClass() const
	{
		return _T("IconButtonUI");
	}
	virtual LPVOID GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, "IconButton") == 0 ) return this;
		return __super::GetInterface(pstrName);
	}
	virtual UINT GetControlFlags() const
	{
		return (IsKeyboardEnabled() ? UIFLAG_TABSTOP : 0) | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
	}

	virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if(_tcscmp(pstrName,_T("path"))==0) m_strPath = pstrValue;

		else return __super::SetAttribute(pstrName,pstrValue);
	}

	void PaintText(HDC hDC)
	{

	}
	void PaintStatusImage(HDC hDC)
	{

		if(m_hIcon==NULL){
			m_hIcon = APathLib::getFileIcon(m_strPath);
			if(!m_hIcon)
				m_hIcon = APathLib::GetClsidIcon(m_strPath.GetData());
			if(!m_hIcon)
				m_hIcon = (HICON)INVALID_HANDLE_VALUE;
		}
		if(m_hIcon == (HICON)INVALID_HANDLE_VALUE)
			return;
		::DrawIconEx(hDC,m_rcPaint.left,m_rcPaint.top,m_hIcon,GetFixedWidth(),GetFixedHeight(),0,nullptr,DI_NORMAL);
		return __super::PaintStatusImage(hDC);

	}
private:
	HICON m_hIcon;
	CDuiString m_strPath;
};

class CIndexListUI : public CTileLayoutUI,public IDialogBuilderCallback
{
private:
	virtual CControlUI* CreateControl(LPCTSTR pstrClass)
	{
		if(_tcscmp(pstrClass,"IconButton")==0) return new CIconButtonUI;
		return nullptr;
	}

public:
	CIndexListUI(CSQLite* db,const char* cat,CPaintManagerUI& pm)
	{
		SetItemSize(CSize(80,80));
		CDialogBuilder builder;
		SetManager(&pm,0,false);
		auto pContainer = static_cast<CContainerUI*>(builder.Create("ListItem.xml",0,this));
		assert(pContainer);

		db->QueryCategory(cat,&m_iiVec);

		int i=0;
		auto s = m_iiVec.begin();
		auto e = m_iiVec.end();

		for(; s != e;
			s++,i++){
			if(pContainer==NULL) pContainer = static_cast<CContainerUI*>(builder.Create(this));
			if(pContainer != NULL){
				auto pBtn = (CButtonUI*)static_cast<CHorizontalLayoutUI*>(pContainer->GetItemAt(0))->GetItemAt(0);
				auto pTxt = pContainer->GetItemAt(1);
				pBtn->SetAttribute("path",(*s)->path.c_str());
				pBtn->SetTag(i);
				pTxt->SetText((*s)->comment.c_str());
				Add(pContainer);
				pContainer = nullptr;
			}
		}
		SetAttribute("vscrollbar","true");
		//SetAttribute("vscrollbarstyle",GetVerticalScrollBar());
	}

	~CIndexListUI()
	{
		auto s = m_iiVec.begin();
		auto e = m_iiVec.end();
		for(; s != e; s ++){
			delete *s;
		}
	}

	void RenameIndexItemsCategory(const char* to)
	{
		for(auto s=m_iiVec.begin(),e=m_iiVec.end(); s!=e; ++s){
			(*s)->category = to;
		}
	}

	vector<CIndexItem*> m_iiVec;
};

class CMainDlgImpl : public WindowImplBase
{
public:
	CMainDlgImpl(CSQLite* db)
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
		return "MainDlg.xml";
	}
	virtual LPCTSTR GetWindowClassName(void) const
	{
		return "女孩不哭";
	}

	virtual void InitWindow();
	virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual void OnFinalMessage( HWND hWnd )
	{
		__super::OnFinalMessage(hWnd);
		delete this;
	}

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);
	virtual void OnSelectChanged(TNotifyUI& msg);
	virtual void OnTimer(TNotifyUI& msg);
	virtual void OnMenu(TNotifyUI& msg);
	virtual void Notify(TNotifyUI& msg)
	{
		m_PaintManager.FindControl("title")->SetText(msg.sType);
		return __super::Notify(msg);
	}
	virtual void OnScroll(TNotifyUI& msg);

private:
	bool addTab(const char* name,int tag,const char* category);

private:
	CSQLite*		m_db;

	CHorizontalLayoutUI*	m_pTabList;
	CTabLayoutUI*			m_pTabPage;

	CButtonUI* m_pbtnClose;
	CButtonUI* m_pbtnMin;
	CButtonUI* m_pbtnRestore;
	CButtonUI* m_pbtnMax;

	vector<CIndexListUI*>			m_listVec;
	vector<CMouseWheelOptionUI*>	m_optVec;
};

CMainDlg::CMainDlg(CSQLite* db)
{
	CMainDlgImpl* pFrame = new CMainDlgImpl(db);
	pFrame->Create(NULL,"Software Manager",UI_WNDSTYLE_FRAME|WS_SIZEBOX,WS_EX_WINDOWEDGE);
	pFrame->CenterWindow();
	pFrame->ShowWindow(true);
}

CMainDlg::~CMainDlg()
{

}

DUI_BEGIN_MESSAGE_MAP(CMainDlgImpl, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED,OnSelectChanged)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_TIMER,OnTimer)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_MENU,OnMenu)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_SCROLL,OnScroll)
DUI_END_MESSAGE_MAP()

void CMainDlgImpl::OnClick(TNotifyUI& msg)
{
	if(msg.pSender->GetName() == "listItemBtn"){
		int tag = msg.pSender->GetTag();
		auto pList = m_listVec[m_pTabPage->GetCurSel()];
		auto elem = pList->m_iiVec[tag];
		APathLib::shellExec(GetHWND(),elem->path.c_str(),elem->param.c_str(),0);
	}


	CDuiString n = msg.pSender->GetName();
	if(n == "closebtn"){
		Close(0);
		return;
	}
	else if(n == "minbtn"){
		SendMessage(WM_SYSCOMMAND,SC_MINIMIZE);
	}
	else if(n == "maxbtn"){
		SendMessage(WM_SYSCOMMAND,SC_MAXIMIZE);
	}
	else if(n == "restorebtn"){
		SendMessage(WM_SYSCOMMAND,SC_RESTORE);
	}
}

void CMainDlgImpl::OnSelectChanged(TNotifyUI& msg)
{
	if(msg.pSender->GetUserData() == "index_list_option"){
		auto pOpt = static_cast<CMouseWheelOptionUI*>(msg.pSender);
		m_pTabPage->SelectItem(m_listVec[pOpt->GetTag()]);
	}
}

void CMainDlgImpl::OnTimer(TNotifyUI& msg)
{
	//MessageBox(GetHWND(),0,0,0);
}

void CMainDlgImpl::OnMenu(TNotifyUI& msg)
{
	if(msg.pSender->GetUserData() == "index_list_option"){
		auto pOpt = static_cast<CMouseWheelOptionUI*>(msg.pSender);
		HMENU hMenu = ::LoadMenu(CPaintManagerUI::GetInstance(),MAKEINTRESOURCE(IDM_TABMENU)); //TODO:destroy
		HMENU hSub0 = ::GetSubMenu(hMenu,0);
		::ClientToScreen(GetHWND(),&msg.ptMouse);
		UINT id = (UINT)::TrackPopupMenu(hSub0,TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,msg.ptMouse.x,msg.ptMouse.y,0,GetHWND(),nullptr);
		if(id==0) return;
		if(id == MENU_TAB_RENAME){
			class CRenameCallback : public IInputBoxCallback
			{
			private:
				virtual bool CheckReturn(LPCTSTR str,LPCTSTR* msg) override
				{
					if(_tcschr(str,_T('\''))){
						*msg = _T("新名字中不能包含单引号字符!");
						return false;
					}
					
					return true;
				}
			};
			CRenameCallback rcb;
			CInputBox input(GetHWND(),
				_T("重命名"),
				_T("请输入新名字"),
				pOpt->GetText(),
				&rcb);
			if(rcb.GetDlgCode() == CInputBox::kCancel || rcb.GetDlgCode()== CInputBox::kClose)
				return;
			
			try{
				m_db->RenameCategory(pOpt->GetText(),rcb.GetStr());
				pOpt->SetText(rcb.GetStr());
				m_listVec[pOpt->GetTag()]->RenameIndexItemsCategory(rcb.GetStr());
			}
			catch(CExcept* e)
			{
				::MessageBox(GetHWND(),e->desc.c_str(),nullptr,MB_ICONEXCLAMATION);
			}
			return;
		}
		else{

		}
	}
}

void CMainDlgImpl::OnScroll(TNotifyUI& msg)
{
	if(msg.pSender->GetUserData() == "index_list_option"){
		CMouseWheelOptionUI* pOptSelected=0;
		for(auto s=m_optVec.begin(),e=m_optVec.end(); s!=e; ++s){
			if((*s)->IsSelected()){
				pOptSelected = *s;
				break;
			}
		}
		if(!pOptSelected) return;

		int count = m_listVec.size();
		int sel = m_pTabList->GetItemIndex(pOptSelected);
		if(msg.wParam == SB_LINEUP){
			sel--;
			if(sel<0)
				sel = count-1;
		}
		else if(msg.wParam == SB_LINEDOWN){
			sel++;
			if(sel>count-1)
				sel = 0;
		}
		m_optVec[sel]->Selected(true);
		m_pTabPage->SelectItem(m_listVec[sel]);
		return;
	}
}

void CMainDlgImpl::InitWindow()
{	
	struct{
		void* ptr;
		const char*  name;
	} li[] = {
		{&m_pbtnMin,		"minbtn"},
		{&m_pbtnMax,		"maxbtn"},
		{&m_pbtnRestore,	"restorebtn"},
		{&m_pbtnClose,		"closebtn"},
		{0,0}
	};
	for(int i=0;li[i].ptr; i++){
		*(CControlUI**)li[i].ptr = static_cast<CControlUI*>(m_PaintManager.FindControl(li[i].name));
	}

	m_pTabList = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("tabs")));
	m_pTabPage = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("switch")));

	vector<string>	catVec;
	m_db->GetCategories(&catVec);

	int x=0;
	for(auto it=catVec.begin(); it!=catVec.end(); it++){
		addTab(it->c_str(),x++,"index_list_option");
	}

	assert(m_optVec.size() == m_listVec.size());
	
	if(m_optVec.size()){
		m_optVec[0]->Selected(true);
		m_pTabPage->SelectItem(0);
	}

	//m_PaintManager.SetTimer(m_pbtnMin,0,5000);
}

bool CMainDlgImpl::addTab(const char* name,int tag,const char* category)
{
	auto pOption = new CMouseWheelOptionUI;
	pOption->SetAttribute("text",name);
	pOption->SetAttribute("width","60");
	pOption->SetAttribute("textcolor","0xFF386382");
	pOption->SetAttribute("normalimage","file='tabbar_normal.png' fade='50'");
	pOption->SetAttribute("hotimage","tabbar_hover.png");
	pOption->SetAttribute("pushedimage","tabbar_pushed.png");
	pOption->SetAttribute("selectedimage","file='tabbar_pushed.png' fade='150'");
	pOption->SetAttribute("group","contenttab");
	pOption->SetAttribute("menu","true");

	if(!m_pTabList->Add(pOption)) 
		return false;

	pOption->SetUserData(category);
	pOption->SetTag(tag);
	m_optVec.push_back(pOption);

	CIndexListUI* pList = new CIndexListUI(m_db,name,m_PaintManager);
	m_pTabPage->Add(pList);
	m_listVec.push_back(pList);

	return true;
}

LRESULT CMainDlgImpl::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	BOOL bZoomed = ::IsZoomed(m_hWnd);
	LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	if (::IsZoomed(m_hWnd) != bZoomed)
	{
		if (!bZoomed)
		{
			m_pbtnMax->SetVisible(false);
			m_pbtnRestore->SetVisible(true);
		}
		else 
		{
			m_pbtnMax->SetVisible(true);
			m_pbtnRestore->SetVisible(false);
		}
	}
	return lRes;
}
