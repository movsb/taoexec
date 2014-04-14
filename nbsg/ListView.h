#ifndef __LISTVIEW_H__
#define __LISTVIEW_H__
#include "WindowBase.h"
#include <CommCtrl.h>
#include "SQLite.h"

#include <iostream>
#include <string>

class AListView:public AWindowBase
{
public:
	BOOL DeleteAllItems()
	{
		return ListView_DeleteAllItems(this->GetHwnd());
	}
	BOOL DeleteItem(int iItem)
	{
		return ListView_DeleteItem(this->GetHwnd(),iItem);
	}
	BOOL GetItem(LPLVITEM pitem)
	{
		return ListView_GetItem(this->GetHwnd(),pitem);
	}
	int GetItemCount()
	{
		return ListView_GetItemCount(this->GetHwnd());
	}
	int GetNextItem(int iStart,UINT flags)
	{
		return ListView_GetNextItem(this->GetHwnd(),iStart,flags);
	}
	int InsertColumn(int iCol,const LPLVCOLUMN pcol)
	{
		return ListView_InsertColumn(this->GetHwnd(),iCol,pcol);
	}
	int InsertItem(const LPLVITEM pitem)
	{
		return ListView_InsertItem(this->GetHwnd(),pitem);
	}
	BOOL SetItem(const LPLVITEM pitem)
	{
		return ListView_SetItem(this->GetHwnd(),pitem);
	}
	BOOL SetItemState(int iItem,UINT state,UINT mask)
	{
		ListView_SetItemState(this->GetHwnd(),iItem,state,mask);
		return TRUE;
	}
	VOID SetItemText(int i,int iSubItem,LPCSTR pszText)
	{
		ListView_SetItemText(this->GetHwnd(),i,iSubItem,(LPSTR)pszText);
	}
	int SetView(DWORD iView)
	{
		return ListView_SetView(this->GetHwnd(),iView);
	}
	HIMAGELIST SetImageList(HIMAGELIST himl,int iImageList)
	{
		return ListView_SetImageList(this->GetHwnd(),himl,iImageList);
	}
	LPARAM GetItemLParam(int iItem)
	{
		LV_ITEM lvi = {0};
		assert(iItem < this->GetItemCount());
		lvi.iItem = iItem;
		lvi.iSubItem = 0;
		lvi.mask = LVIF_PARAM;
		ListView_GetItem(this->GetHwnd(),&lvi);
		return lvi.lParam;
	}
	BOOL GetSelectedItemLParam(LPARAM* p)
	{
		int isel = GetNextItem(-1,LVNI_SELECTED);
		if(isel == -1) return FALSE;
		else {
			*p = GetItemLParam(isel);
			return TRUE;
		}
	}
	void SetExtendedListViewStyle(DWORD dwExStyle)
	{
		ListView_SetExtendedListViewStyle(this->GetHwnd(),dwExStyle);
	}
	DWORD GetExtendedListViewStyle()
	{
		return ListView_GetExtendedListViewStyle(this->GetHwnd());
	}
	int HitTest(LPLVHITTESTINFO pinfo)
	{
		return ListView_HitTest(this->GetHwnd(),pinfo);
	}
};

class AIndexList
{

public:
	
	
	
	
};


#endif//!__LISTVIEW_H__
