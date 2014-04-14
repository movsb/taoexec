#include <iostream>
#include <sstream>
#include "SQLite.h"
#include "nbsg.h"
#include "Str.h"
#include "PathLib.h"
#include "Utils.h"
#include "debug.h"

/****************************Sqlite 基类实现***********************************/
bool ASqliteBase::open(const char* zdb)
{
	assert(_pdb == NULL);
	if(_pdb) return false;

	if(sqlite3_open(zdb,&_pdb) != SQLITE_OK){
		AUtils::msgbox(_hParent,MB_ICONSTOP,g_pApp->getAppName(),"初始化sqlite3失败!");
		return false;
	}else{
		return true;
	}
}

/***********************************************************************
名称:close@-
描述:关闭sqlite3数据库
参数:
返回:bool
说明:
***********************************************************************/
bool ASqliteBase::close()
{
	assert(_pdb != NULL);
	if(_pdb){
		if(sqlite3_close(_pdb) != SQLITE_OK){
			AUtils::msgbox(_hParent,MB_ICONSTOP,NULL,"sqlite3_close() error:%s",sqlite3_errmsg(_pdb));
			return false;
		}else{
			_pdb = NULL;
			return true;
		}
	}
	return false;
}

///////////////////////AIndexSqlite 实现部分 开始//////////////////////////////////
void AIndexSqlite::setTableName(const char* zTableName)
{
	strncpy(m_zTableName,zTableName,sizeof(m_zTableName)/sizeof(*m_zTableName));
}

const char* AIndexSqlite::getTableName() const
{
	return m_zTableName;
}

void AIndexSqlite::makeIndex(SQLITE_INDEX* psi,const char** argv)
{
	psi->idx     = argv[0];
	psi->index   = AStr(argv[1],true).toAnsi();
	psi->comment = AStr(argv[2],true).toAnsi();
	psi->path    = AStr(argv[3],true).toAnsi();
	psi->param   = AStr(argv[4],true).toAnsi();
	psi->times   = argv[5];
}

/**********************************************************************
名称:attach
描述:初始化数据库:打开,新建表
参数:
返回:bool成功与否
说明:
***********************************************************************/
bool AIndexSqlite::attach(HWND hParent,sqlite3* pdb)
{
	this->_pdb = pdb;
	this->_hParent = hParent;

	assert(::IsWindow(hParent) && pdb!=NULL && *m_zTableName!='\0');

	std::string sql = "create table if not exists ";
	sql += this->m_zTableName;
	sql += " (idx integer primary key,idxn text,comment text,path text,param text,times integer default 0)";
	if(sqlite3_exec(_pdb,sql.c_str(),NULL,NULL,&_zerr) != SQLITE_OK){
		AUtils::msgbox(_hParent,MB_ICONSTOP,NULL,"[sqlite.init] creation error!\n");
		sqlite3_free(_zerr);
		return false;
	}
	return true;
}

/***********************************************************************
名称:update_times@8
描述:更新某index的使用次数
参数:index-索引名; orig_times-原来的次数
返回:bool
说明:
***********************************************************************/
// bool AIndexSqlite::updateTimes(char* idx,int orig_times)
// {
// 	std::stringstream ss;
// 	bool ret;
// 	ss<<"update "<<m_zTableName<<" set times="<<orig_times+1<<" where idx="<<idx<<";";
// 	if(sqlite3_exec(_pdb,string(ss.str()).c_str(),NULL,NULL,&_zerr) != SQLITE_OK){
// 		AUtils::msgbox(_hParent,MB_ICONSTOP,"sqlite3_exec() error:%s",_zerr);
// 		sqlite3_free(_zerr);
// 		ret = false;
// 	}else{
// 		ret = true;
// 	}
// 	return ret;
// }
bool AIndexSqlite::UpdateTimes(const char* idx,unsigned int new_times)
{
	bool rv;
	stringstream ss;
	ss.clear();
	if(new_times != -1){
		ss<<"update "<<m_zTableName<<" set times="<<new_times<<" where idx="<<idx<<";";
	}else{
		ss<<"update "<<m_zTableName<<" set times=times+1 where idx="<<idx<<";";
	}
	if(sqlite3_exec(_pdb,string(ss.str()).c_str(),0,0,&_zerr) != SQLITE_OK){
		AUtils::msgbox(_hParent,MB_ICONSTOP,"sqlite3_exec() error:%s",_zerr);
		sqlite3_free(_zerr);
		rv = false;
	}else{
		rv = true;
	}
	return rv;
}

/***********************************************************************
名称:delete_idx@4
描述:删除指定索引号的条目
参数:idx-索引号对应的字符串
返回:!0-成功; 0-失败
说明:
***********************************************************************/
bool AIndexSqlite::deleteIndex(const char* idx)
{
	bool ret;
	std::string str = "delete from ";
	str += m_zTableName;
	str += " where idx=";
	str += idx;

	if(sqlite3_exec(_pdb,str.c_str(),NULL,NULL,&_zerr)!=SQLITE_OK){
		AUtils::msgbox(_hParent,MB_ICONSTOP,NULL,"sqlite::delete() 遇到错误:%s",_zerr);
		sqlite3_free(_zerr);
		ret = false;
	}else{
		ret = true;
	}
	return ret;
}

bool AIndexSqlite::search(const char* zIndex,void* pv,sqlite3_callback cb)
{
	int ret;
	bool r;
	std::string str;

	assert(zIndex != NULL);

	if(!*zIndex) return true;

	if(*zIndex=='*'){
		str = "select * from ";
		str += m_zTableName;
		str +=" order by times desc;";
	}else{
		str = "select * from ";
		str += m_zTableName;
		str += " where idxn like \'";
		str += zIndex;
		str += "%\' order by times desc;";
	}
	ret = sqlite3_exec(_pdb,AStr(str.c_str(),false).toUtf8(),cb,pv,&_zerr);
	if(ret!=SQLITE_OK && ret!=SQLITE_ABORT){
		AUtils::msgbox(_hParent,MB_ICONSTOP,NULL,"sqlite::search error:%s",_zerr);
		sqlite3_free(_zerr);
		r = false;
	}
	r = true;
	return r;
}

/***********************************************************************
名称:add@4
描述:增加或修改条目
参数:
返回:
说明:若*idx=='.', 表示新增
***********************************************************************/
bool AIndexSqlite::add(SQLITE_INDEX* psi)
{
	bool bAddNew = psi->idx[0]=='.';

	stringstream ss;
	if(bAddNew){
		ss<<"insert into "<<m_zTableName
			<<" (idxn,comment,path,param,times) values (\'"
			<<psi->index<<"\',\'"
			<<psi->comment<<"\',\'"
			<<psi->path<<"\',\'"
			<<psi->param<<"\',"
			<<psi->times<<");"
		;
	}else{
		ss<<"update "<<m_zTableName
			<<" set "<<"idxn=\'"<<psi->index
			<<"\',comment=\'"<<psi->comment
			<<"\',path=\'"<<psi->path
			<<"\',param=\'"<<psi->param
			<<"\',times="<<psi->times
			<<" where idx="<<psi->idx<<";"
		;
	}

	if(sqlite3_exec(_pdb,AStr(string(ss.str()).c_str(),false).toUtf8(),NULL,NULL,&_zerr) != SQLITE_OK){
		AUtils::msgbox(_hParent,MB_ICONSTOP,NULL,"sqlite3_exec() error:%s",_zerr);
		sqlite3_free(_zerr);
		return false;
	}
	if(bAddNew){
		AUtils::msgbox(_hParent,MB_ICONINFORMATION,"","添加成功:%s - %s",psi->index.c_str(),psi->comment.c_str());
		char t[32];
		_snprintf(t,sizeof(t),"%u",sqlite3_last_insert_rowid(_pdb));
		psi->idx = t;
	}else{
		AUtils::msgbox(_hParent,MB_ICONINFORMATION,"","修改成功:%s - %s",psi->index.c_str(),psi->comment.c_str());
	}
	return true;
}
///////////////////////AIndexSqlite 实现部分 结束//////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
ASettingsSqlite::ASettingsSqlite()
{

}

ASettingsSqlite::~ASettingsSqlite()
{

}

bool ASettingsSqlite::attach(HWND hParent,sqlite3* pdb)
{
	this->_pdb = pdb;
	this->_hParent = hParent;

	assert(::IsWindow(hParent) && pdb!=NULL);

	std::string sql = "create table if not exists settings (idx integer primary key,name text,data blob);";
	if(sqlite3_exec(_pdb,sql.c_str(),NULL,NULL,&_zerr) != SQLITE_OK){
		AUtils::msgbox(_hParent,MB_ICONSTOP,NULL,"[sqlite.init] creation error!\n");
		sqlite3_free(_zerr);
		return false;
	}
	return true;
}

bool ASettingsSqlite::getSetting(const char* item,void** ppv,int* size)
{
	bool ret=false;
	std::string sql = "select * from settings where name=\'";
	sql += item;
	sql += "\';";

	if(sqlite3_prepare(_pdb,sql.c_str(),-1,&_pStmt,NULL) == SQLITE_OK){
		if(sqlite3_step(_pStmt) == SQLITE_ROW){
			int sz = sqlite3_column_bytes(_pStmt,2);
			if(sz>0){
				char* p = new char[sz];
				memcpy(p,sqlite3_column_blob(_pStmt,2),sz);
				*ppv = p;
				*size = sz;
				ret = true;
			}
		}
		sqlite3_finalize(_pStmt);
	}
	return ret;
}

bool ASettingsSqlite::setSetting(const char* item,void* pv,int  size)
{
	bool ret=false;
	int id;
	std::stringstream ss;
	std::string sql = "select * from settings where name=\'";
	sql += item;
	sql += "\';";

	if(sqlite3_prepare(_pdb,sql.c_str(),-1,&_pStmt,NULL) == SQLITE_OK){
		switch(sqlite3_step(_pStmt))
		{
		case SQLITE_DONE:
			ss<<"insert into settings (name,data) values (\'"<<item<<"\',\?);";
			break;
		case SQLITE_ROW:
			id = sqlite3_column_int(_pStmt,0);
			ss<<"update settings set data=\? where idx="<<id<<";";
			break;
		default:
			sqlite3_finalize(_pStmt);
			return false;
		}
		sqlite3_finalize(_pStmt);
		sql = ss.str();
		if(sqlite3_prepare(_pdb,sql.c_str(),-1,&_pStmt,NULL) == SQLITE_OK){
			if(sqlite3_bind_blob(_pStmt,1,pv,size,NULL) == SQLITE_OK){
				if(sqlite3_step(_pStmt) == SQLITE_DONE){
					ret = true;
				}
			}
		}
	}
	sqlite3_finalize(_pStmt);
	return ret;
}
