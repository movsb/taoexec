#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cassert>

using namespace std;

#include "SQLite.h"

#include "sqlite3/sqlite3.h"
#pragma comment(lib,"sqlite3/sqlite3")

#include "Except.h"


class CSQLiteImpl
{
public:
	CSQLiteImpl()
	:m_db(0)
	{}
	bool Open(const char* fn);
	bool Close();
	bool QueryCategory(const char* cat,vector<CIndexItem*>* V);
	bool QueryIndices(const char* find,vector<CIndexItem*>* R,bool* found);
	bool GetCategories(vector<string>* cats);
	bool RenameCategory(const char* from, const char* to);
	bool FreeVectoredIndexItems(vector<CIndexItem*>* R);
	bool AddItem(CIndexItem* pii);
	bool DeleteItem(int idx);
	bool UpdateTimes(CIndexItem* pii);

private:
	bool init();

	struct SqliteCallbackStruct
	{
		enum TYPE{kQueryCategory,kQueryIndices};
		TYPE type;
		CSQLiteImpl* that;
	};

	struct CBS_QueryCategory : public SqliteCallbackStruct
	{
		const char* cat;
		vector<CIndexItem*>* V;
	};

	struct CBS_QueryIndices : public SqliteCallbackStruct
	{
		string find;
		bool found;
		vector<CIndexItem*>* indices;
	};
	static int __cdecl cbQeuryCallback(void* ud,int argc,char** argv,char** col);

private:
	sqlite3* m_db;
};

CSQLite::CSQLite()
{
	m_sqlite = new CSQLiteImpl;
}
CSQLite::~CSQLite()
{
	delete m_sqlite;
}
bool CSQLite::Open(const char* fn)
{
	return m_sqlite->Open(fn);
}
bool CSQLite::Close()
{
	return m_sqlite->Close();
}
bool CSQLite::QueryCategory(const char* cat,vector<CIndexItem*>* V)
{
	return m_sqlite->QueryCategory(cat,V);
}
bool CSQLite::QueryIndices(const char* find,vector<CIndexItem*>* R,bool* found)
{
	return m_sqlite->QueryIndices(find,R,found);
}
bool CSQLite::GetCategories(vector<string>* cats)
{
	return m_sqlite->GetCategories(cats);
}
bool CSQLite::RenameCategory(const char* from, const char* to)
{
	return m_sqlite->RenameCategory(from,to);
}
bool CSQLite::FreeVectoredIndexItems(vector<CIndexItem*>* R)
{
	return m_sqlite->FreeVectoredIndexItems(R);
}
bool CSQLite::AddItem(CIndexItem* pii)
{
	return m_sqlite->AddItem(pii);
}
bool CSQLite::DeleteItem(int idx)
{
	return m_sqlite->DeleteItem(idx);
}
bool CSQLite::UpdateTimes(CIndexItem* pii)
{
	return m_sqlite->UpdateTimes(pii);
}



bool CSQLiteImpl::Open(const char* fn)
{
	int rv;
	rv = sqlite3_open(fn,&m_db);
	if(rv != SQLITE_OK){
		sqlite3_close(m_db);
		m_db = nullptr;
		throw CExcept("sqlite3数据库打开失败!","CSQLiteImpl::Open()");
	}
	init();
	return true;
}

bool CSQLiteImpl::Close()
{
	if(sqlite3_close(m_db)==SQLITE_OK){
		m_db = nullptr;
		return true;
	}
	else{
		throw CExcept("sqlite3数据库未能成功关系!","CSQLiteImpl::Close()");
	}
	return false;
}

bool CSQLiteImpl::init()
{
	char* err;
	string sql("create table if not exists tbl_index (idx integer primary key,category text,name text,visible integer,comment text,path text,param text,times integer default 0);");
	if(sqlite3_exec(m_db, sql.c_str(), nullptr,nullptr, &err) == SQLITE_OK){
		return true;
	}else{
		string s(err);
		sqlite3_free(err);
		throw CExcept(s.c_str(),"CSQLiteImpl::init()");
	}
	return false;
}

bool CSQLiteImpl::GetCategories(vector<string>* cats)
{
	sqlite3_stmt* stmt=0;
	int rv;
	string sql("select distinct category from tbl_index;");
	rv = sqlite3_prepare(m_db,sql.c_str(),-1,&stmt,nullptr);
	if(rv == SQLITE_OK){
		cats->clear();
		for(bool loop=true; loop ;){
			switch(sqlite3_step(stmt))
			{
			case SQLITE_BUSY:
			case SQLITE_ERROR:
			case SQLITE_MISUSE:
				sqlite3_finalize(stmt);
				throw CExcept(sqlite3_errmsg(m_db),"CSQLiteImpl::getCategories()");
			case SQLITE_ROW:
				{
					const char* cat = (char*)sqlite3_column_text(stmt,0);
					cats->push_back(cat);
					continue;
				}
			case SQLITE_DONE:
				loop = false;
				break;
			}
		}
		sqlite3_finalize(stmt);
		return true;
	}
	return false;
}

int __cdecl CSQLiteImpl::cbQeuryCallback(void* ud,int argc,char** argv,char** col)
{
	enum {kAbort=1,kContinue=0};
	auto scs = static_cast<SqliteCallbackStruct*>(ud);
	if(scs->type == SqliteCallbackStruct::kQueryCategory){
		auto pqc = static_cast<CBS_QueryCategory*>(scs);
		CIndexItem* pii = new CIndexItem;
		pii->idx      = atoi(argv[0]);
		pii->visible  = atoi(argv[3]);
		pii->times    = argv[7];
		pii->idxn     = argv[2];
		pii->category = argv[1];
		pii->comment  = argv[4];
		pii->path     = argv[5];
		pii->param    = argv[6];

		pqc->V->push_back(pii);
		return kContinue;
	}
	else if(scs->type == SqliteCallbackStruct::kQueryIndices){
		auto pqi = static_cast<CBS_QueryIndices*>(scs);
		CIndexItem* pii = new CIndexItem;
		pii->idx      = atoi(argv[0]);
		pii->visible  = atoi(argv[3]);
		pii->times    = argv[7];
		pii->idxn     = argv[2];
		pii->category = argv[1];
		pii->comment  = argv[4];
		pii->path     = argv[5];
		pii->param    = argv[6];

		pqi->indices->push_back(pii);
		//if(pii->idxn == pqi->find){
		if(_stricmp(pii->idxn.c_str(),pqi->find.c_str()) == 0){
			pqi->found = true;
			return kAbort;	
		}
		else{
			return kContinue;
		}
	}
	else{
		assert(0);
		return 1;
	}
}

bool CSQLiteImpl::QueryCategory(const char* cat,vector<CIndexItem*>* V)
{
	char* err;
	CBS_QueryCategory qcs;
	qcs.type = SqliteCallbackStruct::kQueryCategory;
	qcs.that = this;
	qcs.V = V;
	qcs.cat = cat;

	string sql("select * from tbl_index where category==\'");
	sql += cat;
	sql += "\' order by times desc;";

	if(sqlite3_exec(m_db,sql.c_str(),cbQeuryCallback,&qcs,&err) == SQLITE_OK){
		return true;
	}
	else{
		string e(err);
		sqlite3_free(err);
		throw CExcept(e.c_str(),"CSQLiteImpl::QueryCategory()");
	}
	return false;
}

bool CSQLiteImpl::QueryIndices(const char* find,vector<CIndexItem*>* R,bool* found)
{
	char* err;
	CBS_QueryIndices qi;
	qi.type = SqliteCallbackStruct::kQueryIndices;
	qi.that = this;
	qi.found = false;
	qi.indices = R;
	qi.find = find;

	string sql("select * from tbl_index where name like \'");
	sql += find;
	sql += "%\';";

	int rv = sqlite3_exec(m_db,sql.c_str(),cbQeuryCallback,&qi,&err);
	if(rv==SQLITE_OK || rv==SQLITE_ABORT){
		*found = qi.found;
		return true;
	}
	else{
		string e(err);
		sqlite3_free(err);
		throw CExcept(e.c_str(),"CSQLiteImpl::QueryIndices()");
	}
	return false;
}

bool CSQLiteImpl::FreeVectoredIndexItems(vector<CIndexItem*>* R)
{
	auto s = R->begin();
	auto e = R->end();
	while( s != e ){
		delete *s;
		++s;
	}
	return true;
}

bool CSQLiteImpl::AddItem(CIndexItem* pii)
{
	bool bnew = pii->idx == -1;
	stringstream ss;

	int rv;
	char* err;
	if(bnew){
		ss << "insert into tbl_index (category,name,visible,comment,path,param,times) values ("
			<< "\'" << pii->category	<< "\',"
			<< "\'" << pii->idxn		<< "\',"
			<<		pii->visible		<< ","
			<< "\'" << pii->comment		<< "\',"
			<< "\'" << pii->path		<< "\',"
			<< "\'" << pii->param		<< "\',"
			<< "\'" <<pii->times << "\');";
		string sql(ss.str());
		rv = sqlite3_exec(m_db,sql.c_str(),nullptr,nullptr,&err);
		if( rv != SQLITE_OK){
			string t(err);
			sqlite3_free(err);
			throw CExcept("添加到数据库失败!","CSQLiteImpl::AddItem()");
		}
		pii->idx = (int)sqlite3_last_insert_rowid(m_db);
		return true;
	}
	else{
		ss << "update tbl_index set category=\'" << pii->category << "\',"
			<< "name=\'"	<< pii->idxn		<< "\',"
			<< "visible="	<< pii->visible		<< ","
			<< "comment=\'"	<< pii->comment		<< "\',"
			<< "path=\'"	<< pii->path		<< "\',"
			<< "param=\'"	<< pii->param		<< "\',"
			<< "times="		<< pii->times		<< " "
			<< "where idx="	<< pii->idx			<< ";";
		string sql(ss.str());
		rv = sqlite3_exec(m_db,sql.c_str(),nullptr,nullptr,&err);
		if(rv != SQLITE_OK){
			string t(err);
			sqlite3_free(err);
			throw CExcept("更新数据库失败!","CSQLiteImpl::AddItem()");
		}
		return true;
	}
	return false;
}

bool CSQLiteImpl::DeleteItem(int idx)
{
	stringstream ss;
	ss << "delete from tbl_index where idx=";
	ss << idx << ";";
	string sql(ss.str());

	int rv;
	char* err;
	rv = sqlite3_exec(m_db,sql.c_str(),nullptr,nullptr,&err);
	if(rv != SQLITE_OK){
		string t(err);
		sqlite3_free(err);
		throw CExcept("删除失败!",__FUNCTION__);
	}
	else{
		return true;
	}
}

bool CSQLiteImpl::UpdateTimes(CIndexItem* pii)
{
	stringstream ss;
	ss << "update tbl_index set times="
		<< pii->times 
		<< " where idx="<< pii->idx;
	string str(ss.str());

	int rv;
	char* err;
	rv = sqlite3_exec(m_db,str.c_str(),nullptr,nullptr,&err);
	if(rv != SQLITE_OK){
		string t(err);
		sqlite3_free(err);
		throw CExcept("更新次数失败!","CSQLiteImpl::UpdateTimes()");
	}
	else{
		return true;
	}
}

bool CSQLiteImpl::RenameCategory(const char* from, const char* to)
{
	assert(strchr(from,'\'')==0);
	assert(strchr(to,'\'')==0);

	stringstream ss;
	ss << "update tbl_index set category=\'"
		<< to
		<< "\' "
		<< "where category=\'"
		<< from
		<< "\';";
	string str(ss.str());
	int rv;
	char* err;
	rv = sqlite3_exec(m_db,str.c_str(),0,0,&err);
	if(rv!=SQLITE_OK){
		string t(err);
		sqlite3_free(err);
		throw CExcept("重命名分类失败!",__FUNCTION__);
	}
	else{
		return true;
	}
}
