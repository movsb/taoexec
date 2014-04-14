#include "reg.h"

AReg::AReg(HKEY hKeyRoot,LPCTSTR lpSubKey):
	m_hKey(NULL),
	m_LastError(0)
{
	LONG result = RegOpenKeyEx(hKeyRoot,lpSubKey,0,KEY_ALL_ACCESS,&m_hKey);
	if(result == ERROR_SUCCESS){
		m_LastError = 0;
	}else{
		m_LastError = result;
	}
}

AReg::AReg(HKEY hKey):
	m_hKey(NULL),
	m_LastError(0)
{
	m_hKey = hKey;
}

AReg::~AReg()
{
	if(m_hKey){
		CloseKey();
	}
}

bool AReg::CreateKey(IN LPCTSTR lpSubKey,IN OPTIONAL PHKEY phkResult)
{
	HKEY hKey;
	bool ret=false;

	LONG result = RegCreateKeyEx(m_hKey,lpSubKey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,NULL);
	if(result == ERROR_SUCCESS){
		m_LastError = 0;

		if(phkResult == NULL){
			RegCloseKey(hKey);
		}else{
			*phkResult = hKey;
		}
		ret = true;
	}else{
		m_LastError = result;

		if(phkResult == NULL){

		}else{
			*phkResult = NULL;
		}
		ret = false;
	}
	return ret;
}

bool AReg::CloseKey(void)
{
	//系统根HKEY项都大于0x80000000是吗?

	if((DWORD)m_hKey & 0x80000000){
		return true;
	}

	LONG result = RegCloseKey(m_hKey);
	if(result==ERROR_SUCCESS){
		m_hKey = NULL;
		m_LastError = 0;
		return true;
	}else{
		m_LastError = result;
		return false;
	}
}

bool AReg::DeleteKey(IN LPTSTR lpSubKeys)
{
	//lpSubKeys:subkey1\subkey2\subkey3
	bool ret = false;
	LONG result;
	TCHAR* p = lpSubKeys;
	while(*p) p++;
	p--;
	for(;;){
		result = RegDeleteKey(m_hKey,lpSubKeys);
		if(result != ERROR_SUCCESS){
			if(result != ERROR_FILE_NOT_FOUND){
				m_LastError = result;
				break;
			}
		}

		while(*p!=TEXT('\\') && p>lpSubKeys){
			p--;
		}
		if(*p == TEXT('\\')){
			*p = TEXT('\0');
			if(p > lpSubKeys){
				continue;
			}else if(p == lpSubKeys){
				break;
			}
		}else if(p == lpSubKeys){
			break;
		}
	}

	return ret;
}

bool AReg::DeleteValue(IN OPTIONAL LPCTSTR lpValueName)
{
	LONG result = RegDeleteValue(m_hKey,lpValueName);
	if(result == ERROR_SUCCESS){
		m_LastError = 0;
		return true;
	}else{
		m_LastError = result;
		return false;
	}
}

bool AReg::QueryValue(IN OPTIONAL LPCTSTR lpValueName,OUT OPTIONAL LPDWORD lpType,OUT OPTIONAL LPBYTE lpData,IN OUT OPTIONAL LPDWORD lpcbData)
{
	LONG result = RegQueryValueEx(m_hKey,lpValueName,0,lpType,lpData,lpcbData);
	if(result == ERROR_SUCCESS){
		m_LastError = 0;
		return true;
	}else{
		m_LastError = result;
		return false;
	}
}

bool AReg::SetValue(IN OPTIONAL LPCTSTR lpValueName,IN DWORD dwType,IN const BYTE* lpData,IN DWORD cbData)
{
	LONG result = RegSetValueEx(m_hKey,lpValueName,0,dwType,lpData,cbData);
	if(result == ERROR_SUCCESS){
		m_LastError = 0;
		return true;
	}else{
		m_LastError = result;
		return false;
	}
}

