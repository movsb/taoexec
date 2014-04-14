# Microsoft Developer Studio Project File - Name="nbsg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=nbsg - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nbsg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nbsg.mak" CFG="nbsg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nbsg - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "nbsg - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nbsg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nbsg___Win32_Release"
# PROP BASE Intermediate_Dir "nbsg___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../output/nbsg_release_vc6"
# PROP Intermediate_Dir "../output/nbsg_release_vc6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D _WIN32_WINNT=0x0502 /FR /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib version.lib sqlite3/sqlite3.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"../bin/nbsg_vc6.exe" /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nbsg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nbsg___Win32_Debug"
# PROP BASE Intermediate_Dir "nbsg___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../output/nbsg_debug_vc6"
# PROP Intermediate_Dir "../output/nbsg_debug_vc6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D _WIN32_WINNT=0x0502 /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib version.lib sqlite3/sqlite3.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../bin/nbsg_debug_vc6.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "nbsg - Win32 Release"
# Name "nbsg - Win32 Debug"
# Begin Group "class"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;asm"
# Begin Source File

SOURCE=.\App.cpp
# End Source File
# Begin Source File

SOURCE=.\App.h
# End Source File
# Begin Source File

SOURCE=.\debug.c
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\Encode.cpp
# End Source File
# Begin Source File

SOURCE=.\encode.h
# End Source File
# Begin Source File

SOURCE=.\Memory.cpp
# End Source File
# Begin Source File

SOURCE=.\Memory.h
# End Source File
# Begin Source File

SOURCE=.\nbsg.cpp
# End Source File
# Begin Source File

SOURCE=.\nbsg.h
# End Source File
# Begin Source File

SOURCE=.\PathLib.cpp
# End Source File
# Begin Source File

SOURCE=.\PathLib.h
# End Source File
# Begin Source File

SOURCE=.\plugin.cpp
# End Source File
# Begin Source File

SOURCE=.\plugin.h
# End Source File
# Begin Source File

SOURCE=.\reg.cpp
# End Source File
# Begin Source File

SOURCE=.\reg.h
# End Source File
# Begin Source File

SOURCE=.\SQLite.cpp
# End Source File
# Begin Source File

SOURCE=.\sqlite.h
# End Source File
# Begin Source File

SOURCE=.\Str.cpp
# End Source File
# Begin Source File

SOURCE=.\Str.h
# End Source File
# Begin Source File

SOURCE=.\tips.c
# End Source File
# Begin Source File

SOURCE=.\tips.h
# End Source File
# Begin Source File

SOURCE=.\Utils.cpp
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\100.ico
# End Source File
# Begin Source File

SOURCE=.\nbsg.rc
# End Source File
# End Group
# Begin Group "Window"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AddDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AddDlg.h
# End Source File
# Begin Source File

SOURCE=.\Button.cpp
# End Source File
# Begin Source File

SOURCE=.\Button.h
# End Source File
# Begin Source File

SOURCE=.\ChildFileDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFileDlg.h
# End Source File
# Begin Source File

SOURCE=.\ChildIndexDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildIndexDlg.h
# End Source File
# Begin Source File

SOURCE=.\ChildSettingsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildSettingsDlg.h
# End Source File
# Begin Source File

SOURCE=.\EditBox.cpp
# End Source File
# Begin Source File

SOURCE=.\EditBox.h
# End Source File
# Begin Source File

SOURCE=.\ListView.cpp
# End Source File
# Begin Source File

SOURCE=.\ListView.h
# End Source File
# Begin Source File

SOURCE=.\MainDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MainDlg.h
# End Source File
# Begin Source File

SOURCE=.\Mini.cpp
# End Source File
# Begin Source File

SOURCE=.\mini.h
# End Source File
# Begin Source File

SOURCE=.\MoveToDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MoveToDlg.h
# End Source File
# Begin Source File

SOURCE=.\ShellContextMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\ShellContextMenu.h
# End Source File
# Begin Source File

SOURCE=.\TabCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\TabCtrl.h
# End Source File
# Begin Source File

SOURCE=.\Thunk.cpp
# End Source File
# Begin Source File

SOURCE=.\Thunk.h
# End Source File
# Begin Source File

SOURCE=.\WindowBase.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowBase.h
# End Source File
# Begin Source File

SOURCE=.\WindowManager.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowManager.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\xps.manifest
# End Source File
# End Target
# End Project
