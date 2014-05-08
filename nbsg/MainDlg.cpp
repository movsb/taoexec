#include <string>
#include <vector>
#include <sstream>
#include <windows.h>

using namespace std;

#include "Utils.h"
#include "SQLite.h"
#include "PathLib.h"

#include "AddDlg.h"
#include "MainDlg.h"
#include "ico_from_exe/icon_from_exe.h"

#include <UIlib.h>

using namespace DuiLib;

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

		if(!m_hIcon){
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

	void SetIcon(CButtonUI* pbtn,const CIndexItem* item)
	{
		stringstream ss;
		ss << "./icons/" << item->idx << ".ico";
		string ico(ss.str());
		if(GetFileAttributes(ico.c_str()) == INVALID_FILE_ATTRIBUTES){
			if(item->path.find(".exe")!=string::npos || item->path.find(".dll")!=string::npos){
				try{
					CIconFromExe ife;
					ife.LoadExe(item->path.c_str());
					vector<string> ids;
					ife.EnumIcons(&ids);
					if(ids.size()){
						ife.SaveIcon(ids[0].c_str(),ico.c_str());
					}
				}
				catch(const char* s){
					::MessageBox(NULL,s,0,0);
				}
			}
		}
		if(GetFileAttributes(ico.c_str()) != INVALID_FILE_ATTRIBUTES){
			pbtn->SetAttribute("normalimage",ico.c_str());
			pbtn->SetAttribute("bkimage",ico.c_str());
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
		return "Å®º¢²»¿Þ";
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

public:
	const vector<string>& GetCategory() const {return m_catVec;}

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

	vector<string>			m_catVec;
	vector<CIndexListUI*>	m_listVec;
	vector<COptionUI*>		m_optVec;
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
		auto pOpt = static_cast<COptionUI*>(msg.pSender);
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
		auto pOpt = static_cast<COptionUI*>(msg.pSender);
		::MessageBox(GetHWND(),pOpt->GetText(),"",0);
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

	m_db->GetCategories(&m_catVec);

	int x=0;
	for(auto it=m_catVec.begin(); it!=m_catVec.end(); it++){
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
	COptionUI* pOption = new COptionUI;
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
