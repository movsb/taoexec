#pragma once

class CDropFiles
{
public:
	CDropFiles(HDROP hDrop,std::vector<std::string>* files);
};

class CCharset
{
public:
	static bool Acp2Unicode(std::string& str,std::wstring* wstr);
};

// Yet Another Garbage Collector
class yagc
{
public:
	yagc(void* ptr,bool (*func)(void* ptr))
	{
		this->ptr = ptr;
		this->pfunc = func;
	}
	~yagc()
	{
		bool r = pfunc(ptr);
#ifdef _DEBUG
		if(!r){
			::MessageBox(nullptr,"yagc failed!",nullptr,MB_ICONERROR);
			//assert(0);
		}
#endif
	}
private:
	void* ptr;
	bool (*pfunc)(void* ptr);
};

// class __yagc_base
// {
// public:
// 	__yagc_base(){}
// 	virtual ~__yagc_base(){}
// };
// 
// template<class TT,class DD>
// class __yagc : public __yagc_base
// {
// public:
// 	__yagc(TT t, DD d) : m_t(t),m_d(d){}
// 	~__yagc(){m_d(m_t);}
// private:
// 	TT m_t;
// 	DD m_d;
// };
// 
// template <class T>
// class yagc
// {
// public:
// 	template<class T,class D>
// 	yagc(T t, D d){baseptr = new __yagc<T,D>(t,d);}
// 	~yagc(){delete baseptr;}
// 
// private:
// 	__yagc_base* baseptr;
// };
