#include "WindowBase.h"

#include <vector>

class AMainDlg:public AWindowBase
{
public:
	AMainDlg(const char* cmd,bool show);
	~AMainDlg();
	bool create(const char* cmd,bool nShowCmd);

public:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM lParam);
	INT_PTR OnClose();
	INT_PTR OnNCDestroy();
	INT_PTR OnSize(int width,int height);
	INT_PTR OnSizing(WPARAM wParam,LPRECT pRect);
	INT_PTR OnNotify(LPNMHDR phdr);
	INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl);
	INT_PTR OnHotKey(WPARAM id);
	INT_PTR OnUser(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnApp(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnNull(LPARAM lParam);

public:
	void AddNewTab(AWindowBase* pWindow,const char* szTabName);
	void AdjustTabPageSize(AWindowBase* pWindow=NULL);
	void ShowTabPage(int nPage);
	void SetAutoHide(BOOL bAutoHide);

public:
	ATabCtrl*	m_pTabCtrl;
	AMini*		m_pMini;

private:
	struct TabInfo
	{
		AWindowBase* pWindow;
		int iItem;
	};
	typedef std::vector<TabInfo*> TabPages;
	typedef std::vector<TabInfo*>::iterator TabPagesIter;
private:
	BOOL m_bAutoHide;
	TabPages m_Pages;
	static RECT m_RectWnd;
	bool bAutoHide;
};
