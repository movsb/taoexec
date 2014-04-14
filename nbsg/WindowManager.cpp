#include "WindowManager.h"
#include "WindowBase.h"

void AWindowManager::insert(AWindowBase* pWindow)
{
	m_Windows.push_back(pWindow);
}

void AWindowManager::remove(AWindowBase* pWindow)
{
	for(std::vector<AWindowBase*>::iterator it=m_Windows.begin();
		it!=m_Windows.end();
		it++
	)
	{
		if(*it == pWindow){
			m_Windows.erase(it);
			break;
		}
	}
}

bool AWindowManager::empty()
{
	return true;
}
