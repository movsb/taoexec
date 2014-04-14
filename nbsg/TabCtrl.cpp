#include "TabCtrl.h"
#include "Utils.h"
#include "resource.h"
#include "nbsg.h"
#include "ChildFileDlg.h"


ATabCtrl::ATabCtrl(void)
{

}


ATabCtrl::~ATabCtrl(void)
{

}

INT_PTR ATabCtrl::OnMouseWheel(int delta,int key,int x,int y)
{
	ControlMessage cm = {0};
	cm.self = this;
	cm.uMsg = WM_MOUSEWHEEL;
	cm.lParam = delta;
	return GetParent()->SendMessage(WM_NULL,0,LPARAM(&cm));
}

INT_PTR ATabCtrl::OnLButtonDown(int key,int x,int y)
{
	DoDefault(m_uMsg,m_wParam,m_lParam);
	this->SetFocus();
	return 0;
}
