#include "resource.h"
#include "Utils.h"
#include "SQLite.h"
#include "nbsg.h"
#include "ListView.h"
#include "EditBox.h"
#include "Button.h"
#include "PathLib.h"
#include "Str.h"

#include "ChildIndexDlg.h"
#include "AddDlg.h"

AAddDlg::AAddDlg(AWindowBase* parent,const char* table,int type,LPARAM lParam):
	m_pEditIndex(new AEditBox),
	m_pEditComment(new AEditBox),
	m_pEditPath(new AEditBox),
	m_pEditParam(new AEditBox),
	m_pEditTimes(new AEditBox),

	m_pBtnBrowse(new AButton),
	m_pBtnBatch(new AButton),
	m_pBtnNew(new AButton),
	m_pBtnSave(new AButton),
	m_pBtnClose(new AButton)
{
	bNeedFree = FALSE;
	this->m_Table = table;
	this->SetParent(parent);
	this->m_lParam = lParam;
	this->m_type = type;

	void* pThunk = m_WndThunk.Stdcall(this,&AAddDlg::WindowProc);
	DialogBoxParam(GetModuleHandle(0),MAKEINTRESOURCE(IDD_ADD),parent->GetHwnd(),(DLGPROC)pThunk,0);
}

AAddDlg::~AAddDlg()
{
	delete m_pEditIndex;
	delete m_pEditComment;
	delete m_pEditPath;
	delete m_pEditParam;
	delete m_pEditTimes;
	
	delete m_pBtnBrowse;
	delete m_pBtnBatch;
	delete m_pBtnNew; 
	delete m_pBtnSave;
	delete m_pBtnClose;

	if(bNeedFree) delete (void*)m_lParam;
}

void AAddDlg::MakeIndex(int type,LPARAM lParam)
{
	string path(""),args(""),desc("");
	if(type == TYPE_PATH){
		const char* file = (const char*)lParam;
		DWORD dwAttr = ::GetFileAttributes(file);
		if(dwAttr == INVALID_FILE_ATTRIBUTES) return;

		if(dwAttr & FILE_ATTRIBUTE_DIRECTORY){
			path = file;
			APathLib::getNameString(file,desc);
		}else{
			if(APathLib::IsLink(file)){
				if(APathLib::ParseLnk(AStr(file,false).toWchar(),path,args,desc)){
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

		m_pEditIndex	->SetWindowText("");
		m_pEditComment	->SetWindowText(desc.c_str());
		m_pEditPath		->SetWindowText(path.c_str());
		m_pEditParam	->SetWindowText(args.c_str());
		m_pEditTimes	->SetWindowText("0");
	}else if(type == TYPE_MODIFY){
		AChildIndexDlg::LPARAM_STRUCT* param = reinterpret_cast<AChildIndexDlg::LPARAM_STRUCT*>(lParam);

		m_pEditIndex	->SetWindowText(param->si.index.c_str());
		m_pEditComment	->SetWindowText(param->si.comment.c_str());
		m_pEditPath		->SetWindowText(param->si.path.c_str());
		m_pEditParam	->SetWindowText(param->si.param.c_str());
		m_pEditTimes	->SetWindowText(param->si.times.c_str());
	}else if(type == TYPE_NEW){
		assert(lParam == 0);
		m_pEditIndex	->SetWindowText("");
		m_pEditComment	->SetWindowText("");
		m_pEditPath		->SetWindowText("");
		m_pEditParam	->SetWindowText("");
		m_pEditTimes	->SetWindowText("0");
	}
}

void AAddDlg::MakeTables()
{
	HWND hComboType = GetDlgItem(this->GetHwnd(),IDC_ADD_COMBO_TYPE);
	ASettingsSqlite set;
	int size;
	char* tables;
	set.attach(this->GetHwnd(),g_pSqliteBase->getPdb());
	set.getSetting("index_list",(void**)&tables,&size);
	
	std::string str(tables);
	str += "\r\n";
	
	//取得个数
	int n=1;
	std::string::size_type pos=-1;
	while((pos=str.find('\n',pos+1))!=std::string::npos){
		n++;
	}
	//根据个数分配空间,前面是指针数组,后面是数据
	m_zTables = new char[size+(n+1)*sizeof(char*)];
	
	try{
		int cindex=0;
		int cpos = 0;
		char* cptr=m_zTables+(n+1)*sizeof(char*);
		
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
			
			((char**)m_zTables)[cindex] = cptr;
			memcpy(cptr,data_base.c_str(),data_base.size()+1);
			
			cindex++;
			cptr += data_base.size()+1;
			
			ComboBox_AddString(hComboType,all.c_str());
			
			last_pos = pos+1;
		}
		delete[] tables;
		
		((char**)m_zTables)[cindex] = NULL;
	}
	catch(...){
		AUtils::msgbox(this->GetHwnd(),MB_ICONERROR,g_pApp->getAppName(),"索引列表不正确!");
	}
	
	if(!m_Table || m_Table[0] == '\0'){
		ComboBox_SetCurSel(hComboType,0);
	}else{
		int i=0;
		char** p = (char**)m_zTables;
		while(p[i] != NULL){
			if(strcmp(p[i],m_Table) == 0){
				ComboBox_SetCurSel(hComboType,i);
				break;
			}
			i++;
		}
	}

}

INT_PTR AAddDlg::DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR AAddDlg::OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM lParam)
{
	m_hWnd = hWnd;
	this->CenterWindow(GetParent()->GetHwnd());
	this->ShowWindow(SW_SHOW);

	m_pEditIndex	->attach(this,IDC_ADD_EDIT_INDEX);
	m_pEditComment	->attach(this,IDC_ADD_EDIT_COMMENT);
	m_pEditPath		->attach(this,IDC_ADD_EDIT_PATH);
	m_pEditParam	->attach(this,IDC_ADD_EDIT_PARAM);
	m_pEditTimes	->attach(this,IDC_ADD_EDIT_TIMES);

	m_pEditIndex	->SubClass();
	m_pEditComment	->SubClass();
	m_pEditPath		->SubClass();
	m_pEditParam	->SubClass();
	m_pEditTimes	->SubClass();

	m_pEditPath		->DragAcceptFiles(TRUE);
	m_pEditParam	->DragAcceptFiles(TRUE);

	m_pBtnBrowse	->attach(this,IDC_ADD_BROWSE_PATH);
	m_pBtnBatch		->attach(this,IDC_ADD_BATCH);
	m_pBtnNew 		->attach(this,IDC_ADD_NEW);
	m_pBtnSave		->attach(this,IDC_ADD_SAVE);
	m_pBtnClose		->attach(this,IDC_ADD_CLOSE);

	this->MakeIndex(m_type,m_lParam);

	this->DragAcceptFiles(TRUE);

	MakeTables();

	::SetFocus(NULL);

	return 0;
}

INT_PTR AAddDlg::OnClose()
{
	::EndDialog(this->GetHwnd(),0);
	return 0;
}

INT_PTR AAddDlg::OnNcDestroy()
{
	return 0;
}

INT_PTR AAddDlg::OnDropFiles(HDROP hDrop)
{
	char files[MAX_PATH];

	DragQueryFile(hDrop,0,files,sizeof(files));

	this->MakeIndex(TYPE_PATH,LPARAM(files));

	return 0;
}

INT_PTR AAddDlg::OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl)
{
	switch (ctrlID)
	{
	case IDC_ADD_CLOSE:
		this->SendMessage(WM_CLOSE);
		return 0;
	case IDC_ADD_BATCH:
		AUtils::msgbox(this->GetHwnd(),MB_ICONEXCLAMATION,g_pApp->getAppName(),"未实现!");
		return 0;
	case IDC_ADD_BROWSE_PATH:
		{
			char str[MAX_PATH];
			if(!APathLib::getOpenFileName(
				this->GetHwnd(),
				"选择可执行文件的路径",
				"应用程序(*.exe)\x00*.exe\x00"
				"批处理(*.bat;*.cmd)\x00*.bat;*.cmd\x00"
				"Windows 脚本(*.vbs;*.vbe)\x00*.vbs;*.vbe)\x00"
				"其它文件(*.*)\x00*.*\x00",
				str))
			{	
				return 0;
			}
			m_pEditPath->SetWindowText(str);
			return 0;
		}
	case IDC_ADD_NEW:
		{
			AUtils::msgbox(this->GetHwnd(),MB_ICONEXCLAMATION,"sorry...","暂时不支持从这里新增,请从菜单中进入!");
			return 0;
		}
	case IDC_ADD_SAVE:
		{
			std::string strIndex = m_pEditIndex->GetWindowText();

			if(strIndex.length()<1){
				if(AUtils::msgbox(this->GetHwnd(),MB_ICONINFORMATION|MB_OKCANCEL,"提示",
					"你没有输入索引名, 这样你将不能通过输入索引名来快速打开目标, 确实不用输入么\?") == IDCANCEL)
				{
					return 0;
				}
			}

			std::string strComment = m_pEditComment->GetWindowText();

			std::string strPath = m_pEditPath->GetWindowText();

			std::string strParam = m_pEditParam->GetWindowText();

			std::string strTimes = m_pEditTimes->GetWindowText();

			if(	strIndex.find('\'')!=string::npos ||
				strIndex.find('\"')!=string::npos ||
				strIndex.find(' ') !=string::npos ||
				strIndex.find('\t')!=string::npos)
			{
				AUtils::msgbox(this->GetHwnd(),MB_ICONEXCLAMATION,NULL,"索引包含非法字符!");
				return 0;
			}

			if( strComment.find('\"')!=string::npos||
				strComment.find('\'')!=string::npos)
			{
				AUtils::msgbox(this->GetHwnd(),MB_ICONEXCLAMATION,NULL,"说明包含非法字符!");
				return 0;
			}

			if(	strPath.find('\'')!=string::npos ||
				strPath.find('\"')!=string::npos ||
				strPath.find('\t')!=string::npos)
			{
				AUtils::msgbox(this->GetHwnd(),MB_ICONEXCLAMATION,NULL,"路径包含非法字符!");
				return 0;
			}

			if(strPath[0]!='{' && !APathLib::isFileExists(strPath.c_str())){
				AUtils::msgbox(this->GetHwnd(),MB_ICONEXCLAMATION,NULL,"文件 \'%s\' 不存在!",strPath.c_str());
				return 0;
			}

			char buf_times[12];
			sprintf(buf_times,"%u",atoi(strTimes.c_str()));
			if(strcmp(buf_times,strTimes.c_str())){
				AUtils::msgbox(this->GetHwnd(),MB_ICONEXCLAMATION,NULL,"初始次数不正确!");
				return 0;
			}

			AIndexSqlite::SQLITE_INDEX si;
			AIndexSqlite::SQLITE_INDEX* lastpsi = NULL;

			lastpsi = reinterpret_cast<AIndexSqlite::SQLITE_INDEX*>(m_lParam);

			if(m_type == TYPE_NEW || m_type == TYPE_PATH){
				si.idx[0] = '.';
			}else{//修改
				//strcpy(si.idx,lastpsi->idx);
				si.idx = lastpsi->idx;
			}

			si.index   = strIndex.c_str();
			si.comment = strComment.c_str();
			si.path    = strPath.c_str();
			si.param   = strParam.c_str();
			si.times   = strTimes.c_str();

			

			HWND hComboType = GetDlgItem(this->GetHwnd(),IDC_ADD_COMBO_TYPE);
			const char* selTable = ((char**)m_zTables)[ComboBox_GetCurSel(hComboType)];
			BOOL bChangeTable = m_type==TYPE_MODIFY && strcmp(selTable,m_Table);

			AIndexSqlite is;
			is.setTableName(selTable); //---采用新选择的表了
			is.attach(this->GetHwnd(),g_pSqliteBase->getPdb());

			if(bChangeTable){
				//更改了表名,不管怎样idx都应该为空
				si.idx[0] = '.';
			}

			if(is.add(&si)){
				if(m_type != TYPE_MODIFY){//如果是新增的话,更改标志为修改
					m_type = TYPE_MODIFY;
					bNeedFree = TRUE;
					lastpsi = new AIndexSqlite::SQLITE_INDEX;
					*lastpsi = si;
					m_lParam = (LPARAM)lastpsi;
				}else if(m_type == TYPE_MODIFY){
					if(bChangeTable){
						AIndexSqlite is;
						is.setTableName(m_Table);
						is.attach(this->GetHwnd(),g_pSqliteBase->getPdb());
						is.deleteIndex((char*)lastpsi->idx.c_str());
					}
					//memcpy((void*)m_lParam,&si,sizeof(si));
					*reinterpret_cast<AIndexSqlite::SQLITE_INDEX*>(m_lParam) = si;
				}
				//添加成功,更改为当前的表名
				m_Table = selTable;
			}
			return 0;
		}
	}
	return 0;
}


INT_PTR AAddDlg::OnNull(LPARAM lParam)
{
	ControlMessage* pcm = reinterpret_cast<ControlMessage*>(lParam);
	if(!pcm) return 0;

	if(pcm->uMsg == WM_DROPFILES){
		char path[MAX_PATH]={0};
		DragQueryFile(HDROP(pcm->lParam),0,path,sizeof(path));
		pcm->self->SetWindowText(path);
		return SetDlgResult(0);
	}
	return 0;
}
