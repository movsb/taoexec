#include "StdAfx.h"

using namespace std;
using namespace DuiLib;

#include "res/resource.h"

#include "Utils.h"
#include "SQLite.h"
#include "PathLib.h"
#include "Except.h"

#include "AddDlg.h"
#include "MainDlg.h"
#include "InputBox.h"

class CIndexListItemUI : public CVerticalLayoutUI
{
public:
	CIndexListItemUI()
		: m_dwState(0)
	{

	}
	virtual LPCTSTR GetClass() const override
	{
		return _T("ListItemUI");
	}
	virtual LPVOID GetInterface(LPCTSTR pstrName) override
	{
		if(_tcscmp(pstrName, _T("ListItem")) == 0) return this;
		return __super::GetInterface(pstrName);
	}

	virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override
	{
		if(_tcscmp(pstrName, _T("hotimage")) == 0) m_hotimage = pstrValue;

		else return __super::SetAttribute(pstrName, pstrValue);
	}

	virtual void DoEvent(TEventUI& event) override
	{
		if(event.Type == UIEVENT_MOUSEENTER){
			m_dwState |= UISTATE_HOT;
			Invalidate();
		}
		else if(event.Type == UIEVENT_MOUSELEAVE){
			m_dwState &= ~UISTATE_HOT;
			Invalidate();
		}
		else if(event.Type == UIEVENT_BUTTONDOWN){
			GetManager()->SendNotify(this, DUI_MSGTYPE_CLICK);
		}
		else if(event.Type == UIEVENT_CONTEXTMENU){
			GetManager()->SendNotify(this, DUI_MSGTYPE_MENU);
			return;
		}
		return __super::DoEvent(event);
	}

	virtual void PaintStatusImage(HDC hDC)
	{
		if(m_dwState & UISTATE_HOT){
			if(!m_hotimage.IsEmpty()){
				if(!DrawImage(hDC, m_hotimage)){
					m_hotimage.Empty();
				}
			}
		}
	}

protected:
	DWORD			m_dwState;
	CDuiString		m_hotimage;
};

class CMouseWheelHorzUI : public CHorizontalLayoutUI
{
public:
	virtual LPCTSTR GetClass() const override
	{
		return _T("MouseWheelHorzUI");
	}
	virtual LPVOID GetInterface(LPCTSTR pstrName) override
	{
		if(_tcscmp(pstrName, _T("MouseWheelHorz")) == 0) return this;
		return __super::GetInterface(pstrName);
	}

	virtual void DoEvent(TEventUI& event) override
	{
		if(IsMouseEnabled() && event.Type>UIEVENT__MOUSEBEGIN && event.Type<UIEVENT__MOUSEEND){
			if(event.Type == UIEVENT_SCROLLWHEEL){
				TNotifyUI msg;
				msg.pSender = this;
				msg.sType = DUI_MSGTYPE_SCROLL;
				msg.wParam = event.wParam;	//SB_LINEDOWN && SB_LINEUP
				msg.lParam = event.lParam;
				GetManager()->SendNotify(msg);
				return;	//__super不处理,所以直接返回了
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
		if(m_hIcon)
			SMART_ENSURE(::DestroyIcon(m_hIcon),!=FALSE).Ignore();
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
		return 0;
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

		if(m_hIcon == NULL){
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
		if(_tcscmp(pstrClass,_T("IconButton"))==0) return new CIconButtonUI;
		else if(_tcscmp(pstrClass, _T("ListItem")) == 0) return new CIndexListItemUI;
		return nullptr;
	}

	virtual void DoEvent(TEventUI& event) override
	{
		if(IsMouseEnabled() && event.Type>UIEVENT__MOUSEBEGIN && event.Type<UIEVENT__MOUSEEND)
		{
			auto CheckScrollBar = [](CScrollBarUI* pScroll,WPARAM msevt)->bool
			{
				return !pScroll
					|| pScroll->IsVisible() == false
					|| pScroll->GetScrollPos()==0 && msevt==SB_LINEUP
					|| pScroll->GetScrollPos()==pScroll->GetScrollRange() && msevt==SB_LINEDOWN;
			};
			if(event.Type == UIEVENT_SCROLLWHEEL
				&& CheckScrollBar(GetVerticalScrollBar(),event.wParam)
				&& CheckScrollBar(GetHorizontalScrollBar(),event.wParam)
				)
			{
				TNotifyUI msg;
				msg.pSender = this;
				msg.sType = DUI_MSGTYPE_SCROLL;
				msg.wParam = event.wParam;	//SB_LINEDOWN && SB_LINEUP
				msg.lParam = event.lParam;
				GetManager()->SendNotify(msg);
				return;	//__super不处理,所以直接返回了
			}
		}
		return __super::DoEvent(event);
	}

private:
	CDialogBuilder	m_builder;
	CSQLite*		m_db;

public:
	CIndexListUI(CSQLite* db,const char* cat,CPaintManagerUI* pm):
		m_db(db)
	{
		SetItemSize(CSize(80,80));
		SetManager(pm,0,false);
		delete m_builder.Create("ListItem.xml",0,this);

		db->QueryCategory(cat,&m_iiVec);

		for(auto& s : m_iiVec)
			AddItem(s);

		SetAttribute("vscrollbar","true");
	}

	~CIndexListUI()
	{
		for(auto p : m_iiVec)
			delete p;
	}

	void AddItem(const CIndexItem* pii, bool bAddToVector=false)
	{
		CIconButtonUI* pBtn;
		CTextUI* pText;
		auto pContainer = (CIndexListItemUI*)CreateContainer(&pBtn,&pText);
		pBtn->SetAttribute("path",pii->path.c_str());
		pText->SetText(pii->comment.c_str());
		pContainer->SetTag(pii->idx);
		Add(pContainer);
		if(bAddToVector){
			m_iiVec.push_back(const_cast<CIndexItem*>(pii));
		}
	}

	void RemoveItem(CContainerUI* pContainer, int tag)
	{
		CIndexItem* pii = FindIndexList(tag);
		for(auto s=m_iiVec.begin(); s!=m_iiVec.end(); ++s){
			if(*s == pii){
				m_db->DeleteItem(pii->idx); //throw
				m_iiVec.erase(s);
				delete pii;
				SMART_ENSURE(Remove(pContainer),==true).Fatal();
				return;
			}
		}
	}

	CContainerUI* CreateContainer(CIconButtonUI** ppBtn,CTextUI** ppText)
	{
		auto pContainer = m_builder.Create(this)->ToContainerUI();
		*ppBtn = (CIconButtonUI*)pContainer->GetItemAt(0)->ToHorizontalLayoutUI()->GetItemAt(0);
		*ppText = pContainer->ToHorizontalLayoutUI()->GetItemAt(1)->ToTextUI();
		return pContainer;
	}

	CIndexItem* FindIndexList(int tag)
	{
		for(auto& i : m_iiVec){
			if(i->idx == tag){
				return i;
			}
		}
		return nullptr;
	}

	CTextUI* GetTextControlFromButton(CButtonUI* pBtn)
	{
		auto pVert = static_cast<CVerticalLayoutUI*>(pBtn->GetParent()->GetParent());
		return static_cast<CTextUI*>(pVert->GetItemAt(1));
	}

	void RenameIndexItemsCategory(const char* to)
	{
		for(auto& i : m_iiVec){
			i->category = to;
		}
	}

	vector<CIndexItem*> m_iiVec;

private:
};

class CMainDlgImpl : public WindowImplBase
{
public:
	CMainDlgImpl(CSQLite* db)
		: m_tag(0)
		, m_bSearchBoxExpanded(false)
	{
		m_db = db;
	}
	~CMainDlgImpl()
	{

	}
	virtual CControlUI* CreateControl(LPCTSTR pstrClass) override
	{
		if(_tcscmp(pstrClass, _T("MouseWheelHorz")) == 0) return new CMouseWheelHorzUI;

		else return __super::CreateControl(pstrClass);
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
	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
	virtual void OnFinalMessage( HWND hWnd )
	{
		__super::OnFinalMessage(hWnd);
		delete this;
		::PostQuitMessage(0);
	}

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);
	virtual void OnSelectChanged(TNotifyUI& msg);
	virtual void OnTimer(TNotifyUI& msg);
	virtual void OnMenu(TNotifyUI& msg);
	virtual void OnScroll(TNotifyUI& msg);

private:
	bool addTab(const char* name,int tag,const char* group);
	UINT GetTag()
	{
		return m_tag++;
	}

private:
	CSQLite*		m_db;
	UINT			m_tag;
	bool			m_bSearchBoxExpanded;

	CMouseWheelHorzUI*		m_pTabList;
	CTabLayoutUI*			m_pTabPage;

	CButtonUI* m_pbtnClose;
	CButtonUI* m_pbtnMin;
	CButtonUI* m_pbtnRestore;
	CButtonUI* m_pbtnMax;
	CButtonUI* m_pbtnSearch;

	class TabManager
	{
	private:
		struct IndexListOptionMap
		{
			CIndexListUI*	list;
			COptionUI*		option;
			IndexListOptionMap(CIndexListUI* li,COptionUI* op)
			{
				list = li;
				option = op;
			}
		};
	public:
		void Add(COptionUI* op, CIndexListUI* li)
		{
			m_Pages.push_back(IndexListOptionMap(li,op));
		}
		UINT Size() const 
		{
			return m_Pages.size();
		}
		IndexListOptionMap GetAt(UINT index)
		{
			UINT i=0;
			auto s=m_Pages.begin();
			for(;i!=index;){
				++s;
				++i;
			}
			return *s;
		}
		CIndexListUI* FindList(COptionUI* op)
		{
			for(auto s=m_Pages.begin(); s!=m_Pages.end(); ++s){
				if(s->option == op){
					return s->list;
				}
			}
			return nullptr;
		}
		COptionUI* FindOption(CIndexListUI* li)
		{
			for(auto s=m_Pages.begin(); s!=m_Pages.end(); ++s){
				if(s->list == li){
					return s->option;
				}
			}
			for(auto& i : m_Pages){
				if(i.list == li)
					return i.option;
			}
			return nullptr;
		}
		void Remove(COptionUI* op)
		{
			for(auto s=m_Pages.begin(); s!=m_Pages.end(); ++s){
				if(s->option == op){
					m_Pages.erase(s);
					break;
				}
			}
		}
	private:

		list<IndexListOptionMap> m_Pages;
	};

	TabManager				m_tm;

};

CMainDlg::CMainDlg(CSQLite* db)
{
	CMainDlgImpl* pFrame = new CMainDlgImpl(db);
	pFrame->Create(NULL,"<system error!>",UI_WNDSTYLE_FRAME|WS_SIZEBOX,WS_EX_WINDOWEDGE);
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
	if(_tcscmp(msg.pSender->GetClass(), _T("ListItemUI")) == 0){
		int tag = msg.pSender->GetTag();
		auto pList = static_cast<CIndexListUI*>(m_pTabPage->GetItemAt(m_pTabPage->GetCurSel()));
		auto elem = pList->FindIndexList(tag);
		if(APathLib::shellExec(GetHWND(),elem->path.c_str(),elem->param.c_str(),0)){
			//SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
			::ShowWindow(*this, SW_MINIMIZE);
		}
		return;
	}
	if(msg.pSender->GetName() == _T("searchbtn")){
		auto pSearchBox = FindControl(_T("searchbox"))->ToHorizontalLayoutUI();
		pSearchBox->SetFixedWidth(m_bSearchBoxExpanded?20:100);
		m_bSearchBoxExpanded = !m_bSearchBoxExpanded;
		auto pRichEdit = FindControl(_T("searchtext"))->ToRichEditUI();
		pRichEdit->SetText(_T("type sth."));
		pRichEdit->SetSelAll();
		pRichEdit->SetModify(false);
		pRichEdit->SetFocus();
		return;
	}
	return __super::OnClick(msg);
}

void CMainDlgImpl::OnSelectChanged(TNotifyUI& msg)
{
	if(msg.pSender->GetUserData() == "index_list_option"){
		auto pOpt = static_cast<COptionUI*>(msg.pSender);
		m_pTabPage->SelectItem(m_tm.FindList(pOpt));
	}
}

void CMainDlgImpl::OnTimer(TNotifyUI& msg)
{
	//MessageBox(GetHWND(),0,0,0);
}

void CMainDlgImpl::OnMenu(TNotifyUI& msg)
{
	if(msg.pSender->GetUserData() == _T("index_list_option")){
		GetManager()->SendNotify(/*msg.pSender*/ m_pTabList, DUI_MSGTYPE_MENU);
		return;
	}
	if(typeid(*msg.pSender) == typeid(CMouseWheelHorzUI)){
		auto pOption = GetManager()->FindSubControlByPoint(msg.pSender, msg.ptMouse)->ToOptionUI();
		if(pOption == msg.pSender) pOption = nullptr;
		HMENU hMenu = ::LoadMenu(CPaintManagerUI::GetInstance(),MAKEINTRESOURCE(IDM_TABMENU));
		yagc gc(hMenu,[](void* p){return ::DestroyMenu(HMENU(p))!=FALSE;});
		HMENU hSub0 = ::GetSubMenu(hMenu,0);
		auto EnableOptionMenu = [](HMENU hMenu,bool bEnable)->void
		{
			UINT flag = bEnable ? MF_ENABLED : MF_DISABLED|MF_GRAYED;
			::EnableMenuItem(hMenu, MENU_TAB_RENAME,		flag);
			::EnableMenuItem(hMenu, MENU_TAB_CLOSETAB,		flag);
			::EnableMenuItem(hMenu, MENU_TAB_DELETETAB,		flag);
			::EnableMenuItem(hMenu, MENU_TAB_NEWTAB,		MF_ENABLED);
		};
		EnableOptionMenu(hSub0, pOption != nullptr);

		::ClientToScreen(GetHWND(), &msg.ptMouse);
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
					for(string& s : *m_V){
						if(_tcscmp(str,s.c_str()) == 0
							&& str != m_filter)
						{
							*msg = _T("新名字不能与现有名重复!");
							return false;
						}
					}
					return true;
				}
			public:
				void SetCats(vector<string>* V,string filter)
				{
					m_V = V;
					m_filter = filter;
				}
			private:
				vector<string>* m_V;
				string m_filter;
			};

			vector<string>	catVec;
			try{
				m_db->GetCategories(&catVec);
			}
			catch(CExcept* e){
				::MessageBox(GetHWND(),e->desc.c_str(), nullptr, MB_ICONERROR);
				return;
			}

			CRenameCallback rcb;
			CDuiString origin = pOption->GetText();
			rcb.SetCats(&catVec, origin.GetData());

			CInputBox input(GetHWND(),
				_T("重命名"),
				_T("请输入新名字"),
				origin.GetData(),
				&rcb);

			if(rcb.GetDlgCode() != CInputBox::kOK)
				return;

			if(rcb.GetStr() == origin.GetData())
				return;

			try{
				m_db->RenameCategory(origin.GetData(), rcb.GetStr());
				pOption->SetText(rcb.GetStr());
				m_tm.FindList(pOption)->RenameIndexItemsCategory(rcb.GetStr());
			}
			catch(CExcept* e)
			{
				::MessageBox(GetHWND(),e->desc.c_str(),nullptr,MB_ICONEXCLAMATION);
			}
			return;
		}
		else if(id == MENU_TAB_CLOSETAB){
			auto pList = m_tm.FindList(pOption);
			m_tm.Remove(pOption);
			m_pTabList->Remove(pOption);
			m_pTabPage->Remove(pList);
		}
		else if(id == MENU_TAB_NEWTAB){
			class CNewTabCallback : public IInputBoxCallback
			{
			private:
				virtual bool CheckReturn(LPCTSTR str,LPCTSTR* msg) override
				{
					if(_tcschr(str,_T('\''))){
						*msg = _T("标签名中不能包含单引号字符!");
						return false;
					}
					for(string& s : *m_V){
						if(_tcscmp(str,s.c_str()) == 0){
							*msg = _T("新名字不能与现有名重复!");
							return false;
						}
					}
					return true;
				}
			public:
				void SetCats(vector<string>* V)
				{
					m_V = V;
				}
			private:
				vector<string>* m_V;
			};

			vector<string>	catVec;
			try{
				m_db->GetCategories(&catVec);
			}
			catch(CExcept* e){
				::MessageBox(GetHWND(),e->desc.c_str(), nullptr, MB_ICONERROR);
				return;
			}

			CNewTabCallback cb;
			cb.SetCats(&catVec);

			CInputBox input(GetHWND(),
				_T("新标签"),
				_T("请输入新的标签名:"),
				_T(""),
				&cb);

			if(cb.GetDlgCode() != CInputBox::kOK)
				return;

			addTab(cb.GetStr(),GetTag(),"index_list_option");
			return;
		}

		else return;
	}

	if(_tcscmp(msg.pSender->GetClass(), _T("ListItemUI")) == 0){ 
		auto pList = static_cast<CIndexListUI*>(m_pTabPage->GetItemAt(m_pTabPage->GetCurSel()));
		auto elem = pList->FindIndexList(msg.pSender->GetTag());
		SMART_ASSERT(pList && elem).Fatal();

		HMENU hMenu = ::LoadMenu(CPaintManagerUI::GetInstance(),MAKEINTRESOURCE(IDM_MENU_MAIN));
		yagc gc(hMenu,[](void* ptr){return ::DestroyMenu(HMENU(ptr))!=FALSE;});
		HMENU hSub0 = ::GetSubMenu(hMenu,0);
		::ClientToScreen(GetHWND(),&msg.ptMouse);
		UINT id = (UINT)::TrackPopupMenu(hSub0,TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,msg.ptMouse.x+1,msg.ptMouse.y+1,0,GetHWND(),nullptr);
		if(id==0) return;
		switch(id)
		{
		case IDM_INDEX_ADD:
			{
				//为add时,CindexItem*指针分类名
				CAddDlg dlg(GetHWND(),CAddDlg::TYPE_NEW,(CIndexItem*)elem->category.c_str(),m_db);
				if(dlg.GetDlgCode() == CAddDlg::kCancel || dlg.GetDlgCode() == CAddDlg::kClose)
					return;
				CIndexItem* pii = dlg.GetIndexItem();
				pList->AddItem(pii,true);

				return;
			}
		case IDM_INDEX_MODIFY:
			{
				CAddDlg dlg(GetHWND(),CAddDlg::TYPE_MODIFY,elem,m_db);
				if(dlg.GetDlgCode() == CAddDlg::kOK){
					auto pEdit = msg.pSender->ToContainerUI()->GetItemAt(1)->ToEditUI();
					SMART_ASSERT(typeid(*pEdit) == typeid(CTextUI))(typeid(*pEdit).name()).Stop();
					if(pEdit->GetText() != elem->comment.c_str()){
						pEdit->SetText(elem->comment.c_str());
					}
				}
				return;
			}
		case IDM_INDEX_REMOVE_MULTI:
			{
				try{
					pList->RemoveItem(msg.pSender->ToContainerUI(),msg.pSender->GetTag());
				}
				catch(CExcept* e)
				{
					::MessageBox(GetHWND(),e->desc.c_str(), nullptr, MB_ICONEXCLAMATION);
				}
				return;
			}
		case IDM_VIEW_DETAIL:
			{
				string detail(4096,0);
				CIndexItem& si = *elem;
				char tmp[128];
				sprintf(tmp,"%d",si.idx);
				detail = "索引序号: ";
				detail += tmp;
				detail += "\n索引名称: ";
				detail += si.idxn;
				detail += "\n索引分类: ";
				detail += si.category;
				detail += "\n索引说明: ";
				detail += si.comment;
				detail += "\n文件路径: ";
				detail += si.path;
				detail += "\n默认参数: ";
				detail += si.param;
				detail += "\n使用次数: ";
				detail += si.times;
				detail += "\n是否可见: ";
				detail += si.visible?"可见":"不可见";

				::MessageBox(GetHWND(),detail.c_str(),"",MB_OK);
				return ;
			}
		case IDM_VIEW_DIR:
			{
				CIndexItem& ii = *elem;
				APathLib::showDir(GetHWND(),ii.path.c_str());
				return;
			}
		case IDM_VIEW_COPYDIR:
		case IDM_VIEW_COPYPARAM:
			{
				AUtils::setClipData(id==IDM_VIEW_COPYDIR?elem->path.c_str():elem->param.c_str());
				return;
			}
		}//switch(id)
		return;
	}// if btn

	if(msg.pSender->GetName() == "switch"){
		auto isel = m_pTabPage->GetCurSel();
		if(isel == -1) return;
		auto pList = static_cast<CIndexListUI*>(m_pTabPage->GetItemAt(isel));
		auto pOpt = m_tm.FindOption(pList);
		SMART_ASSERT(pList && pOpt);

		HMENU hMenu = ::LoadMenu(CPaintManagerUI::GetInstance(),MAKEINTRESOURCE(IDM_INDEXTAB_MENU));
		yagc gc(hMenu, [](void* ptr){return ::DestroyMenu(HMENU(ptr))!=FALSE;});
		HMENU hSub0 = ::GetSubMenu(hMenu,0);
		::ClientToScreen(GetHWND(),&msg.ptMouse);
		UINT id = (UINT)::TrackPopupMenu(hSub0,TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,msg.ptMouse.x+1,msg.ptMouse.y+1,0,GetHWND(),nullptr);
		if(id==0) return;
		switch(id)
		{
		case MENU_TABINDEX_NEWINDEX:
			{
				//为add时,CindexItem*指针分类名
				CAddDlg dlg(GetHWND(),CAddDlg::TYPE_NEW,(CIndexItem*)pOpt->GetText().GetData(),m_db);
				if(dlg.GetDlgCode() == CAddDlg::kCancel || dlg.GetDlgCode() == CAddDlg::kClose)
					return;
				CIndexItem* pii = dlg.GetIndexItem();
				pList->AddItem(pii,true);

				return;
			}
		}
		return;
	}
}

void CMainDlgImpl::OnScroll(TNotifyUI& msg)
{
	if(msg.pSender == m_pTabList
		|| msg.pSender->GetUserData() == _T("index_list_option"))
	{
		int sel = -1;
		int sz = (int)m_tm.Size();
		for(int i=0; i<sz; ++i){
			if(m_tm.GetAt(i).option->IsSelected()){
				sel = i;
				break;
			}
		}
		if(sel == -1) return;

		if(msg.wParam == SB_LINEUP){
			sel--;
			if(sel<0)
				sel = sz-1;
		}
		else if(msg.wParam == SB_LINEDOWN){
			sel++;
			if(sel>sz-1)
				sel = 0;
		}

		m_tm.GetAt(sel).option->Selected(true);
		m_pTabPage->SelectItem(m_tm.GetAt(sel).list);
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
		{&m_pbtnSearch,		"searchbtn"},
		{0,0}
	};
	for(int i=0;li[i].ptr; i++){
		SMART_ENSURE(*(CControlUI**)li[i].ptr = FindControl(li[i].name),!=nullptr)(i)(li[i].name).Stop();
	}

	m_pTabList = static_cast<CMouseWheelHorzUI*>(FindControl(_T("tabs")));
	m_pTabPage = FindControl(_T("switch"))->ToTabLayoutUI();

	SMART_ASSERT(m_pTabList && m_pTabPage).Fatal();

	vector<string>	catVec;
	m_db->GetCategories(&catVec);

	for(string& s : catVec){
		addTab(s.c_str(),GetTag(),"index_list_option");
	}
	
	if(m_tm.Size()){
		m_pTabPage->SelectItem(m_tm.GetAt(0).list);
		m_tm.GetAt(0).option->Selected(true);
	}

	::DragAcceptFiles(GetHWND(),TRUE);

	if(auto pTitle = FindControl(_T("title"))){
		::SetWindowText(*this, pTitle->GetText());
	}
}

bool CMainDlgImpl::addTab(const char* name,int tag,const char* group)
{
	auto pOption = new COptionUI;
	pOption->SetAttribute("text",name);
	pOption->SetAttribute("width","60");
	pOption->SetAttribute("textcolor","0xFF386382");
	pOption->SetAttribute("normalimage","file='tabbar_normal.png' fade='50'");
	pOption->SetAttribute("hotimage","tabbar_hover.png");
	pOption->SetAttribute("pushedimage","tabbar_pushed.png");
	pOption->SetAttribute("selectedimage","file='tabbar_pushed.png' fade='150'");
	pOption->SetAttribute("group","contenttab");
	pOption->SetAttribute("menu","true");
	pOption->SetAttribute("font", "1");

	pOption->SetUserData(group);
	pOption->SetTag(tag);

	auto pList = new CIndexListUI(m_db,name,GetManager());
	pList->SetTag(tag);
	pList->SetUserData(group);

	m_pTabList->Add(pOption);
	m_pTabPage->Add(pList);
	m_tm.Add(pOption,pList);
	return true;
}

LRESULT CMainDlgImpl::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

	switch(uMsg)
	{
	case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP)wParam;
			POINT pt;
			::GetCursorPos(&pt);
			::ScreenToClient(GetHWND(),&pt);

			auto pControl = FindControl(pt);
			const char* p = pControl->GetClass();

			vector<string> files;
			CDropFiles drop(hDrop, &files);

			if(typeid(*pControl) == typeid(CIndexListUI)){
				auto pList = static_cast<CIndexListUI*>(m_pTabPage->GetItemAt(m_pTabPage->GetCurSel()));
				auto pOpt = m_tm.FindOption(pList);

				for(string& f : files){
					CIndexItem ii;
					ii.category = pOpt->GetText();
					ii.path = f;
					CAddDlg dlg(GetHWND(),CAddDlg::TYPE_PATH,&ii,m_db);
					if(dlg.GetDlgCode() != CInputBox::kOK)
						continue;
					CIndexItem* pii = dlg.GetIndexItem();
					pList->AddItem(pii,true);
				}
			}
			else if(typeid(*pControl) == typeid(CIndexListItemUI)){
				int tag = pControl->GetTag();
				auto pList = static_cast<CIndexListUI*>(m_pTabPage->GetItemAt(m_pTabPage->GetCurSel()));
				auto elem = pList->FindIndexList(tag);

				for(string& f : files){
					APathLib::shellExec(GetHWND(), elem->path.c_str(),f.c_str(),0);
				}
			}
			else if(pControl->GetName()==_T("titlebar")){
				FindControl(_T("clientarea"))->ToVerticalLayoutUI()->SetBkImage(files[0].c_str());
			}
			else{
				bHandled = FALSE;
				return 0;
			}
			bHandled = TRUE;
			return 0;
		}
// 	case WM_KILLFOCUS:
// 		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
// 		goto brk;
	}
brk:
	bHandled = FALSE;
	return 0;
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
