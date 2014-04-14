#pragma once
#include <Windows.h>
#include "App.h"
#include "SQLite.h"
#include "WindowManager.h"
#ifndef NBSG_CPP
extern ASqliteBase* g_pSqliteBase;
extern AApp* g_pApp;
extern AWindowManager* g_pWindowManager;
#else
#undef NBSG_CPP
#endif
