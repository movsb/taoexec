#ifndef __SQLITE_H__
#define __SQLITE_H__

#include <Windows.h>
#include "sqlite3/sqlite3.h"
#include "Thunk.h"

#include <cassert>
#include <string>

using namespace std;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class ASqliteBase
{
public:
	ASqliteBase()
	{
		_zerr = NULL;
		_pdb = NULL;
		_pStmt = NULL;
	}
	virtual ~ASqliteBase()
	{
		
	}
public:
	bool open(const char* zdb);
	bool close();
	sqlite3* getPdb() const{assert(_pdb!=NULL);return _pdb;}
	void sethParent(HWND hParent){_hParent=hParent;}

protected:
	char* _zerr;
	sqlite3* _pdb;
	sqlite3_stmt* _pStmt;
	HWND _hParent;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class AIndexSqlite:public ASqliteBase
{
public:
	AIndexSqlite()
	{
		*m_zTableName = '\0';
	}
	~AIndexSqlite()
	{
	}

public:
	struct SQLITE_INDEX{
		string idx;
		string index;
		string comment;
		string path;
		string param;
		string times;
	};

public:
	bool attach(HWND hParent,sqlite3* pdb);
	void makeIndex(SQLITE_INDEX* psi,const char** argv);
	bool add(SQLITE_INDEX* psi);
	//bool updateTimes(char* idx,int orig_times);
	bool UpdateTimes(const char* idx, unsigned int new_times=-1);
	bool deleteIndex(const char* idx);
	bool search(const char* zIndex,void* pv,sqlite3_callback cb);
	void setTableName(const char* zTableName);
	const char* getTableName() const;

private:
	char m_zTableName[32];
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class ASettingsSqlite:public ASqliteBase
{
public:
	bool attach(HWND hParent,sqlite3* pdb);
	ASettingsSqlite();
	~ASettingsSqlite();

public:
	bool getSetting(const char* item,void** ppv,int* size);
	bool setSetting(const char* item,void* pv,int  size);

private:
	
};


#endif//!__SQLITE_H__
