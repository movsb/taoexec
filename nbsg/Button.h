#pragma once

#include "WindowBase.h"

class AButton:public AWindowBase
{
public:
	AButton(void);
	~AButton(void);

public:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
	{
		return CallWindowProc(m_WndProcOrig,this->GetHwnd(),uMsg,wParam,lParam);
	}
public:
	int GetCheck();
	void SetCheck(WPARAM wState);
	HANDLE SetImage(CHAR* image);

private:
	HANDLE m_hImage;
};
