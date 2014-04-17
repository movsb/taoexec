#pragma once

class CAddDlg
{
public:
	enum TYPE{TYPE_PATH,TYPE_MODIFY,TYPE_NEW};
	CAddDlg(HWND parent,const char* table,TYPE type,LPARAM lParam);
	~CAddDlg();
};

