#include "StdAfx.h"

using namespace DuiLib;

#include "InputBox.h"

class CInputBoxImpl : public WindowImplBase
{
public:
	CInputBoxImpl(LPCTSTR T,LPCTSTR P,LPCTSTR I,IInputBoxCallback* cb):
		m_t(T),
		m_p(P),
		m_i(I),
		m_ibcb(cb)
	{

	}
	~CInputBoxImpl()
	{

	}

	virtual CDuiString GetSkinFolder()
	{
		return "skin/";
	}
	virtual CDuiString GetSkinFile()
	{
		return "InputBox.xml";
	}
	virtual LPCTSTR GetWindowClassName(void) const
	{
		return "Å®º¢²»¿Þ";
	}
	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam)
	{
		return FALSE;
	}
	virtual void OnFinalMessage( HWND hWnd ) override
	{
		__super::OnFinalMessage(hWnd);
		//delete this;
	}

	virtual void InitWindow()
	{
		m_edtInput = static_cast<CEditUI*>(m_PaintManager.FindControl("input"));
		m_lblPrompt = static_cast<CLabelUI*>(m_PaintManager.FindControl("prompt"));
		m_lblTitle = static_cast<CLabelUI*>(m_PaintManager.FindControl("title"));

		SMART_ASSERT(m_edtInput && m_lblPrompt && m_lblTitle);

		m_edtInput->SetText(m_i);
		m_lblTitle->SetText(m_t);
		m_lblPrompt->SetText(m_p);

		m_edtInput->SetFocus();
	}

public:
	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg)
	{
		CDuiString name = msg.pSender->GetName();
		if(name == _T("closebtn")){
			m_ibcb->m_code = CInputBox::kClose;
			Close();
			return;
		}
		else if(name == _T("btnOK")){
			LPCTSTR msg = 0;
			m_ibcb->m_str = m_edtInput->GetText();
			if(!m_ibcb->CheckReturn(m_ibcb->m_str,&msg))
			{
				::MessageBox(GetHWND(),msg,"",MB_ICONINFORMATION);
				m_edtInput->SetFocus();
				return;
			}
			m_ibcb->m_code = CInputBox::kOK;
			Close();
			return;
		}
		else if(name == _T("btnCancel")){
			m_ibcb->m_code = CInputBox::kCancel;
			Close();
			return;
		}
	}
private:
	//CInputBox::DLGCODE m_dlgcode;
	LPCTSTR m_t,m_p,m_i;
	IInputBoxCallback* m_ibcb;
	//CDuiString m_str;
	CEditUI*	m_edtInput;
	CLabelUI*	m_lblPrompt;
	CLabelUI*	m_lblTitle;
};

DUI_BEGIN_MESSAGE_MAP(CInputBoxImpl,WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
DUI_END_MESSAGE_MAP()

CInputBox::CInputBox(HWND hParent,LPCTSTR T,LPCTSTR P,LPCTSTR I,IInputBoxCallback* cb)
{
	m_impl = new CInputBoxImpl(T,P,I,cb);
	m_impl->CreateDuiWindow(hParent,T);
	m_impl->CenterWindow();
	m_impl->ShowModal();
	delete m_impl;
}
CInputBox::~CInputBox()
{

}
