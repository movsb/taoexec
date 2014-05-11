#include <string>
#include <vector>
using namespace std;

#include "Utils.h"
#include "SQLite.h"
#include "PathLib.h"
#include "MyUtil.h"

#include <Uilib.h>
using namespace DuiLib;

#include "AddDlg.h"

class CAddDlgImpl : public WindowImplBase
{
public:
	CAddDlgImpl(CAddDlg::TYPE type,CIndexItem* pii,CSQLite* db)
	{
		m_type = type;
		m_pii = pii;
		m_db = db;
	}
	CAddDlg::DLGCODE GetDlgCode() const
	{
		return m_dlgcode;
	}
	CIndexItem* GetIndexItem() const{
		return m_pii;
	}
protected:
	virtual CDuiString GetSkinFolder()
	{
		return "skin/";
	}
	virtual CDuiString GetSkinFile()
	{
		return "AddDlg.xml";
	}
	virtual LPCTSTR GetWindowClassName(void) const
	{
		return "女孩不哭";
	}
	virtual void OnFinalMessage( HWND hWnd )
	{
		__super::OnFinalMessage(hWnd);
		//delete this;	//因为还要取返回码,所以不在此删除
	}
	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam)
	{
		return FALSE;
	}



	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);
	virtual void Notify(TNotifyUI& msg);
	virtual void InitWindow();

private:
	void initFromParam()
	{
		if(m_type == CAddDlg::TYPE::TYPE_PATH){
			string path,args,desc;
			const char* file = m_pii->path.c_str();
			DWORD dwAttr = ::GetFileAttributes(file);
			if(dwAttr == INVALID_FILE_ATTRIBUTES) return;
				
			if(dwAttr & FILE_ATTRIBUTE_DIRECTORY){
				path = file;
				APathLib::getNameString(file,desc);
			}else{
				if(APathLib::IsLink(file)){
					wstring wstr;
					CCharset::Acp2Unicode(string(file),&wstr);
					if(APathLib::ParseLnk(wstr.c_str(),path,args,desc)){
						if(desc == ""){
							APathLib::getFileDescription(path.c_str(),desc);
							if(desc == ""){
								APathLib::getNameString(file,desc);
							}
						}
					}
				}else{
					path = file;
					APathLib::getFileDescription(file,desc);
					if(desc == ""){
						APathLib::getNameString(file,desc);
					}
				}
			}
		
		
			CIndexItem& ii = *m_pii;
			preIndex	->SetText("");
			preComment	->SetText(desc.c_str());
			prePath		->SetText(path.c_str());
			preParam	->SetText(args.c_str());
			preTimes	->SetText("0");

			int i=0;
			auto s=m_tbls.begin(),e=m_tbls.end();
			for(; s!=e; ++s,++i){
				if(*s == m_pii->category){
					break;
				}
			}
			if(s!=e){
				pcboClass->SelectItem(i);
				m_editClass->SetText(m_tbls[i].c_str());
			}
			else{
				CListLabelElementUI* list = new CListLabelElementUI;
				list->SetAttribute("font","0");
				list->SetText(m_pii->category.c_str());
				list->SetPadding(CDuiRect(5,0,5,0));
				pcboClass->Add(list);
				m_tbls.push_back(m_pii->category);
				pcboClass->SelectItem(pcboClass->GetCount()-1);
			}
			//ATTENTION!!!
			m_pii = nullptr;
		}else if(m_type == CAddDlg::TYPE::TYPE_MODIFY){
			preIndex	->SetText(m_pii->idxn.c_str());
			preComment	->SetText(m_pii->comment.c_str());
			prePath		->SetText(m_pii->path.c_str());
			preParam	->SetText(m_pii->param.c_str());
			preTimes	->SetText(m_pii->times.c_str());

			int i=0;
			auto s=m_tbls.begin();
			auto e=m_tbls.end();
			for(; s != e; s++,i++){
				if(m_pii->category == *s){
					pcboClass->SelectItem(i);
					m_editClass->SetText(s->c_str());
					break;
				}
			}
			if(i>=m_tbls.size()){
				assert(0);
			}
		}else if(m_type == CAddDlg::TYPE::TYPE_NEW){
			preIndex	->SetText("");
			preComment	->SetText("");
			prePath		->SetText("");
			preParam	->SetText("");
			preTimes	->SetText("0");

			int i=0;
			auto s=m_tbls.begin(),e=m_tbls.end();
			for(; s!=e; ++s,++i){
				if(*s == (char*)m_pii){
					break;
				}
			}
			if(s!=e){
				pcboClass->SelectItem(i);
				m_editClass->SetText(m_tbls[i].c_str());
			}
			else{
				CListLabelElementUI* list = new CListLabelElementUI;
				list->SetAttribute("font","0");
				list->SetText((char*)m_pii);
				list->SetPadding(CDuiRect(5,0,5,0));
				pcboClass->Add(list);
				m_tbls.push_back((char*)m_pii);
				pcboClass->SelectItem(pcboClass->GetCount()-1);
			}
		}
	}

private:
	CButtonUI* pbtnBrowse;
	CButtonUI* pbtnSave;
	CButtonUI* pbtnClose;
	CRichEditUI* preIndex;
	CRichEditUI* preComment;
	CRichEditUI* prePath;
	CRichEditUI* preParam;
	CRichEditUI* preTimes;
	CComboBoxUI* pcboClass;

	CEditUI* m_editClass;

private:
	CAddDlg::TYPE m_type;
	CIndexItem*	  m_pii;
	CSQLite*	  m_db;
	vector<string> m_tbls;		//已改成分类, 名字还没改

private:
	CAddDlg::DLGCODE m_dlgcode;
};

DUI_BEGIN_MESSAGE_MAP(CAddDlgImpl,WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
DUI_END_MESSAGE_MAP()

CAddDlg::CAddDlg(HWND parent,TYPE type,CIndexItem* pii,CSQLite* db)
{
	assert(parent!=NULL && "CAddDlg::CAddDlg()");
	m_impl = new CAddDlgImpl(type,pii,db);
	m_impl->Create(parent,NULL, WS_VISIBLE, WS_EX_WINDOWEDGE);	
	m_impl->CenterWindow();
	m_impl->ShowModal();
}

CAddDlg::~CAddDlg()
{
	delete m_impl;
}

CAddDlg::DLGCODE CAddDlg::GetDlgCode() const
{
	return m_impl->GetDlgCode();
}
CIndexItem* CAddDlg::GetIndexItem() const
{
	return m_impl->GetIndexItem();
}
void CAddDlgImpl::InitWindow()
{
	struct{
		void* ptr;
		const char*  name;
	} li[] = {
		{&pbtnBrowse,	"btnBrowse"},
		{&pbtnSave,		"btnSave"},
		{&pbtnClose,	"btnClose"},
		{&preIndex,		"richIndex"},
		{&preComment,	"richComment"},
		{&prePath,		"richPath"},
		{&preParam,		"richParam"},
		{&preTimes,		"richTimes"},
		{&pcboClass,	"cboClass"},
		{&m_editClass,	"edtClass"},
		{0,0}
	};
	for(int i=0;li[i].ptr; i++){
		*(CControlUI**)li[i].ptr = static_cast<CControlUI*>(m_PaintManager.FindControl(li[i].name));
	}

	m_db->GetCategories(&m_tbls);

	auto s = m_tbls.begin();
	auto e = m_tbls.end();
	for(; s!=e; s++){
		CListLabelElementUI* list = new CListLabelElementUI;
		list->SetAttribute("font","0");
		list->SetText(s->c_str());
		list->SetPadding(CDuiRect(5,0,5,0));
		pcboClass->Add(list);
	}
	initFromParam();
}

void CAddDlgImpl::OnClick(TNotifyUI& msg)
{
	if(msg.pSender == pbtnClose || msg.pSender->GetName()==_T("closebtn")){
		m_dlgcode = CAddDlg::kCancel;
		Close();
	}
	else if(msg.pSender == pbtnBrowse){
		char str[MAX_PATH];
		if(!APathLib::getOpenFileName(
			GetHWND(),
			"选择可执行文件的路径",
			"应用程序(*.exe)\x00*.exe\x00"
			"批处理(*.bat;*.cmd)\x00*.bat;*.cmd\x00"
			"Windows 脚本(*.vbs;*.vbe)\x00*.vbs;*.vbe)\x00"
			"其它文件(*.*)\x00*.*\x00",
			str))
		{	
			return;
		}
		prePath->SetText(str);
	}
	else if(msg.pSender == pbtnSave){
		CDuiString strIndex = preIndex->GetText();

		if(strIndex.GetLength()<1){
			if(AUtils::msgbox(GetHWND(),MB_ICONINFORMATION|MB_OKCANCEL,"提示",
				"你没有输入索引名, 这样你将不能通过输入索引名来快速打开目标, 确实不用输入么\?") == IDCANCEL)
			{
				return;
			}
		}

		CDuiString strComment = preComment	->GetText();
		CDuiString strPath    = prePath		->GetText();
		CDuiString strParam   = preParam	->GetText();
		CDuiString strTimes   = preTimes	->GetText();

		if(	strIndex.Find('\'')!=-1
			|| strIndex.Find('\"')!=-1
			|| strIndex.Find(' ') !=-1
			|| strIndex.Find('\t')!=-1)
		{
			AUtils::msgbox(GetHWND(),MB_ICONEXCLAMATION,NULL,"索引包含非法字符!");
			return;
		}

		if( strComment.Find('\"')!=-1
			|| strComment.Find('\'')!=-1)
		{
			AUtils::msgbox(GetHWND(),MB_ICONEXCLAMATION,NULL,"说明包含非法字符!");
			return;
		}

		if(	strPath.Find('\'')!=-1
			|| strPath.Find('\"')!=-1
			|| strPath.Find('\t')!=-1)
		{
			AUtils::msgbox(GetHWND(),MB_ICONEXCLAMATION,NULL,"路径包含非法字符!");
			return;
		}

		if(strPath[0]!='{' && !APathLib::isFileExists(strPath.GetData())){
			AUtils::msgbox(GetHWND(),MB_ICONEXCLAMATION,NULL,"文件 \'%s\' 不存在!",strPath.GetData());
			return;
		}

		char buf_times[12];
		sprintf(buf_times,"%u",atoi(strTimes.GetData()));
		if(strcmp(buf_times,strTimes.GetData())){
			AUtils::msgbox(GetHWND(),MB_ICONEXCLAMATION,NULL,"初始次数不正确!");
			return ;
		}

		if(!m_pii) m_pii = new CIndexItem;	//Constructed
		m_pii->idxn = strIndex;
		m_pii->comment = strComment;
		m_pii->path = strPath;
		m_pii->param = strParam;
		m_pii->times = buf_times;
		m_pii->category = m_tbls[pcboClass->GetCurSel()];
		bool bnew = m_pii->idx == -1;
		bool rb = m_db->AddItem(m_pii);
		if(rb){
			::MessageBox(GetHWND(),bnew?"添加成功!":"修改成功!","",MB_OK);
			m_dlgcode = CAddDlg::kOK;
			Close();
			return;
		}
		else{
			::MessageBox(GetHWND(),"保存失败!","",MB_ICONERROR);
			return;
		}
	}
}

void CAddDlgImpl::Notify(TNotifyUI& msg)
{
	if(msg.sType == DUI_MSGTYPE_SETFOCUS){
		if(typeid(*msg.pSender) == typeid(CRichEditUI)){
			auto ctrl = static_cast<CRichEditUI*>(msg.pSender);
			ctrl->SetBkColor(0xFFFFFFFF);
			ctrl->SetTextColor(0xFF000000);
		}
	}
	else if(msg.sType == DUI_MSGTYPE_KILLFOCUS){
		if(typeid(*msg.pSender) == typeid(CRichEditUI)){
			auto ctrl = static_cast<CRichEditUI*>(msg.pSender);
			ctrl->SetBkColor(0x00000000);
			ctrl->SetTextColor(0xFFFFFFFF);
		}
	}
	return __super::Notify(msg);
}
