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

class IIndexItemObserver
{
public:
	virtual void Update(const CIndexItem* pItem) = 0;
	virtual void Delete(const CIndexItem* pItem) = 0;
	virtual void UpdateTimes(const CIndexItem* pItem) = 0;
};

class CIndexItemObserver
{
public:
	CIndexItemObserver()
	{}

	~CIndexItemObserver()
	{}


	bool Update(const CIndexItem* pItem)
	{
		for (auto& ob : m_obs){
			ob->Update(pItem);
		}
		return true;
	}

	bool Delete(const CIndexItem* pItem)
	{
		for (auto& ob : m_obs){
			ob->Delete(pItem);
		}
		return true;
	}

	bool UpdateTimes(const CIndexItem* pItem)
	{
		for (auto& ob : m_obs){
			ob->UpdateTimes(pItem);
		}
		return true;
	}

	bool Add(IIndexItemObserver* pOb)
	{
		SMART_ASSERT(Find(pOb) == m_obs.end()).Fatal();
		m_obs.push_back(pOb);
		return true;
	}

	bool Remove(IIndexItemObserver* pOb)
	{
		auto it = Find(pOb);
		SMART_ASSERT(it != m_obs.end()).Fatal();
		m_obs.erase(it);
		return true;
	}

	std::vector<IIndexItemObserver*>::const_iterator
		Find(const IIndexItemObserver* pOb)
	{
		for (auto it = m_obs.begin(); it != m_obs.end(); it++){
			if (*it == pOb)
				return it;
		}
		return m_obs.end();
	}


protected:
	std::vector<IIndexItemObserver*> m_obs;
};

class CSlideImageClientUI : public CVerticalLayoutUI
{
public:
	CSlideImageClientUI()
		: m_iTimeout(60*5)
		, m_iCurrent(-1)
	{}

	virtual LPCTSTR GetClass() const override
	{
		return _T("SlideImageClientUI");
	}
	virtual LPVOID GetInterface(LPCTSTR pstrName) override
	{
		if(_tcscmp(pstrName, _T("SlideImageClient")) == 0) return this;
		return __super::GetInterface(pstrName);
	}

	virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override
	{
		if(_tcscmp(pstrName, _T("imgdir")) == 0) m_strDir = pstrValue;
		else if(_tcscmp(pstrName, _T("timeout")) == 0) m_iTimeout = _ttoi(pstrValue);

		else return __super::SetAttribute(pstrName, pstrValue);
	}

	virtual void DoInit() override
	{
		CDuiString path = CPaintManagerUI::GetInstancePath()+"skin/";
		path += m_strDir;
		SMART_ENSURE(APathLib::GetDirectoryFiles(path,&m_files),==true)(m_strDir).Warning();
		SMART_ENSURE(m_files.size(),>0)(path.GetData()).Warning();
		if(m_files.size()){
			setNextSlideImage();
		}
		SMART_ENSURE(GetManager()->SetTimer(this, 0, m_iTimeout*1000), == true)(m_iTimeout).Warning();
	}

	virtual void DoEvent(TEventUI& event) override
	{
		if(event.Type == UIEVENT_TIMER){
			if(event.wParam == 0){
				setNextSlideImage();
				return;
			}
		}
		return __super::DoEvent(event);
	}

private:
	void setNextSlideImage()
	{
		if(!m_files.size()) return;

		auto getNextId = [](int total,int previous)->int
		{
			if(total<=0) return 0;
			if(total==1) return 0;
			if(previous>total-1 || previous<0)
				previous = 0;
			int tmp=0;
			do{
				::srand(::GetTickCount());
				tmp = ::rand()%total;
			}while(tmp == previous);
			return tmp;
		};
		int iNext = getNextId((int)m_files.size(),m_iCurrent);
		if(iNext == m_iCurrent) return;	//as if there is only one bkimage presents
		CDuiString previous = GetBkImage();
		SetBkImage(m_files[iNext].c_str());
		if(!_tcslen(GetBkImage())){
			SetBkImage(previous);
		}
		else{
			SMART_ENSURE(GetManager()->RemoveImage(previous),==true)(previous).Ignore();
		}
	}

private:
	int				m_iTimeout;
	CDuiString		m_strDir;
	vector<string>	m_files;
	int				m_iCurrent;
};

class CIndexListItemUI : public CVerticalLayoutUI, public IContainerSelectUI
{
public:
	CIndexListItemUI()
		: m_dwState(0)
	{

	}
	virtual LPCTSTR GetClass() const override
	{
		return GetClassStatic();
	}
	static LPCTSTR GetClassStatic()
	{
		return _T("IndexListItemUI");
	}
	virtual LPVOID GetInterface(LPCTSTR pstrName) override
	{
		if(_tcscmp(pstrName, _T("ListItem")) == 0) 
			return this;
		else if(_tcscmp(pstrName, _T("IconButton")) == 0)
			return GetItemAt(0)->ToHorizontalLayoutUI()->GetItemAt(0);
		else if(_tcscmp(pstrName, _T("Text"))==0) 
			return GetItemAt(1);
		else if(_tcscmp(pstrName, "IContainerSelectUI") == 0) 
			return static_cast<IContainerSelectUI*>(this);
		else 
			return __super::GetInterface(pstrName);
	}

	virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override
	{
		if(_tcscmp(pstrName, _T("hotimage")) == 0) m_hotimage = pstrValue;
		else if(_tcscmp(pstrName,_T("pushedimage")) == 0) m_pushedimage = pstrValue;

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
			SetCapture();
			m_dwState |= UISTATE_PUSHED;
			Invalidate();
		}
		else if(event.Type == UIEVENT_MOUSEMOVE){
			if(::PtInRect(&m_rcItem, event.ptMouse)){
				m_dwState |= UISTATE_HOT;
				Invalidate();
			}
			else{
				m_dwState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		else if(event.Type == UIEVENT_BUTTONUP){
			if(::PtInRect(&m_rcItem, event.ptMouse)){
				GetManager()->SendNotify(this, DUI_MSGTYPE_CLICK);
			}
			m_dwState &= ~UISTATE_PUSHED;
			Invalidate();
		}
		else if(event.Type == UIEVENT_CONTEXTMENU){
			GetManager()->SendNotify(this, DUI_MSGTYPE_MENU);
			return;
		}
		return __super::DoEvent(event);
	}

	virtual void PaintStatusImage(HDC hDC)
	{
		if(m_uButtonState & UISTATE_SELECTED){
			//CRenderEngine::DrawColor(hDC, m_rcItem, 0x55F00FFF);
			if(!m_hotimage.IsEmpty()){
				if(!DrawImage(hDC, m_hotimage)){
					m_hotimage.Empty();
				}
			}
		}

		if(m_dwState & UISTATE_HOT){
			if(!m_hotimage.IsEmpty()){
				if(!DrawImage(hDC, m_hotimage)){
					m_hotimage.Empty();
				}
			}
		}

		if(m_dwState & UISTATE_PUSHED){
			if(!m_pushedimage.IsEmpty()){
				if(!DrawImage(hDC, m_pushedimage)){
					m_pushedimage.Empty();
				}
			}
		}
	}

	virtual void SelectionSetSelected(bool bSelecte) override
	{
		m_bSelected = bSelecte;
		if(m_bSelected){
			m_uButtonState |= UISTATE_SELECTED;
			Invalidate();
		}
		else {
			m_uButtonState &= ~UISTATE_SELECTED;
			Invalidate();
		}

	}


	virtual bool SelectionIsSelected() override
	{
		return m_bSelected;
	}

protected:
	bool m_bSelected;
	DWORD			m_dwState;
	CDuiString		m_hotimage;
	CDuiString		m_pushedimage;
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
	void PaintBkImage(HDC hDC)
	{
		//第1页内容会在timer到达之前绘制
		if(!m_hIcon) LoadPathIcon();
		if(m_hIcon==(HICON)INVALID_HANDLE_VALUE) return;
		::DrawIconEx(hDC,
			m_rcItem.left,m_rcItem.top,
			m_hIcon,
			GetFixedWidth(),GetFixedHeight(),
			0,nullptr,DI_NORMAL);
	}

public:
	void LoadPathIcon()
	{
		if(m_hIcon == NULL){
			m_hIcon = APathLib::getFileIcon(m_strPath);
			if(!m_hIcon)
				m_hIcon = APathLib::GetClsidIcon(m_strPath.GetData());
			if(!m_hIcon)
				m_hIcon = (HICON)INVALID_HANDLE_VALUE;
		}
	}
private:
	HICON m_hIcon;
	CDuiString m_strPath;
};

class CIndexListUI : public CTileLayoutUI,public IDialogBuilderCallback, public IIndexItemObserver
{
// IIndexItemOberver interface
public:
	virtual void Update(const CIndexItem* pItem) override
	{
		int i = 0;
		for (auto& it : m_iiVec){
			if (pItem->idx == it->idx
				&& pItem->category == it->category)
			{
				*it = *pItem;
				auto pText = GetItemAt(i)->ToContainerUI()->GetItemAt(1);
				pText->SetText(it->comment.c_str());
				break; // only one
			}
			i++;
		}
	}

	virtual void Delete(const CIndexItem* pItem) override
	{
		int i=0;
		for (auto it = m_iiVec.begin(); it != m_iiVec.end(); it++){
			auto item = *it;
			if (item->idx == pItem->idx
				&& item->category == pItem->category)
			{
				m_iiVec.erase(it);
				SMART_ENSURE(RemoveAt(i),==true).Fatal();
				break;
			}
		}
	}

	virtual void UpdateTimes(const CIndexItem* pItem) override
	{
		for (auto& it : m_iiVec){
			if (pItem->idx == it->idx
				&& pItem->category == it->category)
			{
				it->times = pItem->times;
				break; // only one
			}
		}
	}

private:
	const int kInitIconButtonTimerID;
	const int kInitIconButtonTimerDelay;
public:
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
				// 考虑到 1 页能够显示大概 20+ 个图标, 别上常用的在前面, 所以启用鼠标滚轮始终为滚动标签
				// 的功能, 而如果要滚动页面, 则需要手动拖动滚动条
				// 或许我应该开始启用配置文件来应用用户配置了
				return true;
// 				return !pScroll
// 					|| pScroll->IsVisible() == false
// 					|| pScroll->GetScrollPos()==0 && msevt==SB_LINEUP
// 					|| pScroll->GetScrollPos()==pScroll->GetScrollRange() && msevt==SB_LINEDOWN
// 					;
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
			else if(event.Type == UIEVENT_MOUSEMOVE){
				if(GetManager()->GetCapturedUI() == this){
					for(int count=GetCount(); count>=0; count--){
						auto c = GetItemAt(count);
						if(c && c!=this){
							auto i = static_cast<IContainerSelectUI*>(c->GetInterface(_T("IContainerSelectUI")));
							if(i){
								CDuiRect ri = c->GetPos();
								CDuiRect rs = GetSelectionControl()->GetPos();
								CDuiRect r;
								i->SelectionSetSelected(!!::IntersectRect(&r, &ri, &rs));
							}
						}
					}
				}
				if(event.ptMouse.y > GetPos().bottom){
					LineDown();
				}

			}
			else if(event.Type == UIEVENT_CONTEXTMENU){
				if(IsContextMenuUsed()){
					GetManager()->SendNotify(this, DUI_MSGTYPE_MENU);
					return;
				}
			}
			// no return
		}

		if(event.Type == UIEVENT_TIMER){
			if(event.wParam == kInitIconButtonTimerID){
				SMART_ENSURE(GetManager()->KillTimer(this, kInitIconButtonTimerID),==true).Warning();

				//如果在此之间 增加/删除 控件则会造成程序崩溃, 所以必需disable
				//最后在SetEnable的时候会导致自动重绘, 所以不需要手动刷新
				bool bEnabled = IsEnabled();
				SetEnabled(false);

				int cnt = GetCount();
				for(int i=0; i<cnt; ++i){
					auto pItem = static_cast<CIndexListItemUI*>(GetItemAt(i));
					SMART_ASSERT(pItem).Fatal();
					auto pBtn = static_cast<CIconButtonUI*>(pItem->GetInterface(_T("IconButton")));
					SMART_ASSERT(pBtn).Fatal();
					pBtn->LoadPathIcon();
				}

				SetEnabled(bEnabled);
				return;
			}
		}
		return __super::DoEvent(event);
	}

	virtual void DoInit() override
	{
		InitItems(m_cat);
		GetManager()->SetTimer(this, kInitIconButtonTimerID, kInitIconButtonTimerDelay);

		auto pSel = new CButtonUI;
		pSel->SetBkColor(0x22FF0000);
		pSel->SetBorderColor(0x88FF0000);
		pSel->SetBorderSize(1);
		pSel->SetText("~ ^^ ~");
		SetSelectionControl(pSel);
	}

	virtual LPCTSTR GetClass() const override
	{
		return GetClassStatic();
	}

	static LPCTSTR GetClassStatic()
	{
		return _T("IndexListUI");
	}

protected:
	CDialogBuilder	m_builder;
	CSQLite*		m_db;
	CDuiString		m_cat;

public:
	CIndexListUI(CSQLite* db,const char* cat,CPaintManagerUI* pm, CIndexItemObserver& ob)
		: m_db(db)
		, m_cat(cat)
		, kInitIconButtonTimerID(0)
		, kInitIconButtonTimerDelay(3000)
		, m_observer(ob)
	{
		SetItemSize(CSize(80,80));
		SetManager(pm,0,false);
		delete m_builder.Create("ListItem.xml",0,this);

		SetAttribute("vscrollbar","true");
		SetContextMenuUsed(true);

		m_observer.Add(this);
	}

	virtual ~CIndexListUI()
	{
		for(auto p : m_iiVec)
			delete p;
		m_observer.Remove(this);
	}

	virtual void InitItems(const char* cat)
	{
		m_db->QueryCategory(cat,&m_iiVec);
		for(auto& s : m_iiVec)
			AddItem(s);
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

	void RenameIndexItemsCategory(const char* to)
	{
		for(auto& i : m_iiVec){
			i->category = to;
		}
	}

	vector<CIndexItem*> m_iiVec;

private:
	CIndexItemObserver& m_observer;
};

class CIndexListTopIndexUI : public CIndexListUI
{
private:
	const int kTopItems;

public:
	CIndexListTopIndexUI(CSQLite* db, CPaintManagerUI* pm, CIndexItemObserver& ob)
		: CIndexListUI(db, nullptr, pm, ob)
		, kTopItems(20)
	{}

	virtual LPCTSTR GetClass() const override
	{
		return GetClassStatic();
	}

	static LPCTSTR GetClassStatic()
	{
		return _T("IndexListTopIndexUI");
	}

	virtual void InitItems(const char* cat) override // 糟糕的接口
	{
		SMART_ASSERT(cat==nullptr || !*cat)(cat).Stop();

		m_db->QueryTopIndices(&m_iiVec, kTopItems);

		for(auto& i : m_iiVec){
			AddItem(i);
		}
	}
};

CIndexItemObserver g_IndexItemObserver;

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
		else if(_tcscmp(pstrClass, _T("SlideImageClient")) == 0) return new CSlideImageClientUI;

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
	bool AddIndexTab(const char* name,int tag,const char* group);
	bool AddTopIndexTab(const char* name, int tag, const char* group);
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


	template<class T1,class T2>
	class TabManager
	{
	private:
		struct TabItem
		{
		public:
			T1* pt1;
			T2* pt2;

			TabItem(T1* _pt1, T2* _pt2)
			{
				pt1 = _pt1;
				pt2 = _pt2;
			}
// 		private:
// 			TabItem(const TabItem&);
// 			TabItem(const TabItem&&);
// 			TabItem& operator=(const TabItem&);
		};

		std::list<TabItem> m_Tabs;
	public:
		bool Add(T1* pt1, T2* pt2)
		{
			m_Tabs.push_back(TabItem(pt1, pt2));
			return true;
		}

		int Size()
		{
			return m_Tabs.size();
		}

		TabItem& GetAt(int index)
		{
			SMART_ASSERT(index>=0 && index<(int)m_Tabs.size())(index).Fatal();
			int i=0;
			auto s=m_Tabs.begin();
			for(; i!= index;){
				++s;
				++i;
			}
			return *s;
		}

		TabItem& operator[](int index)
		{
			return GetAt(index);
		}

		T1* Find(T2* pt2)
		{
			for(auto& tab : m_Tabs){
				if(tab.pt2 == pt2){
					return tab.pt1;
				}
			}
			return nullptr;
		}

		T2* Find(T1* pt1)
		{
			for(auto& tab : m_Tabs){
				if(tab.pt1 == pt1){
					return tab.pt2;
				}
			}
			return nullptr;
		}

		T1* operator()(T2* pt2)
		{
			return Find(pt2);
		}

		T2* operator()(T1* pt1)
		{
			return Find(pt1);
		}

		void Remove(T1* pt1)
		{
			for(auto s=m_Tabs.begin(); s!=m_Tabs.end(); ++s){
				if(s->pt1 == pt1){
					m_Tabs.erase(s);
					break;
				}
			}
		}

		void Remove(T2* pt2)
		{
			Remove(FindFirst(pt2));
		}
	};

	TabManager<COptionUI,CIndexListUI> m_tm;
};

CMainDlg::CMainDlg(CSQLite* db)
{
	CMainDlgImpl* pFrame = new CMainDlgImpl(db);
	pFrame->CreateDuiWindow(nullptr, nullptr,UI_WNDSTYLE_FRAME);
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
	if(_tcscmp(msg.pSender->GetClass(), CIndexListItemUI::GetClassStatic()) == 0){
		int tag = msg.pSender->GetTag();
		auto pList = static_cast<CIndexListUI*>(m_pTabPage->GetItemAt(m_pTabPage->GetCurSel()));
		auto elem = pList->FindIndexList(tag);
		if(APathLib::shellExec(GetHWND(),elem->path.c_str(),elem->param.c_str(),0)){
			SMART_ENSURE(m_db->UpdateTimes(elem),==true)(elem->idx)(elem->comment).Warning();
			g_IndexItemObserver.UpdateTimes(elem);
			::ShowWindow(*this, SW_MINIMIZE);
		}
		return;
	}
	return __super::OnClick(msg);
}

void CMainDlgImpl::OnSelectChanged(TNotifyUI& msg)
{
	if(msg.pSender->GetUserData() == "index_list_option"
		|| msg.pSender->GetUserData() == "index_option_top"){
		auto pOpt = static_cast<COptionUI*>(msg.pSender);
		m_pTabPage->SelectItem(m_tm(pOpt));
	}
}

void CMainDlgImpl::OnTimer(TNotifyUI& msg)
{
	//MessageBox(GetHWND(),0,0,0);
}

void CMainDlgImpl::OnMenu(TNotifyUI& msg)
{
	if(_tcscmp(msg.pSender->GetClass(), _T("OptionUI"))==0){
		auto pOption = msg.pSender->ToOptionUI();
		if(_tcscmp(pOption->GetGroup(),_T("contenttab")) == 0){
			GetManager()->SendNotify(msg.pSender->GetParent(),DUI_MSGTYPE_MENU);
			return;
		}
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
				m_tm.Find(pOption)->RenameIndexItemsCategory(rcb.GetStr());
			}
			catch(CExcept* e)
			{
				::MessageBox(GetHWND(),e->desc.c_str(),nullptr,MB_ICONEXCLAMATION);
			}
			return;
		}
		else if(id == MENU_TAB_CLOSETAB){
			auto pList = m_tm(pOption);
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

			AddIndexTab(cb.GetStr(),GetTag(),"index_list_option");
			return;
		}

		else return;
	}

	if(_tcscmp(msg.pSender->GetClass(), CIndexListItemUI::GetClassStatic()) == 0){ 
		auto pList = static_cast<CIndexListUI*>(m_pTabPage->GetItemAt(m_pTabPage->GetCurSel()));
		auto elem = pList->FindIndexList(msg.pSender->GetTag());
		SMART_ASSERT(pList && elem).Fatal();
		//
		

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
					g_IndexItemObserver.Update(elem);
				}
				return;
			}
		case IDM_INDEX_REMOVE_MULTI:
			{
				CDuiString prompt;
				prompt += _T("确定要删除以下索引?\n");
				prompt += _T("\n索引: ");
				prompt += elem->idxn.c_str();
				prompt += _T("\n说明: ");
				prompt += elem->comment.c_str();
				if(::MessageBox(GetHWND(),prompt,_T("提示"),
						MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2)==IDNO)
					return;
				try{
					CIndexItem tmpitem = *elem;
					pList->RemoveItem(msg.pSender->ToContainerUI(),msg.pSender->GetTag());
					g_IndexItemObserver.Delete(&tmpitem);
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

	auto pClass = msg.pSender->GetClass();
	if(_tcscmp(pClass, CIndexListUI::GetClassStatic())==0
		|| _tcscmp(pClass, CIndexListTopIndexUI::GetClassStatic())== 0)
	{
		auto isel = m_pTabPage->GetCurSel();
		if(isel == -1) return;
		auto pList = static_cast<CIndexListUI*>(m_pTabPage->GetItemAt(isel));
		auto pOpt = m_tm(pList);
		SMART_ASSERT(pList && pOpt);

		HMENU hMenu = ::LoadMenu(CPaintManagerUI::GetInstance(),MAKEINTRESOURCE(IDM_INDEXTAB_MENU));
		yagc gc(hMenu, [](void* ptr){return ::DestroyMenu(HMENU(ptr))!=FALSE;});
		HMENU hSub0 = ::GetSubMenu(hMenu,0);

		if(_tcscmp(pClass, CIndexListTopIndexUI::GetClassStatic())==0){
			::EnableMenuItem(hSub0, MENU_TABINDEX_NEWINDEX, MF_DISABLED|MF_GRAYED);
		}

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
		|| msg.pSender->GetUserData() == _T("index_list_option")
		|| msg.pSender->GetUserData() == _T("index_option_top"))
	{
		int sel = -1;
		int sz = (int)m_tm.Size();
		for(int i=0; i<sz; ++i){
			if(m_tm.GetAt(i).pt1->IsSelected()){
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

		m_tm.GetAt(sel).pt1->Selected(true);
		m_pTabPage->SelectItem(m_tm.GetAt(sel).pt2);
	}
}

void CMainDlgImpl::InitWindow()
{	
	m_pTabList = static_cast<CMouseWheelHorzUI*>(FindControl(_T("tabs")));
	m_pTabPage = FindControl(_T("switch"))->ToTabLayoutUI();

	SMART_ASSERT(m_pTabList && m_pTabPage).Fatal();

	// 在这里插入常用项
	AddTopIndexTab(_T("常用"), GetTag(), _T("index_option_top"));

	vector<string>	catVec;
	m_db->GetCategories(&catVec);

	for(string& s : catVec){
		AddIndexTab(s.c_str(),GetTag(),"index_list_option");
	}
	
	if(m_tm.Size()){
		m_pTabPage->SelectItem(m_tm.GetAt(0).pt2);
		m_tm.GetAt(0).pt1->Selected(true);
	}

	::DragAcceptFiles(GetHWND(),TRUE);

	auto laSetIcon = [](HWND hWnd){
		HANDLE hIconSmall = ::LoadImage(nullptr, "./head.ico", IMAGE_ICON, 48, 48, LR_LOADFROMFILE);
		HANDLE hIconBig   = ::LoadImage(nullptr, "./head.ico", IMAGE_ICON, 256, 256, LR_LOADFROMFILE);

		if(hIconSmall && hIconBig){
			::SendMessage(hWnd, WM_SETICON, ICON_SMALL, LPARAM(hIconSmall));
			::SendMessage(hWnd, WM_SETICON, ICON_BIG, LPARAM(hIconBig));
			::SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_SMALL, LPARAM(hIconSmall));
			::SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_BIG, LPARAM(hIconBig));
		}
	};

	laSetIcon(GetHWND());

	if(auto pTitle = FindControl(_T("title"))){
		::SetWindowText(*this, pTitle->GetText());
	}
}

bool CMainDlgImpl::AddIndexTab(const char* name,int tag,const char* group)
{
	auto pOption = new COptionUI;
	pOption->SetAttribute("text",name);
	pOption->SetAttribute("width","60");
	pOption->SetAttribute("textcolor","0xFF386382");
	pOption->SetAttribute("hotbkcolor", "0x33FF0000");
	pOption->SetAttribute("selectedbkcolor", "0x33555555");
	pOption->SetAttribute("group","contenttab");
	pOption->SetAttribute("menu","true");
	pOption->SetAttribute("font", "1");

	pOption->SetUserData(group);
	pOption->SetTag(tag);

	auto pList = new CIndexListUI(m_db,name,GetManager(), g_IndexItemObserver);
	pList->SetTag(tag);
	pList->SetUserData(group);

	m_pTabList->Add(pOption);
	m_pTabPage->Add(pList);
	m_tm.Add(pOption,pList);
	return true;
}


bool CMainDlgImpl::AddTopIndexTab(const char* name, int tag, const char* group)
{
	auto pOption = new COptionUI;
	pOption->SetAttribute("text",name);
	pOption->SetAttribute("width","60");
	pOption->SetAttribute("textcolor","0xFF386382");
	pOption->SetAttribute("hotbkcolor", "0x33FF0000");
	pOption->SetAttribute("selectedbkcolor", "0x33555555");
	pOption->SetAttribute("group", "contenttab");
	pOption->SetAttribute("menu","true");
	pOption->SetAttribute("font", "1");

	pOption->SetUserData(group);
	pOption->SetTag(tag);

	auto pList = new CIndexListTopIndexUI(m_db,GetManager(), g_IndexItemObserver);
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
			auto pClass = pControl->GetClass();

			vector<string> files;
			CDropFiles drop(hDrop, &files);

			if (_tcscmp(pControl->GetClass(), CIndexListUI::GetClassStatic()) == 0){
				auto pList = static_cast<CIndexListUI*>(m_pTabPage->GetItemAt(m_pTabPage->GetCurSel()));
				auto pOpt = m_tm(pList);

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
			else if (_tcscmp(pControl->GetClass(), CIndexListItemUI::GetClassStatic()) == 0){
				int tag = pControl->GetTag();
				auto pList = static_cast<CIndexListUI*>(m_pTabPage->GetItemAt(m_pTabPage->GetCurSel()));
				auto elem = pList->FindIndexList(tag);

				for(string f : files){
					if(f.find(' ')!=string::npos){
						string t("\"");
						t += f;
						t += "\"";
						f = t;
					}
					APathLib::shellExec(GetHWND(), elem->path.c_str(),f.c_str(),0);
				}
			}
			else if(pControl->GetName()==_T("titlebar")){
				auto pClient = FindControl(_T("clientarea"))->ToVerticalLayoutUI();
				CDuiString image = pClient->GetBkImage();
				pClient->SetBkImage(files[0].c_str());
				SMART_ENSURE(GetManager()->RemoveImage(image),==true)(image).Warning();
			}
			else{
				bHandled = FALSE;
				return 0;
			}
			bHandled = TRUE;
			return 0;
		}
	}

	bHandled = FALSE;
	return 0;
}

LRESULT CMainDlgImpl::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	BOOL bZoomed = ::IsZoomed(m_hWnd);
	LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	if (bZoomed != ::IsZoomed(m_hWnd))
	{
		bZoomed = ::IsZoomed(m_hWnd);
		auto btnmax = FindControl(_T("maxbtn"));
		auto btnrestore = FindControl(_T("restorebtn"));
		if (bZoomed)
		{
			btnmax->SetVisible(false);
			btnrestore->SetVisible(true);
		}
		else 
		{
			btnmax->SetVisible(true);
			btnrestore->SetVisible(false);
		}
	}
	return lRes;
}
