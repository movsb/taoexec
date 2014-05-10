#pragma once


class CInputBoxImpl;
class IInputBoxCallback;

class CInputBox
{
public:
	enum DLGCODE
	{
		kClose,
		kOK,
		kCancel,
	};
public:
	CInputBox(HWND hParent,LPCTSTR T,LPCTSTR P,LPCTSTR I,IInputBoxCallback* cb);
	~CInputBox();
private:
	CInputBoxImpl* m_impl;
};

class IInputBoxCallback
{
private:
	virtual bool CheckReturn(LPCTSTR str,LPCTSTR* msg)
	{
		return true;
	}

public:
	CDuiString GetStr() const
	{
		return m_str;
	}
	CInputBox::DLGCODE GetDlgCode() const
	{
		return m_code;
	}

private:
	friend class CInputBoxImpl;
	CDuiString m_str;
	CInputBox::DLGCODE m_code;
};
