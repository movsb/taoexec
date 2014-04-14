#pragma once
#include "WindowBase.h"

class ATabCtrl:public AWindowBase
{
public:
	ATabCtrl(void);
	~ATabCtrl(void);

public:
	void AdjustRect(BOOL fLarger,RECT *prc)
	{
		TabCtrl_AdjustRect(this->GetHwnd(),fLarger,prc);
	}
	BOOL DeleteAllItems()
	{
		return TabCtrl_DeleteAllItems(this->GetHwnd());
	}
	BOOL DeleteItem(int iItem)
	{
		return TabCtrl_DeleteItem(this->GetHwnd(),iItem);
	}
	VOID DeselectAll(UINT fExcludeFocus)
	{
		TabCtrl_DeselectAll(this->GetHwnd(),fExcludeFocus);
	}
	int GetCurSel()
	{
		return TabCtrl_GetCurSel(this->GetHwnd());
	}
	BOOL GetItem(int iItem,LPTCITEM pitem)
	{
		return TabCtrl_GetItem(this->GetHwnd(),iItem,pitem);
	}
	int GetItemCount()
	{
		return TabCtrl_GetItemCount(this->GetHwnd());
	}
	BOOL GetItemRect(int iItem,RECT *prc)
	{
		return TabCtrl_GetItemRect(this->GetHwnd(),iItem,prc);
	}
	int HitTest(LPTCHITTESTINFO pinfo)
	{
		return TabCtrl_HitTest(this->GetHwnd(),pinfo);
	}
	int InsertItem(int iItem,const LPTCITEM pitem)
	{
		return TabCtrl_InsertItem(this->GetHwnd(),iItem,pitem);
	}
	int SetCurSel(int iItem)
	{
		return TabCtrl_SetCurSel(this->GetHwnd(),iItem);
	}
	BOOL SetItem(int iItem,LPTCITEM pitem)
	{
		return TabCtrl_SetItem(this->GetHwnd(),iItem,pitem);
	}

public:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
	{
		return CallWindowProc(m_WndProcOrig,this->GetHwnd(),uMsg,wParam,lParam);
	}
	INT_PTR OnMouseWheel(int delta,int key,int x,int y);
	INT_PTR OnLButtonDown(int key,int x,int y);
};
