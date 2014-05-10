#pragma once

struct CIndexItem;
class CSQLite;
class CAddDlgImpl;

class CAddDlg
{
public:
	enum TYPE{TYPE_PATH,TYPE_MODIFY,TYPE_NEW};
	enum DLGCODE{kClose,kOK,kCancel};
	CAddDlg(HWND parent,TYPE type,CIndexItem* pii,CSQLite* db);
	~CAddDlg();
	DLGCODE GetDlgCode() const;
	CIndexItem* GetIndexItem() const;

private:
	CAddDlgImpl* m_impl;
};

