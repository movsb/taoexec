#include "Button.h"

AButton::AButton(void)
{
	m_hImage = NULL;
}


AButton::~AButton(void)
{
	if(m_hImage) DeleteBitmap(m_hImage);
}

int AButton::GetCheck()
{
	return SendMessage(BM_GETCHECK);
}

void AButton::SetCheck(WPARAM wState)
{
	SendMessage(BM_SETCHECK,wState);
}

HANDLE AButton::SetImage(CHAR* image)
{
	if(m_hImage) DeleteBitmap(m_hImage);
	m_hImage = ::LoadImage(NULL,image,IMAGE_BITMAP,42,42,LR_DEFAULTCOLOR|LR_LOADFROMFILE);
	return (HANDLE)SendMessage(BM_SETIMAGE,IMAGE_BITMAP,LPARAM(m_hImage));
}
