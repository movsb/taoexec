#include "exec.h"
#include "shell.h"
#include "utils.h"
#include "event.h"

#include <Shlwapi.h>

namespace taoexec {
namespace exec {
/*
// ----- registry_executor -----
registry_executor::registry_executor() {

}

bool registry_executor::execute(const std::string& all, const std::string& scheme, const std::string& args) {
    // cache ?
    if(!_commands.count(scheme)) {
        bool has_name;
        taoexec::shell::query_registry(HKEY_CLASSES_ROOT, scheme, "URL Protocol", &has_name);
        if(has_name) {
            std::string subkey = scheme + R"(\shell\open\command)";
            _commands[scheme] = taoexec::shell::query_registry(HKEY_CLASSES_ROOT, subkey, "");
        }
    }

    auto it = _commands.find(scheme);
    if(it == _commands.end()) {
        //_pMini->msgbox("尚未向注册表注册的协议。");
        return false;
    }

    auto cmd = it->second;
    if(!cmd.size()) {
        //_pMini->msgbox("未正确注册的协议。");
        return false;
    }

    _execute_command(cmd, all);

    return true;
}

bool registry_executor::_execute_command(const std::string& cmd, const std::string& all) {
    std::string str;
    // 实参替换，目前只支持：%0, %1, %L, %l
    auto p = cmd.c_str();
    for(; *p;) {
        if(*p == '%') {
            switch(*++p) {
            case '0':
            case '1':
            case 'l':
            case 'L':
                str += all;
                ++p;
                break;
            default:
                str += '%';
                ++p;
                break;
            }
        } else {
            auto bp = p;
            while(*p && *p != '%')
                ++p;
            str.append(bp, p);
        }
    }

    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi;

    if(::CreateProcess(nullptr, (LPSTR)str.c_str(), nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi)) {
        ::CloseHandle(pi.hThread);
        ::CloseHandle(pi.hProcess);

        return true;
    } else {
        return false;
    }
}

// ----- registry_executor -----
*/

// ----- executor_main -----
executor_main::executor_main()
{
    _cmds[""] = [&]() {
        _evtmgr->trigger("mini:hide");
    };

    _cmds["main"] = [&]() {
        _evtmgr->trigger("main:new");
    };

    _cmds["exit"] = [&]() {
        _evtmgr->trigger("exit");
    };

    _cmds["settings"] = [&]() {
        _evtmgr->trigger("settings:new");
    };
}

bool executor_main::execute(const std::string& args) {
    auto found = _cmds.find(args);
    if(found != _cmds.end()) {
        found->second();
        return true;
    } else {
        _evtmgr->trigger("msgbox", new eventx::event_msgbox_args("无此内建命令。"));
        return false;
    }
}

// ----- executor_main -----

// ----- executor_indexer -----
bool executor_indexer::execute(const std::string& args) {
    std::string index, params;

    auto p = args.c_str();
    auto bp = p;

    while(*p && ::isalnum(*p))
        ++p;

    if(*p == '\0') { // 没有参数
        index = bp;
    } else {
        index = std::string(bp, p - bp);

        while(*p == ' ' || *p == '\t')
            ++p;
        bp = p;
        for(p = args.c_str() + args.size() - 1; *p == ' ' || *p == '\t';)
            --p;
        params = std::string(bp, p + 1);
    }

    auto from_db = [&]()->taoexec::model::item_t* {
        taoexec::model::item_t* found = nullptr;
        std::vector<taoexec::model::item_t*> items;
        int rc = _itemdb->query(index, &items);
        if(rc == -1) {
            _evtmgr->msgbox("sqlite3 error.");
        } else if(rc == 0) {
            _evtmgr->msgbox("Your search `" + index + "` does not match anything.");
        } else if(rc == 1) {
            found = items[0];
        } else {
            decltype(items.cbegin()) it;
            for(it = items.cbegin(); it != items.cend(); it++) {
                if((*it)->index == index) {
                    found = *it;
                    break;
                }
            }

            for(auto pi : items) {
                if(pi != found)
                    delete pi;
            }

            if(found == nullptr) {
                _evtmgr->msgbox("There are many rows that match your given prefix.");
            }
            // found
        }

        return found;
    };

    auto item = from_db();

    if(!item) {
        return false;
    }

    bool r = false;

    std::vector<std::string> path_arr;
    utils::split_paths(item->paths, &path_arr);
    for(auto& path : path_arr) {
        struct event_parsescheme_args : eventx::event_args_i {
            std::string raw;
            std::string scheme;
            std::string args;

            event_parsescheme_args() {
                flag &= ~eventx::flags::auto_delete;
            }
        };

        event_parsescheme_args schemeargs;
        schemeargs.raw = path;
        if(_evtmgr->trigger("exec:parse-scheme", &schemeargs)) {
            if(schemeargs.scheme == "__none__")
                schemeargs.scheme = "fs";
            else if(schemeargs.scheme == "file")
                schemeargs.scheme = "fs";

            if(schemeargs.scheme == "fs") {
                struct event_execfile_args : eventx::event_args_i {
                    std::string path;
                    std::string args;
                    std::string wd;
                    std::string env;
                };

                auto thefile = new event_execfile_args;
                thefile->path = schemeargs.args;
                thefile->args = item->params;
                thefile->wd = item->work_dir;
                thefile->env = item->env;

                if(_evtmgr->trigger("exec:file", thefile)) {
                    r = true;
                    break;
                }
            }
            else {
                if(_evtmgr->trigger("exec:any", &schemeargs)) {
                    r = true;
                    break;
                }
            }
        }
    }

    delete item;

    return r;
}

// ----- executor_indexer -----

// ----- executor_qq -----
executor_qq::executor_qq(taoexec::model::config_db_t* cfg) 
: _cfg(cfg)
, _uin("191035066")
{
    _path = _cfg->get("qq_path");

    auto userstr = _cfg->get("qq_users");
    std::istringstream iss(userstr);

    for(std::string line; std::getline(iss, line, '\n');) {
        std::regex user_n_hash(R"(([0-9a-zA-Z]+),([0-9A-F]{80})\r*)");
        std::smatch tokens;

        if(std::regex_match(line, tokens, user_n_hash)) {
            std::string nickname = tokens[1].str();
            std::string hash = tokens[2].str();

            _users[nickname] = hash;
        }
    }
}

bool executor_qq::execute(const std::string& args) {
    auto it = _users.find(args);
    if(it != _users.end()) {
        std::string cmd = std::string("/uin:") + _uin + " /quicklunch:" + it->second;
        ::ShellExecute(::GetActiveWindow(), "open", _path.c_str(), cmd.c_str(), nullptr, SW_SHOWNORMAL);
        return true;
    }
    else {
        _evtmgr->trigger("msgbox", new eventx::event_msgbox_args(std::string("找不到此QQ用户：") + args));
        return false;
    }
}

// ----- executor_qq -----

// ----- executor_fs -----

executor_fs::executor_fs() {
    _initialize_globals();
    _initialize_event_listners();
}

std::string executor_fs::env_var_t::serialize() const {
    std::ostringstream oss;
    for (auto& v : _nameless) {
        oss << '=' << v;
        oss.write("", 1);
    }
    for (auto it = _vars.cbegin(), end = _vars.cend(); it != end; ++it) {
        oss << it->first << '=' << it->second;
        oss.write("", 1);
    }
    oss.write("", 1);
    return std::move(oss.str());
}

void executor_fs::env_var_t::patch_current() {
    std::string def_env([]() {
        auto def_env = ::GetEnvironmentStrings();
        auto p = def_env;
        while (*p) while (*p++);
        std::string def_env_str(def_env, p + 1);
        ::FreeEnvironmentStrings(def_env);
        return std::move(def_env_str);
    }());

    patch(def_env);
}

void executor_fs::env_var_t::patch(const std::string& envstr) {
    const char* p = envstr.c_str();
    for (; *p;) {
        auto key_begin = p;
        while (*p != '=') p++; // buggy
        auto key_end = p++;
        std::string key(key_begin, key_end);

        std::string val;
        while (*p && *p != '\r' && *p != '\n') {
            if (*p == '%') {
                auto var_start = ++p;
                while (*p != '%') p++; // buggy
                auto var_end = p++;
                std::string var(var_start, var_end);
                if (_vars.count(var))
                    val += _vars[var];
            }
            else {
                val += *p++;
            }
        }

        if (key.size() == 0)
            _nameless.push_back(std::move(val));
        else
            _vars[std::move(key)] = std::move(val);

        while (*p == '\r' || *p == '\n')
            p++;
        if (!*p) p++;
    }
}

void executor_fs::env_var_t::set(const std::string& envstr) {
    _vars.clear();
    patch(envstr);
}

bool executor_fs::execute(const std::string& __args) {
    std::string path, args;

    try {
        auto p = __args.c_str();

        // skip prefix white spaces
        while(*p == ' ' || *p == '\t')
            ++p;

        // get path
        if(*p == '\'' || *p == '"') {
            auto bp = p++;
            while(*p && *p != *bp)
                ++p;
            if(!*p)
                throw "路径中的引号未配对。";
            path.assign(bp + 1, p);
            ++p;
        }
        else {
            auto bp = p;
            while(*p && *p != ' ' && *p != '\t')
                ++p;
            if(bp == p) {
                path = "";
            }
            else {
                path.assign(bp, p);
            }
        }

        // span
        if(*p && *p != ' ' && *p != '\t')
            throw "路径与参数之间应该至少有一个空白字符。";

        // skip
        while(*p == ' ' || *p == '\t')
            ++p;

        // got args, get args
        if(*p) {
            auto q = __args.c_str() + (int)__args.size() - 1;
            while(*q == ' ' || *q == '\t')
                --q;
            ++q;
            args.assign(p, q);
        }
    }
    catch(const char* e) {
        _evtmgr->msgbox(e);
        return false;
    }

    return execute(path, args);
}

void executor_fs::_expand_exec(conststring& exec_str, conststring& path, conststring& rest, std::string* __cmdline) {
    // 实参替换，目前只支持：%0, %1, %L, %l, %*
    std::string str;
    auto p = exec_str.c_str();
    for(; *p;) {
        if(*p == '%') {
            switch(*++p) {
            case '0':
            case '1':
            case 'l':
            case 'L':
                str += path;
                ++p;
                break;
            case '*':
                str += rest;
                ++p;
                break;
            default:
                throw "不支持的变量替换。";
            }
        } else {
            auto bp = p;
            while(*p && *p != '%')
                ++p;
            str.append(bp, p);
        }
    }

    *__cmdline = std::move(str);
}

void executor_fs::_expand_path(const std::string& before, std::string* __after) {
    std::string after([&]() {
        auto p = before.c_str();
        auto q = before.c_str() + before.size();
        if(*p == '\'' || *p == '"') {
            ++p;
            --q;
            if(p >= q)
                throw "第1个参数不正确。";
        }

        std::string s;

        for(;;) {
            if(*p == '$') {
                ++p;
                if(*p == '$') {
                    s += '$';
                    ++p;
                } else if(::isalpha(*p)) {
                    std::string fn;
                    while(::isalnum(*p) || (*p && ::strchr("_", *p)))
                        fn += *p++;
                    if(*p == '(') {
                        ++p;

                        func_args fargs;

                        for(;;) {
                            auto bp = p;
                            while(p < q && *p != ',' && *p != ')')
                                ++p;
                            if(p == q)
                                throw "函数调用期待`)`。";
                            else {
                                fargs.push_back({bp, p});
                                if(*p == ')') {
                                    ++p;
                                    break;
                                }
                            }
                        }

                        s += _expand_function(fn, fargs);
                    } else {
                        throw "函数调用需要`(`。";
                    }
                } else if(*p == '{') {
                    ++p;
                    if(p == q || *p == '}')
                        throw "需要变量名。";

                    auto bp = p;

                    if(::isalpha(*p))
                        while(::isalnum(*p) || (*p && ::strchr("_", *p)))
                            ++p;

                    if(p == bp)
                        throw "变量名不规范。";
                    else if(*p != '}')
                        throw "变量展开期待`}`。";

                    s += _expand_variable({bp, p});

                    ++p;
                } else {
                    throw "$后面存在不可识别的字符。";
                }
            } else if(*p == '%') {
                ++p;
                if(p == q || *p == '%')
                    throw "需要变量名。";

                auto bp = p;
                while(::isalnum(*p) || (*p && ::strchr("_", *p)))
                    ++p;

                if(*p != '%')
                    throw "需要以`%`结束。";

                s += [&]() {
                    const int bufsize = 32 * 1024 * 1024;
                    std::unique_ptr<char[]> var(new char[bufsize]);
                    var.get()[0] = '\0';
                    ::GetEnvironmentVariable(std::string(bp, p).c_str(), var.get(), bufsize);
                    return std::move(std::string(var.get()));
                }();

                ++p;
            } else if(p == q)
                break;
            else {
                auto bp = p;
                while(p < q && *p != '$' && *p != '%')
                    ++p;
                s.append(bp, p);
            }
        }

        return std::move(s);
    }());

    if(std::regex_match(after, std::regex(R"([0-9a-zA-z_]+)")))
        after = _which(after, "");

    if(!after.size())
        throw "无法取得目标文件路径。";

    *__after = std::move(after);
}

void executor_fs::_initialize_globals() {
    auto initialize_variables = [this]() {
        char path[MAX_PATH];

        // the exe directory
        if(GetModuleFileName(NULL, path, _countof(path)) > 0) {
            *strrchr(path, '\\') = '\0';
            _variables["exe_dir"] = path;
        }

        // the Windows directory
        path[GetWindowsDirectory(path, _countof(path))] = '\0';
        if(path[3] == '\0') path[2] = '\0'; // on root drive, removes backslash
        _variables["windows"] = path;

        // the Current Directory
        path[GetCurrentDirectory(_countof(path), path)] = '\0';
        if(path[3] == '\0') path[2] = '\0'; // on root drive, removes backslash
        _variables["cd"] = path;

        // the System directory
        path[GetSystemDirectory(path, _countof(path))] = '\0';
        if(path[3] == '\0') path[2] = '\0'; // on root drive, removes backslash
        _variables["system"] = path;

        // the AppData
        if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
            _variables["appdata"] = path;

        // the home
        if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, path)))
            _variables["home"] = path;

        // the desktop
        if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, path)))
            _variables["desktop"] = path;

        // the Program Files (x86)
        if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, path)))
            _variables["program_x86"] = path;

        // the Program Files
        if(GetEnvironmentVariable("ProgramW6432", path, _countof(path)) > 0)
            _variables["program"] = path;
    };

    auto initialize_functions = [this]() {
        _functions["reg"] = [](func_args& args)->std::string {
            std::string result;
            if(args.size() >= 3) {
                static std::map<std::string, HKEY> _hkeys {
                    {"HKEY_CLASS_ROOT", HKEY_CLASSES_ROOT},
                    {"HKCR", HKEY_CLASSES_ROOT},
                    {"HKEY_CURRENT_USER", HKEY_CURRENT_USER},
                    {"HKCU", HKEY_CURRENT_USER},
                    {"HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE},
                    {"HKLM", HKEY_LOCAL_MACHINE},
                };

                if(_hkeys.count(args[0])) {
                    char value[2048];
                    DWORD cb = sizeof(value);

                    REGSAM sam = KEY_READ;
                    if(taoexec::shell::is_wow64()) {
                        if(args.size() >= 4 && args[3] == "64")
                            sam |= KEY_WOW64_64KEY;
                        else
                            sam |= KEY_WOW64_32KEY;
                    }

                    HKEY hkey;
                    if(RegOpenKeyEx(_hkeys[args[0]], NULL, 0, sam, &hkey) == ERROR_SUCCESS) {
                        if(RegGetValue(hkey, args[1].c_str(), args[2].c_str(),
                            RRF_RT_REG_SZ, nullptr, (void*)value, &cb) == ERROR_SUCCESS) {
                            result = value;
                        }
                        RegCloseKey(hkey);
                    }
                }
            }
            return result;
        };

        _functions["env"] = [](func_args& args)->std::string {
            std::string result;
            if(args.size() >= 1) {
                char buf[2048];
                if(GetEnvironmentVariable(args[0].c_str(), buf, _countof(buf)) > 0) {
                    result = buf;
                }
            }

            return result;
        };

        _functions["app_path"] = [this](func_args& args)->std::string {
            std::string result;
            auto reg = _functions["reg"];

            if(args.size() >= 1) {
                func_args as {
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

std::string executor_fs::_expand_variable(const std::string& var) {
    if(_variables.count(var) > 0)
        return _variables[var];

    return "";
}

std::string executor_fs::_expand_function(const std::string& fn, func_args& args) {
    if(_functions.count(fn))
        return _functions[fn](args);

    return "";
}

std::string executor_fs::_which(const std::string& cmd, const std::string& env) {
    std::string result;

    // collect all search directories that match CreateProcess' behavior
    std::vector<std::string> search_dirs;
    // 1. the directory from which the application loaded
    search_dirs.push_back(_expand_variable("exe_dir"));
    // 2. the current directory for the parent process
    search_dirs.push_back(_expand_variable("cd"));
    // 3. The 32-bit Windows system directory
    search_dirs.push_back(_expand_variable("system"));
    // 4. The 16-bit Windows system directory (not implemented)
    // 5. The Windows directory
    search_dirs.push_back(_expand_variable("windows"));
    // 6. The directories that are listed in the PATH environment variable
    const int var_size = 32 * 1024;
    std::unique_ptr<char[]> path(new char[var_size]);
    if(::GetEnvironmentVariable("PATH", path.get(), var_size)) {
        auto spath = path.get();
        auto end = spath;
        while(*end) {
            auto begin = end;
            while(*end && *end != ';')
                end++;
            if(end > begin)
                search_dirs.push_back(std::string(begin, end));
            if(*end == ';')
                end++;
        }
    }

    // now search the specified cmd as ``which`` always does
    std::vector<std::string> matches;
    bool has_match = false;
    for(auto& dir : search_dirs) {
        if(!dir.size())
            continue;

        std::string folder = dir;
        if(folder.back() != '/' && folder.back() != '\\')
            folder.append(1, '\\');

        WIN32_FIND_DATA wfd;
        std::string pattern = folder + cmd + '*';
        if(taoexec::shell::is_wow64()) ::Wow64DisableWow64FsRedirection(nullptr);
        HANDLE hfind = ::FindFirstFile(pattern.c_str(), &wfd);
        if(hfind != INVALID_HANDLE_VALUE) {
            do {
                std::string file = folder + wfd.cFileName;
                matches.push_back(std::move(file));
                // check exactly match
                if(_stricmp(cmd.c_str(), wfd.cFileName) == 0) {
                    has_match = true;
                    goto _exit_for;
                }
                // check if executable
                auto offset_match = wfd.cFileName + cmd.size();
                if(_stricmp(offset_match, ".exe") == 0
                    || _stricmp(offset_match, ".bat") == 0
                    || _stricmp(offset_match, ".cmd") == 0
                    ) {
                    has_match = true;
                    goto _exit_for;
                }
            } while(::FindNextFile(hfind, &wfd));
            ::FindClose(hfind);
        }
        if(taoexec::shell::is_wow64()) ::Wow64EnableWow64FsRedirection(TRUE);
    }

_exit_for:
    if(has_match) {
        return matches.back();
    }

    return "";
}


void executor_fs::_add_user_variables(const env_var_t& env_var) {
    for (auto& kv : env_var.get_vars())
        _variables[kv.first] = kv.second;
}

std::string executor_fs::get_executor(const std::string& ext) {
    std::string cmd;
    std::string lower_ext(utils::tolower(ext));

    if (!lower_ext.size())
        return cmd;

    // hit from cache
    if (_exec_strs.count(lower_ext))
        return _exec_strs[lower_ext];

    std::string what_file = shell::query_registry(HKEY_CLASSES_ROOT, ext, "");
    if (what_file.size()) {
        cmd = shell::query_registry(HKEY_CLASSES_ROOT, what_file + R"(\shell\open\command)", "");
        if (cmd.size()) {

        }
    }

    // make cache
    _exec_strs[lower_ext] = cmd;
    return cmd;
}

void executor_fs::_initialize_event_listners() {
    _evtmgr->attach("exec:file", [&](eventx::event_args_i* ____args) {
        struct event_execfile_args : eventx::event_args_i
        {
            std::string path;
            std::string args;
            std::string wd;
            std::string env;
        };
    
        auto __args = reinterpret_cast<event_execfile_args*>(____args);

        return execute(__args->path, __args->args, __args->wd, __args->env);
    });
}

bool executor_fs::execute(conststring& path, conststring& args, conststring& wd, conststring& env) {
    using string = std::string;

    // the file to be executed
    string path_expanded;
    try {
        _expand_path(path, &path_expanded);
    }
    catch(const char* e) {
        _evtmgr->msgbox(e);
        return false;
    }

    if(!::PathFileExists(path_expanded.c_str())) {
        _evtmgr->msgbox("无法找到目标文件。");
        return false;
    }

    // the working directory
    string wd_expanded;
    if(wd.size()) {
        try {
            _expand_path(wd, &wd_expanded);
        }
        catch(const char* e) {
            _evtmgr->msgbox(e);
            return false;
        }

        if(!::PathFileExists(wd_expanded.c_str())) {
            _evtmgr->msgbox("无法找到工作目录。");
            return false;
        }
    }

    // before executing

    if(::GetFileAttributes(path_expanded.c_str()) & FILE_ATTRIBUTE_DIRECTORY) {
        ::ShellExecute(::GetActiveWindow(), "open", "explorer", ("/select,\"" + path_expanded + "\"").c_str(), nullptr, SW_SHOWNORMAL);
        return true;
    }

    if(shell::is_ext_link(shell::ext(path_expanded))) {
        ::ShellExecute(::GetActiveWindow(), "open", path_expanded.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        return true;
    }

    // cmdline
    string exec_str = get_executor(shell::ext(path_expanded));
    if(!exec_str.size()) {
        _evtmgr->msgbox("此文件没有与之关联的打开方式。");
        return false;
    }

    string cmdline;
    try {
        _expand_exec(exec_str, path_expanded, args, &cmdline);
    }
    catch(const char* e) {
        _evtmgr->msgbox(e);
        return false;
    }

    /*
    // working directory
    // absolute:    c:\windows\notepad.exe  -> c:\windows
    // relative:    ./notepad.exe           -> current directory + ./
    // unspecified: notepad                 -> ${desktop}
    std::string wd([&]() {
        std::string wd2 = std::move(wd_);
        if(wd2.size() && wd2.back() != '\\' &&  wd2.back() != '/')
            wd2 += '\\';
        std::string ts = wd2.size() ? wd2 : path_expanded;
        bool is_abs = ts.size() > 3 // C:\ ~~
            && ts[1] == ':'
            && (ts[2] == '/' || ts[2] == '\\');
        bool is_rel = !is_abs
            && ts.size() > 0
            && ts[0] == '.'; // filenames which start with a period is not processed.

        std::string result;
        if(is_abs)
            result = ts;
        else if(is_rel)
            result = ts;
        else
            result = "";// expand("${desktop}\\");

        auto begin = result.c_str();
        auto p = begin + (int)result.size() - 1;
        if(p < begin) p = begin; // size() maybe 0
        while(p > begin && *p != '/' && *p != '\\')
            --p;
        if(p > begin && (*p == '/' || *p == '\\'))
            return std::string(result.c_str(), p - begin);
        else
            return std::move(ts);
    }());

    // environment variables
    std::string env([&]() {
        std::string env2 = std::move(env_);
        if(env2.size() == 0)
            return std::string();

        env_var_t env_var;
        env_var.patch_current();
        env_var.patch(env2.append(2, '\0'));

        return std::move(env_var.serialize());
    }());
    */

    ::STARTUPINFO si = {sizeof(si)};
    ::PROCESS_INFORMATION pi;

    if(::CreateProcess(nullptr, (char*)cmdline.c_str(),
        nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE,
        (void*)(env.size() ? env.c_str() : nullptr), wd_expanded.size() ? wd_expanded.c_str() : nullptr,
        &si, &pi)) {
        ::CloseHandle(pi.hThread);
        ::CloseHandle(pi.hProcess);
        return true;
    } else {
        return false;
    }
}

// ----- executor_fs -----

// ----- executor_shell -----
bool executor_shell::execute(const std::string& args) {
    // form: ::{00000000-12C9-4305-82F9-43058F20E8D2}
    if(std::regex_match(args, std::regex(R"(::\{.{36}\})"))) {
        ::ShellExecute(nullptr, "open", ("shell:" + args).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        return true;
    } else {
        _evtmgr->trigger("msgbox", new eventx::event_msgbox_args("命名空间不正确。"));
        return false;
    }
}

// ----- executor_shell -----


// ----- executor_manager -----

void executor_manager_t::_init_commanders() {
    add(new executor_fs);
    add(new executor_main);
    add(new executor_shell);
    add(new executor_indexer(_itemdb));
    add(new executor_qq(_cfgdb));
}

void executor_manager_t::_uninit_commanders() {
    for(auto& it : _command_executors) {
        delete it.second;
    }
}

void executor_manager_t::init() {
    _init_commanders();
    _init_event_listners();
}

void executor_manager_t::_init_event_listners() {
    _evtmgr->attach("exec:parse-scheme", [&](eventx::event_args_i* ____args) {
        struct event_parsescheme_args : eventx::event_args_i {
            std::string raw;
            std::string cmd;
            std::string args;

            event_parsescheme_args() {
                flag &= ~eventx::flags::auto_delete;
            }
        };

        auto data = reinterpret_cast<event_parsescheme_args*>(____args);

        std::string commander, args;

        auto& __args = data->raw;
        auto p = __args.c_str();
        char c;

        // 特殊处理的部分
        if (__args.size() >= 3 && ::isalpha(__args[0]) && __args[1] == ':' && (__args[2] == '/' || __args[2] == '\\')) {
            commander = "fs";
            args = __args;
            goto _break;
        }
        if (__args.size() >= 3 && __args[0] == '\\' && __args[1] == '\\') {
            commander = "fs";
            args = __args;
            goto _break;
        }

        for (;;) {
            c = *p;
            if (c == ' ' || c == '\t') {
                ++p;
                continue;
            }
            else if (c == '\0') {
                commander = "__main__";
                goto _break;
            }
            // 到了这里并不知道有没有提供执行者，找到冒号才能确定
            else if (::isalnum(c) || c == ':') {
                auto bp = p; // p's backup
                while ((c = *p) && c != ':' && ::isalnum(c))
                    ++p;

                if (c == ':') {  // 找到了执行者
                    commander = bp == p ? "__main__" : std::string(bp, p - bp);
                    args = ++p;
                    goto _break;
                }
                else { // 其它则全部判断为 __none__，自行决定
                    commander = "__none__";
                    args = std::string(bp);
                    goto _break;
                }
            }
            else {
                goto _exit;
            }
        }
 
    _break:
        data->cmd = commander;
        data->args = args;
        return true;

    _exit:
        data->cmd = "fs";
        data->args = __args;
        return true;
    });

    _evtmgr->attach("exec:cmdstr", [&](eventx::event_args_i* ____args) {
        struct event_cmdstr_args : eventx::event_args_i {
            std::string args;
        };

        auto __args = reinterpret_cast<event_cmdstr_args*>(____args)->args;
        struct event_parsescheme_args : eventx::event_args_i {
            std::string raw;
            std::string cmd;
            std::string args;

            event_parsescheme_args() {
                flag &= ~eventx::flags::auto_delete;
            }
        };

        event_parsescheme_args parsedscheme;
        parsedscheme.raw = __args;

        if(!_evtmgr->trigger("exec:parse-scheme", &parsedscheme)) {
            _evtmgr->msgbox("无法解析的命令行。");
            return false;
        }

        if(parsedscheme.cmd == "__none__")
            parsedscheme.cmd = "__indexer__";

        auto it = _command_executors.find(parsedscheme.cmd);
        if(it == _command_executors.cend()) {
            _evtmgr->msgbox("未找到执行者：" + parsedscheme.cmd);
            return false;
        }
        else {
            return it->second->execute(parsedscheme.args);
        }
    });
}

void executor_manager_t::add(command_executor_i* p) {
    auto name = p->get_name();
    if(!get(name))
        _command_executors[name] = p;
}

command_executor_i* executor_manager_t::get(const std::string& name) {
    auto it = _command_executors.find(name);
    return it != _command_executors.cend()
        ? it->second
        : nullptr;
}

// ----- executor_manager -----

} // namespace exec

} // namespace taoexec

