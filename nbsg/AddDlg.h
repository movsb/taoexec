#pragma once

struct CIndexItem;
class CSQLite;

class CAddDlg
{
public:
	enum TYPE{TYPE_PATH,TYPE_MODIFY,TYPE_NEW};
	CAddDlg(HWND parent,TYPE type,CIndexItem* pii,CSQLite* db);
	~CAddDlg();
};

