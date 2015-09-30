#include <cctype>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>

#include <windows.h>
#include <ShlObj.h>

#include "model.h"
#include "charset.h"
#include "window.h"

#undef min
#undef max

static std::map<std::string, std::string> g_variables;
typedef std::vector<std::string> func_args;
static std::map<std::string, std::function<std::string(func_args& args)>> g_functions;

bool is_64bit() {
    BOOL b64;
    return IsWow64Process(GetCurrentProcess(), &b64)
        && b64 != FALSE;
}

void initialize_globals() {
    auto initialize_variables = []() {
        char path[MAX_PATH];

        // the exe directory
        if (GetModuleFileName(NULL, path, _countof(path)) > 0) {
            *strrchr(path, '\\') = '\0';
            g_variables["exe_dir"] = path;
        }

        // the Windows directory
        path[GetWindowsDirectory(path, _countof(path))] = '\0';
        if (path[3] == '\0') path[2] = '\0'; // on root drive, removes backslash
        g_variables["windows"] = path;

        // the Current Directory
        path[GetCurrentDirectory(_countof(path), path)] = '\0';
        if (path[3] == '\0') path[2] = '\0'; // on root drive, removes backslash
        g_variables["cd"] = path;

        // the System directory
        path[GetSystemDirectory(path, _countof(path))] = '\0';
        if (path[3] == '\0') path[2] = '\0'; // on root drive, removes backslash
        g_variables["system"] = path;

        // the AppData
        if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
            g_variables["appdata"] = path;

        // the home
        if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, path)))
            g_variables["home"] = path;

        // the desktop
        if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, path)))
            g_variables["desktop"] = path;

        // the Program Files (x86)
        if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, path)))
            g_variables["program_x86"] = path;

        // the Program Files
        if (GetEnvironmentVariable("ProgramW6432", path, _countof(path)) > 0)
            g_variables["program"] = path;
    };

    auto initialize_functions = []() {
        g_functions["reg"] = [](func_args& args)->std::string {
            std::string result;
            if (args.size() >= 3) {
                static std::map<std::string, HKEY> _hkeys{
                    { "HKEY_CLASS_ROOT", HKEY_CLASSES_ROOT },
                    { "HKCR", HKEY_CLASSES_ROOT },
                    { "HKEY_CURRENT_USER", HKEY_CURRENT_USER },
                    { "HKCU", HKEY_CURRENT_USER },
                    { "HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE },
                    { "HKLM", HKEY_LOCAL_MACHINE },
                };

                if (_hkeys.count(args[0])) {
                    char value[2048];
                    DWORD cb = sizeof(value);

                    REGSAM sam = KEY_READ;
                    if (is_64bit()) {
                        if (args.size() >= 4 && args[3] == "64")
                            sam |= KEY_WOW64_64KEY;
                        else
                            sam |= KEY_WOW64_32KEY;
                    }

                    HKEY hkey;
                    if (RegOpenKeyEx(_hkeys[args[0]], NULL, 0, sam, &hkey) == ERROR_SUCCESS) {
                        if (RegGetValue(hkey, args[1].c_str(), args[2].c_str(),
                            RRF_RT_REG_SZ, nullptr, (void*)value, &cb) == ERROR_SUCCESS) {
                            result = value;
                        }
                        RegCloseKey(hkey);
                    }
                }
            }
            return result;
        };

        g_functions["env"] = [](func_args& args)->std::string {
            std::string result;
            if (args.size() >= 1) {
                char buf[2048];
                if (GetEnvironmentVariable(args[0].c_str(), buf, _countof(buf)) > 0) {
                    result = buf;
                }
            }

            return result;
        };

        g_functions["app_path"] = [](func_args& args)->std::string {
            std::string result;
            auto reg = g_functions["reg"];

            if (args.size() >= 1) {
                func_args as{
                    "HKLM",
                    "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\" + args[0],
                    ""
                };

                result = reg(as);
            }

            return result;
        };
    };

    initialize_variables();
    initialize_functions();
}

std::string expand_variable(const std::string& var) {
    if (g_variables.count(var) > 0)
        return g_variables[var];

    return "";
}

std::string expand_function(const std::string& fn, func_args& args) {
    if (g_functions.count(fn))
        return g_functions[fn](args);

    return "";
}

std::string expand(const std::string& raw) {
    std::string result;
    const char* p = raw.c_str();

    auto read_arg = [](const char*& p) {
        std::string s;
        while (*p && *p != ',' && *p != ')') {
            s += *p;
            p++;
        }

        return s;
    };

    auto read_ident = [](const char*& p) {
        std::string s;
        while (::isalnum(*p) || strchr("_", *p)) {
            s += *p;
            p++;
        }

        return s;
    };

    while (*p != '\0') {
        if (*p == '$') {
            p++;
            if (*p == '{') {
                p++;
                auto var = read_ident(p);
                if (*p++ != '}') {
                    // assert(0);
                }

                result += expand_variable(var);
            }
            else if (::isalpha(*p)) {
                auto fn = read_ident(p);
                if (*p++ != '(') {
                    // assert(0);
                }

                func_args args;
                while ((true)) {
                    std::string arg(read_arg(p));
                    args.push_back(arg);

                    if (*p == ',') {
                        p++;
                        continue;
                    }
                    else if (*p == ')') {
                        break;
                    }
                }

                if (*p++ != ')') {
                    // assert(0);
                }

                result += expand_function(fn, args);
            }
            else {
                // assert(0);
                p++;
            }
        }
        else {
            result += *p;
            p++;
        }
    }
    return result;
}

nbsg::model::db_t* pdb;

int main() {
    const char* paths[] = {
        "${exe_dir}",
        "${windows}",
        "${cd}",
        "${system}",
        "${appdata}",
        "${home}",
        "${program}",
        "${program_x86}",
        "${desktop}",
        "${windows}\\explorer.exe",
        "$reg(HKLM,Software\\Vim\\Gvim,path,64)",
        "$env(USERPROFILE)",
        "$app_path(firefox.exe)",
    };

    //initialize_globals();

    //for (auto& path : paths) {
    //    std::cout << expand(path) << std::endl;
    //};

    /*
    nbsg::model::db_t db;
    pdb = &db;
    db.open(nbsg::charset::a2e(R"(中文.db)"));

    std::cout << sizeof(R"(中文.db)") << strlen(R"(中文.db)");

    nbsg::model::item_t item;
    item.index = "fx";
    item.comment = "firefox";
    item.group = "app";
    item.path = "firefox.exe";
    item.visibility = 1;
    db.insert(&item);
    */

    class nbsg_window : public nbsg::window::frame_window_t {
    public:
        LRESULT wndproc(UINT msg, WPARAM wp, LPARAM lp) override{
            if(msg == WM_CREATE) {
                /*
                auto _list = new nbsg::window::listview_child_t;
                _list->create(_hwnd, 0, "", { 10, 10, 790, 590 });

                LVCOLUMN col;
                col.mask = LVCF_TEXT | LVCF_WIDTH;
                col.cx = 100;
                col.pszText = "id";
                _list->insert_column(0, &col);
                col.pszText = "index";
                _list->insert_column(1, &col);
                col.pszText = "group";
                _list->insert_column(2, &col);
                col.pszText = "comment";
                _list->insert_column(3, &col);
                col.pszText = "path";
                _list->insert_column(4, &col);
                col.pszText = "params";
                _list->insert_column(5, &col);
                col.pszText = "work_dir";
                _list->insert_column(6, &col);
                col.pszText = "env";
                _list->insert_column(7, &col);
                col.pszText = "visibility";
                _list->insert_column(8, &col);

                auto int2str = [](int i) {
                char buf[20];
                sprintf(buf, "%d", i);
                return std::string(buf);
                };

                pdb->query("", [&](nbsg::model::item_t& item)->bool {
                return -1 != _list->insert_item(
                int2str(item.id),
                item.index,
                item.group,
                item.comment,
                item.path,
                item.params,
                item.work_dir,
                item.env,
                int2str(item.visibility)
                );
                });

                _children.add(_list);
                */

                _root = new nbsg::window::layout_container_t;

                nbsg::window::button_child_t* _btn;
                /*
                auto
                    
                    _btn = new nbsg::window::button_child_t;
                _btn->create(*this);
                _root->add(_btn);

                _btn = new nbsg::window::button_child_t;
                _btn->create(*this);
                _btn->_layout.flags |= 0
                    | nbsg::window::layout_object_t::attr::right
                    | nbsg::window::layout_object_t::attr::top
                    ;
                _btn->_layout.right = 0;
                _btn->_layout.top = 0;
                _btn->_layout.position = "absolute";

                _root->add(_btn);

                _btn = new nbsg::window::button_child_t;
                _btn->create(*this);
                _root->add(_btn);

                _btn = new nbsg::window::button_child_t;
                _btn->create(*this);
                _root->add(_btn);

                _btn = new nbsg::window::button_child_t;
                _btn->create(*this);
                _root->add(_btn);


                _btn = new nbsg::window::button_child_t;
                _btn->create(*this);
                _btn->_layout.display = "block";
                _root->add(_btn);

                _btn = new nbsg::window::button_child_t;
                _btn->create(*this);
                _root->add(_btn);*/

                _btn = new nbsg::window::button_child_t;
                _btn->create(*this,0,"3");
                _root->add(_btn);

                auto con = new nbsg::window::layout_container_t;
                con->create(*this);

                _btn = new nbsg::window::button_child_t;
                _btn->create(*this,0,"1");
                con->add(_btn);/*
                _btn = new nbsg::window::button_child_t;
                _btn->create(*this,0,"2");
                con->add(_btn);*/

                _root->add(con);

                
                _btn = new nbsg::window::button_child_t;
                _btn->create(*this,0,"4");
                _root->add(_btn);
                /*
                _btn = new nbsg::window::button_child_t;
                _btn->create(*this);
                _btn->_layout.position = "relative";
                _btn->_layout.flags |= nbsg::window::layout_object_t::attr::top
                    | nbsg::window::layout_object_t::attr::right;
                _btn->_layout.top = 0;
                _btn->_layout.right = 0;
                _root->add(_btn);

                _btn = new nbsg::window::button_child_t;
                _btn->create(*this);
                _root->add(_btn);*/
            }
            else if(msg == WM_SIZE) {
                int cw = (int)LOWORD(lp);
                int ch = (int)HIWORD(lp);

                static auto process_scrollbar = [&](int cw, int ch, int dw, int dh){
                    bool bSetVert = false, bSetHorz = false;

                    if(dh > ch)
                        bSetVert = true;

                    if(dw > cw)
                        bSetHorz = true;

                    SCROLLINFO si = {0};
                    si.cbSize = sizeof(si);
                    si.fMask = SIF_ALL;
                    si.nMin = 0;
                    si.nPos = 0;
                    si.nPage = 200;
                    if(bSetVert) {
                        si.nMax = dh - ch - 1 + si.nPage - 1;
                        ::SetScrollInfo(_hwnd, SB_VERT, &si, TRUE);
                        ::ShowScrollBar(_hwnd, SB_VERT, TRUE);
                    } else {
                        ::ShowScrollBar(_hwnd, SB_VERT, FALSE);
                    }
                    if(bSetHorz) {
                        si.nMax = dw - cw - 1 + si.nPage - 1;
                        ::SetScrollInfo(_hwnd, SB_HORZ, &si, TRUE);
                        ::ShowScrollBar(_hwnd, SB_HORZ, TRUE);
                    } else {
                        ::ShowScrollBar(_hwnd, SB_HORZ, FALSE);
                    }
                };

                SIZE sz = _root->set_pos({ 0, 0, cw, ch });

                process_scrollbar(cw,ch, sz.cx, sz.cy);
                
                return 0;
            }
            else if(msg == WM_VSCROLL || msg == WM_HSCROLL){
                if(msg == WM_VSCROLL) {
                    SCROLLINFO si = {0};
                    int iVertPos = 0;
                    si.cbSize = sizeof(si);
                    si.fMask = SIF_ALL;
                    ::GetScrollInfo(_hwnd, SB_VERT, &si);
                    iVertPos = si.nPos;

                    switch(LOWORD(wp)) {
                    case SB_ENDSCROLL:						break;
                    case SB_TOP:		si.nPos = 0;		break;
                    case SB_BOTTOM:		si.nPos = si.nMax;	break;
                    case SB_LINEUP:		si.nPos--;			break;
                    case SB_LINEDOWN:	si.nPos++;			break;
                    case SB_PAGEUP:		si.nPos -= si.nPage; break;
                    case SB_PAGEDOWN:	si.nPos += si.nPage; break;
                    case SB_THUMBTRACK:
                    case SB_THUMBPOSITION:
                        si.nPos = si.nTrackPos; break;
                    }

                    si.fMask = SIF_POS;
                    ::SetScrollInfo(_hwnd, SB_VERT, &si, TRUE);
                    ::GetScrollInfo(_hwnd, SB_VERT, &si);

                    if(si.nPos != iVertPos) {
                        ::ScrollWindow(_hwnd, 0, (iVertPos - si.nPos), NULL, NULL);
                        ::UpdateWindow(_hwnd);
                    }
                } else if(msg == WM_HSCROLL) {
                    SCROLLINFO si = {0};
                    int iHorzPos = 0;
                    si.cbSize = sizeof(si);
                    si.fMask = SIF_ALL;
                    ::GetScrollInfo(_hwnd, SB_HORZ, &si);
                    iHorzPos = si.nPos;

                    switch(LOWORD(wp)) {
                    case SB_ENDSCROLL:						break;
                    case SB_LEFT:		si.nPos = 0;		break;
                    case SB_RIGHT:		si.nPos = si.nMax;	break;
                    case SB_LINELEFT:	si.nPos--;			break;
                    case SB_LINERIGHT:	si.nPos++;			break;
                    case SB_PAGELEFT:	si.nPos -= si.nPage; break;
                    case SB_PAGERIGHT:	si.nPos += si.nPage; break;
                    case SB_THUMBTRACK:
                    case SB_THUMBPOSITION:
                        si.nPos = si.nTrackPos; break;
                    }

                    si.fMask = SIF_POS;
                    ::SetScrollInfo(_hwnd, SB_HORZ, &si, TRUE);
                    ::GetScrollInfo(_hwnd, SB_HORZ, &si);

                    if(si.nPos != iHorzPos) {
                        ::ScrollWindow(_hwnd, (iHorzPos - si.nPos), 0, NULL, NULL);
                        ::UpdateWindow(_hwnd);
                    }
                }

                return 0;
            }

            return __super::wndproc(msg, wp, lp);
        }
    };

    nbsg_window mainwnd;
    mainwnd.create("nbsg", { 100, 100, 900, 700 });
    mainwnd.show();

    nbsg::window::message_loop_t loop;
    loop.loop();

    //db.close();

    return 0;
}
