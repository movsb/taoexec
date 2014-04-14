#include <string>
#include <windows.h>
using namespace std;
#include "resource.h"
#include "nbsg.h"
#include "ChildIndexDlg.h"
#include "ChildFileDlg.h"
#include "ChildSettingsDlg.h"
#include "TabCtrl.h"
#include "EditBox.h"
#include "Utils.h"
#include "Mini.h"
#include "MainDlg.h"

RECT AMainDlg::m_RectWnd = {-1,-1,700,500};

AMainDlg::AMainDlg(const char* cmd,bool show):
	m_pTabCtrl(new ATabCtrl)
	//m_pMini(new AMini(this))
{
	AWindowBase::AddWindow(this);
	m_bAutoHide = TRUE;
	debug_out(("AMainDlg::AMainDlg()\n"));
	create(cmd,show);
}

AMainDlg::~AMainDlg()
{
	delete m_pTabCtrl;
	delete m_pMini;
	debug_out(("AMainDlg::~AMainDlg()\n"));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////// 
void AMainDlg::SetAutoHide(BOOL bAutoHide)
{
	m_bAutoHide = bAutoHide;
}
void AMainDlg::AddNewTab(AWindowBase* pWindow,const char* szTabName)
{
	assert(pWindow != NULL && szTabName != NULL);

	TabInfo* pti = new TabInfo;
	pti->pWindow = pWindow;

	TCITEM tci = {0};
	tci.mask = TCIF_TEXT;
	tci.pszText = (LPTSTR)szTabName;
	tci.lParam = reinterpret_cast<LPARAM>(pti);

	this->AdjustTabPageSize(pWindow);
	pWindow->ShowWindow(SW_HIDE);

	pti->iItem = m_pTabCtrl->InsertItem(m_pTabCtrl->GetItemCount(),&tci);

	m_Pages.push_back(pti);
}

void AMainDlg::AdjustTabPageSize(AWindowBase* pWindow)
{
	RECT rcThis;
	RECT rcTab;
	this->GetClientRect(&rcThis);
	m_pTabCtrl->GetClientRect(&rcTab);

	if(pWindow){
		pWindow->SetWindowPos(0,rcTab.bottom,rcThis.right,rcThis.bottom-rcTab.bottom);
	}else{
		TabPagesIter iter;
		for(iter=m_Pages.begin(); iter!= m_Pages.end(); ++iter){
			assert((*iter)->pWindow != NULL);
			AdjustTabPageSize((*iter)->pWindow);//一定不能为NULL, 否则递归就死循环了
		}
		//debug_out(("重新为%d个窗口计算大小...\n",m_Pages.size()));
	}
}

void AMainDlg::ShowTabPage(int nPage)
{
	AWindowBase* hShow=0;
	assert(nPage < m_pTabCtrl->GetItemCount());
	for(TabPagesIter iter=m_Pages.begin(); iter!=m_Pages.end(); iter++){
		TabInfo* pinfo = *iter;
// 		if(pinfo->iItem == nPage){
// 			pinfo->pWindow->ShowWindow(SW_SHOW);
// 		}else{
// 			pinfo->pWindow->ShowWindow(SW_HIDE);
// 		}
		pinfo->pWindow->ShowWindow(SW_HIDE);
		if(pinfo->iItem == nPage) hShow = pinfo->pWindow;
	}
	if(hShow) hShow->ShowWindow(SW_SHOW);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//以下是消息处理函数

INT_PTR AMainDlg::OnClose()
{
	for(TabPagesIter iter=m_Pages.begin(); iter!=m_Pages.end(); ++iter){
		TabInfo* info = *iter;
		info->pWindow->DestroyWindow();	//子窗口都是由CreateDialog*创建的,需要由DestroyWindow销毁,而不是EndDialog
		delete info->pWindow;

		m_pTabCtrl->DeleteItem(info->iItem);
		delete info;
	}
	m_pMini->DestroyWindow();

	this->GetWindowRect(&m_RectWnd);
	if(IsZoomed(this->GetHwnd())) m_RectWnd.left=m_RectWnd.top=-2;
	if(IsIconic(this->GetHwnd())) m_RectWnd.left=m_RectWnd.top=-1;

	AWindowBase::DeleteWindow(this);
	this->DestroyWindow();
	//this->EndDialog(0);
	return 0;
}

INT_PTR AMainDlg::OnNCDestroy()
{
	
	return 0;
}

INT_PTR AMainDlg::OnSize(int width,int height)
{
	RECT rcDlg,rcTab,rcMini;
	this->GetClientRect(&rcDlg);
	m_pTabCtrl->GetClientRect(&rcTab);
	m_pMini->GetClientRect(&rcMini);

	//DWM可能会窗口变得更小
	//this->GetWindowRect(&rcThis);
	//if(rcThis.right-rcThis.left < 400) rcThis.right += rcThis.left+400;
	//if(rcThis.bottom-rcThis.top < 300) rcThis.bottom += rcThis.top+300;
	
	m_pTabCtrl->SetWindowPos(0,0,rcDlg.right-rcMini.right-1-1,rcTab.bottom);
	m_pMini->SetWindowPos(rcDlg.right-rcMini.right-1,0+1,rcMini.right,rcTab.bottom-1-1);

	this->AdjustTabPageSize();
	return 0;
}

INT_PTR AMainDlg::OnSizing(WPARAM wParam,LPRECT pRect)
{
	int width = pRect->right-pRect->left;
	int height = pRect->bottom-pRect->top;

	if(width<400)  pRect->right = pRect->left + 400;
	if(height<300) pRect->bottom = pRect->top + 300;

	return TRUE;
}

INT_PTR AMainDlg::OnHotKey(WPARAM id)
{
	return 0;
}

INT_PTR AMainDlg::OnUser(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR AMainDlg::OnApp(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR AMainDlg::OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM lParam)
{
	m_hWnd = hWnd;

	m_pTabCtrl->attach(this,IDD_MAIN_TAB);
	m_pTabCtrl->SubClass();
	m_pTabCtrl->ShowWindow(SW_SHOW);

	m_pMini = new AMini(this);

	//从上次关闭的位置显示窗口
	if(m_RectWnd.left==-1&&m_RectWnd.top==-1) this->CenterWindow(NULL);
	else if(m_RectWnd.left==-2&&m_RectWnd.top==-2) ShowWindow(SW_MAXIMIZE);
	else this->SetWindowPos(NULL,m_RectWnd.left,m_RectWnd.top,m_RectWnd.right-m_RectWnd.left,m_RectWnd.bottom-m_RectWnd.top,SWP_NOZORDER);

	this->SetWindowText("求妹子~");
	this->ShowWindow(SW_SHOWNORMAL);

	AChildSettingsDlg* pSettings = new AChildSettingsDlg(this);

	AddNewTab(pSettings,pSettings->GetWindowText().c_str());

	ShowTabPage(0);

	return FALSE;
}

INT_PTR AMainDlg::OnNotify(LPNMHDR phdr)
{
	if(phdr->idFrom == m_pTabCtrl->GetCtrlID()){
		if(phdr->code==TCN_SELCHANGING){
			int cur_sel = m_pTabCtrl->GetCurSel();
			assert(cur_sel!=-1);
			return SetDlgResult(FALSE);
		}else if(phdr->code==TCN_SELCHANGE){
			int cur_sel = m_pTabCtrl->GetCurSel();
			assert(cur_sel!=-1);
			ShowTabPage(cur_sel);
			return 0;
		}else if(phdr->code == NM_RCLICK){

			return 0;
		}
	}
	return 0;
}

INT_PTR AMainDlg::OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl)
{
	return 0;
}

bool AMainDlg::create(const char* cmd,bool nShowCmd)
{
	void* pWndThunk = m_WndThunk.Stdcall(this,&AMainDlg::WindowProc);
	CreateDialogParam(g_pApp->getInstance(),MAKEINTRESOURCE(IDD_MAIN),NULL,(DLGPROC)pWndThunk,0);
	this->ShowWindow(nShowCmd);
	return true;
}

INT_PTR AMainDlg::OnNull(LPARAM lParam)
{
	ControlMessage* pcm = reinterpret_cast<ControlMessage*>(lParam);
	if(!pcm) return 0;

	if(pcm->self == NULL){
		switch(pcm->uMsg-WM_USER)
		{
		case 0:
			this->SetAutoHide((BOOL)pcm->lParam);
			return 0;
		case 1:
			this->AddNewTab(reinterpret_cast<AWindowBase*>(pcm->wParam),reinterpret_cast<const char*>(pcm->lParam));
			return 0;
		default:
			debug_out(("错误:遇到未处理的WM_NULL消息...\n"));
			return 0;
		}
	}

	if(pcm->self == m_pTabCtrl){
		switch(pcm->uMsg)
		{
		case WM_MOUSEWHEEL:
			{
				int delta = pcm->lParam;
				int cnt = m_pTabCtrl->GetItemCount();
				int cur = m_pTabCtrl->GetCurSel();

				if(cnt <= 1) return SetDlgResult(0);

				if(delta < 0){
					cur++;
					if(cur>cnt-1) cur = 0;
				}else if(delta > 0){
					cur--;
					if(cur < 0){
						cur = cnt - 1;
					}
				}

				ShowTabPage(cur);
				m_pTabCtrl->SetCurSel(cur);

				return SetDlgResult(0);
			}
		default:
			return SetDlgResult(pcm->self->DoDefault(pcm));
		}
	}
	return 0;
}

INT_PTR AMainDlg::DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}
