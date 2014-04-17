#include <string>
#include "nbsg.h"
#include "resource.h"
#include "Utils.h"

#include "ChildIndexDlg.h"
#include "SQLite.h"
#include "MoveToDlg.h"

#include <vector>
#include <map>

using namespace std;

#include <UIlib.h>

using namespace DuiLib;

struct CDataList
{
public:
	string strTable;
	string strAlias;
	string GetAll() const
	{
		return strTable+","+strAlias;
	}
	CDataList(const string& table,const string& alias)
	{
		strTable = table;
		strAlias = alias;
	}
};

typedef vector<CDataList*> CDataListVec;

class CListCallback : public IListCallbackUI
{
public:
	virtual LPCTSTR GetItemText(CControlUI* pList, int iItem, int iSubItem)
	{
		auto pText = reinterpret_cast<CListTextElementUI*>(pList);
		auto tag = pText->GetTag();
		string text;
		switch(iSubItem)
		{
		case 0://数据库表名
			text = (*m_pDataList)[tag]->strTable;
			break;
		case 1://数据库别名
			text = (*m_pDataList)[tag]->strAlias;
			break;
		default:
			assert(0);
			break;
		}
		pList->SetUserData(text.c_str());
		return pList->GetUserData();
	}
	CListCallback(CDataListVec* pDataList)
	{
		m_pDataList = pDataList;
	}
private:
	CDataListVec* m_pDataList;
};

class CMoveToDlgImpl : public WindowImplBase
{
public:
	CMoveToDlgImpl(AChildIndexDlg::LPARAM_STRUCT**ppls,size_t count,const char* origTableName)
		: m_ListCallback(&m_DataListVec)
	{
		m_ppls = ppls;
		m_count = count;
		m_origTableName = origTableName;
	}
	CMoveToDlg::RET_TYPE m_dlgcode;

	virtual CDuiString GetSkinFolder()
	{
		return "skin/";
	}
	virtual CDuiString GetSkinFile()
	{
		return "MoveToDlg.xml";
	}
	virtual LPCTSTR GetWindowClassName(void) const
	{
		return "女孩不哭";
	}

	virtual void OnClick(TNotifyUI& msg);
	virtual void InitWindow();
	virtual void OnFinalMessage(HWND /*hWnd*/);

private:
	void initControls();
	void initTables();
	void deinitTables();
	void initList();
private:
	AChildIndexDlg::LPARAM_STRUCT** m_ppls;
	size_t m_count;
	const char* m_origTableName;
	CListCallback m_ListCallback;
	CDataListVec  m_DataListVec;

private:
	CButtonUI*	m_pbtnMoveTo;
	CButtonUI*	m_pbtnCopyTo;
	CButtonUI*	m_pbtnCancel;
	CListUI*	m_plstList;
};

CMoveToDlg::CMoveToDlg(HWND parent,AChildIndexDlg::LPARAM_STRUCT**ppls,size_t count,const char* origTableName)
{
	CMoveToDlgImpl* pFrame = new CMoveToDlgImpl(ppls,count,origTableName);
	pFrame->Create(parent,NULL,WS_VISIBLE,WS_EX_WINDOWEDGE);
	pFrame->CenterWindow();
	pFrame->ShowModal();
	m_dlgcode = pFrame->m_dlgcode;
	delete pFrame;
}

void CMoveToDlgImpl::OnClick(TNotifyUI& msg)
{
	if(msg.pSender == m_pbtnCancel || msg.pSender->GetName()==_T("closebtn")){
		m_dlgcode = CMoveToDlg::MOVETO_CANCELED;
		Close();
	}
	else if(msg.pSender==m_pbtnMoveTo || msg.pSender==m_pbtnCopyTo){
		bool bMoveto = msg.pSender == m_pbtnMoveTo;
		auto iSel = m_plstList->GetCurSel();
		if(iSel == -1){
			AUtils::msgbox(GetHWND(),MB_ICONEXCLAMATION,"","请选择一个表名项");
			return;
		}

		auto p = reinterpret_cast<CListTextElementUI*>(m_plstList->GetItemAt(iSel));
		const char* newtable = m_DataListVec[p->GetTag()]->strTable.c_str();

		AIndexSqlite* is = new AIndexSqlite;
		is->setTableName(newtable);
		is->attach(GetHWND(),g_pSqliteBase->getPdb());

		AIndexSqlite* ois = new AIndexSqlite;
		ois->setTableName(m_origTableName);
		ois->attach(GetHWND(),g_pSqliteBase->getPdb());

		for(size_t n=0; n<m_count; n++){
			//移动或复制过去算是新增了
			//添加成功会,idx会被修改改新的
			//char* pidx = new char[sizeof(m_ppls[n]->si.idx)];
			//memcpy(pidx,m_ppls[n]->si.idx,sizeof(m_ppls[n]->si.idx));
			//m_ppls[n]->si.idx[0] = '.';
			AIndexSqlite::SQLITE_INDEX si;
			si = m_ppls[n]->si;
			si.idx[0]='.';
			if(is->add(&si)){
				if(bMoveto){
					ois->deleteIndex((char*)m_ppls[n]->si.idx.c_str());
				}
			}else{
				AUtils::msgbox(GetHWND(),MB_ICONERROR,NULL,"失败!");
			}
		}

		delete is;
		delete ois;

		m_dlgcode = bMoveto?CMoveToDlg::MOVETO_MOVETO:CMoveToDlg::MOVETO_COPYTO;

		Close();
	}
}

void CMoveToDlgImpl::InitWindow()
{
	initControls();
	initTables();
	initList();
}

void CMoveToDlgImpl::OnFinalMessage(HWND )
{
	deinitTables();
}

void CMoveToDlgImpl::initControls()
{
	struct{
		void* ptr;
		const char*  name;
	} li[] = {
		{&m_pbtnMoveTo,	"btnMoveTo"},
		{&m_pbtnCopyTo,	"btnCopyTo"},
		{&m_pbtnCancel,	"btnCancel"},
		{&m_plstList,	"listTables"},
		{0,0}
	};
	for(int i=0;li[i].ptr; i++){
		*(CControlUI**)li[i].ptr = static_cast<CControlUI*>(m_PaintManager.FindControl(li[i].name));
	}
}

void CMoveToDlgImpl::initTables()
{
	ASettingsSqlite set;
	int size;
	char* tables;
	set.attach(GetHWND(),g_pSqliteBase->getPdb());
	set.getSetting("index_list",(void**)&tables,&size);

	std::string str(tables);
	str += "\r\n";

	try{
		int cpos = 0;

		std::string::size_type pos=0,last_pos=0;
		while((pos=str.find_first_of('\n',last_pos))!=std::string::npos){
			if(pos-last_pos==1){
				last_pos = pos+1;
				continue;
			}

			std::string all  = str.substr(last_pos,pos-last_pos);
			all[all.length()-1]='\0';

			std::string::size_type pos_2 = all.find_first_of(',');
			std::string data_base = all.substr(0,pos_2);
			std::string data_name = all.substr(pos_2+1);

			m_DataListVec.push_back(new CDataList(data_base,data_name));

			last_pos = pos+1;
		}
		delete[] tables;
	}
	catch(...){
		AUtils::msgbox(GetHWND(),MB_ICONERROR,g_pApp->getAppName(),"索引列表不正确!");
	}
}

void CMoveToDlgImpl::deinitTables()
{
	for(auto it=m_DataListVec.begin(); it!=m_DataListVec.end(); it++){
		delete *it;
	}
}

void CMoveToDlgImpl::initList()
{
	m_plstList->RemoveAll();
	m_plstList->SetTextCallback(&m_ListCallback);

	auto sz = m_DataListVec.size();
	for(auto i=0; i<sz; i++){
		//discard current table
		const char* p = m_DataListVec[i]->strTable.c_str();
		if(strcmp(p, m_origTableName) == 0)
			continue;

		CListTextElementUI* pText = new CListTextElementUI;
		pText->SetTag(i);
		m_plstList->Add(pText);
	}
}

