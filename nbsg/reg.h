#include <windows.h>

class AReg
{
public:
	AReg(HKEY hKeyRoot,LPCTSTR lpSubKey);
	AReg(HKEY hKey);
	~AReg();
	operator bool(){return m_LastError==ERROR_SUCCESS;}

public:
	bool CreateKey(IN LPCTSTR lpSubKey,PHKEY phkResult=NULL);
	bool CloseKey(void);
	bool DeleteKey(IN LPTSTR lpSubKeys);
	bool DeleteValue(IN OPTIONAL LPCTSTR lpValueName);
	bool QueryValue(IN OPTIONAL LPCTSTR lpValueName,OUT OPTIONAL LPDWORD lpType,OUT OPTIONAL LPBYTE lpData,IN OUT OPTIONAL LPDWORD lpcbData);
	bool SetValue(IN OPTIONAL LPCTSTR lpValueName,IN DWORD dwType,IN const BYTE* lpData,IN DWORD cbData);

private:
	LONG m_LastError;
	HKEY m_hKey;
};
