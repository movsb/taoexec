#pragma once

class CSQLiteImpl;

struct CIndexItem
{
	int idx;
	int visible;
	string category;
	string idxn;
	string comment;
	string path;
	string param;
	string times;

	CIndexItem()
	{
		idx = -1;
		visible = 1;
		times = "0";
	}
};

class CSQLite
{
public:
	CSQLite();
	~CSQLite();
	bool Open(const char* fn);
	bool Close();
	bool QueryCategory(const char* cat,vector<CIndexItem*>* V);
	bool GetCategories(vector<string>* cats);
	bool RenameCategory(const char* from, const char* to);
	bool QueryIndices(const char* find,vector<CIndexItem*>* R,bool* found);
	bool FreeVectoredIndexItems(vector<CIndexItem*>* R);
	bool AddItem(CIndexItem* pii);
	bool UpdateTimes(CIndexItem* pii);
	
private:
	CSQLiteImpl* m_sqlite;
};
