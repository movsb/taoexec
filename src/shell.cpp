#include <ShObjIdl.h>
#include <ShlGuid.h>
#include <windows.h>

#include <vector>
#include <string>
#include <sstream>
#include <memory>

#include "shell.h"
#include "charset.h"

namespace taoexec {
namespace shell {

// ----- env_var_t -----
std::string env_var_t::serialize() const {
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

void env_var_t::patch_current() {
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

void env_var_t::patch(const std::string& envstr) {
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

void env_var_t::set(const std::string& envstr) {
    _vars.clear();
    patch(envstr);
}
// ----- env_var_t -----

bool parse_link_file(const std::string& path, link_info* info) {
    bool r = false;
    IShellLink* plnk;

    // 不管是否已经初始化过
    CoInitialize(nullptr);

    if(SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&plnk))) {
        IPersistFile* ppf;
        if(SUCCEEDED(plnk->QueryInterface<IPersistFile>(&ppf))) {
            if(SUCCEEDED(ppf->Load(charset::a2u(path).c_str(), STGM_READ))) {
                if(SUCCEEDED(plnk->Resolve(GetDesktopWindow(), 0))) {
                    char out[4096]; // 足够了吧？
                    if(SUCCEEDED(plnk->GetPath(&out[0], _countof(out), nullptr, SLGP_RAWPATH)))
                        info->path = out;

                    if(SUCCEEDED(plnk->GetArguments(&out[0], _countof(out))))
                        info->args = out;

                    if(SUCCEEDED(plnk->GetDescription(&out[0], _countof(out))))
                        info->desc = out;

                    if(SUCCEEDED(plnk->GetWorkingDirectory(&out[0], _countof(out))))
                        info->wd = out;

                    r = true;
                }
            }
            ppf->Release();
        }
        plnk->Release();
    }

    return r;
}

std::string ext(const std::string& file) {
    // 文件如果有扩展名，其小数点的两边必须有其它字符才算
    // file.exe 算，file.ext. 不算，.git 不算

    auto fs = file.find_last_of('/');
    auto bs = file.find_last_of('\\');
    auto d = file.find_last_of('.');

    auto sep = file.npos;
    if(fs != file.npos)
        sep = fs;
    if(bs != file.npos && ((sep != file.npos && bs > sep) || sep == file.npos))
        sep = bs;

    if(d != std::string::npos                       // 有小数点
        && d != file.size() - 1                     // 不是最后一个字符
        && (sep == file.npos && d != 0              // 如果没有分隔符，则不能是第1个
            || sep != file.npos && d > sep + 1)      // 如果存在分隔符，那么一定不是分隔符后面的第1个字符
        ) 
    {
        return file.c_str() + d;
    }
    return "";
}

file_type type(const char* file) {
    DWORD dw_attr = ::GetFileAttributes(file);
    if(dw_attr == INVALID_FILE_ATTRIBUTES)
        return file_type::error;

    if(dw_attr & FILE_ATTRIBUTE_DIRECTORY)
        return file_type::directory;

    return file_type::file;
}

// https://msdn.microsoft.com/en-us/library/windows/desktop/bb776891%28v=vs.85%29.aspx
/*
HKEY_CLASSES_ROOT
    .xyz
        (Default) = XYZApp
    XYZApp
        IsShortcut
*/
bool is_ext_link(const std::string& ext) {
    if(ext.size()) {
        std::string def = query_registry(HKEY_CLASSES_ROOT, ext, "");
        if(def.size()) {
            bool has;
            query_registry(HKEY_CLASSES_ROOT, def, "IsShortcut", &has);
            return has;
        }
    }

    return false;
}

std::string query_registry(HKEY root, const std::string& subkey, const std::string& name, bool* has_name, bool wow6432) {
    std::string result;
    char value[2048];
    DWORD cb = sizeof(value);

    if(has_name)
        *has_name = false;

    REGSAM sam = KEY_READ;
    if(is_wow64() && wow6432)
        sam |= KEY_WOW64_32KEY;

    HKEY hkey;
    if(RegOpenKeyEx(root, NULL, 0, sam, &hkey) == ERROR_SUCCESS) {
        if(RegGetValue(hkey, subkey.c_str(), name.c_str(),
            RRF_RT_REG_SZ, nullptr, (void*)value, &cb) == ERROR_SUCCESS) {
            result = value;
            if(has_name)
                *has_name = true;
        }
        RegCloseKey(hkey);
    }

    return result;
}

bool parse_hotkey_string(const std::string& hotstr, unsigned int* mods, unsigned int* vk, const char** err) {
    auto is_blank = [](const char c) {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    };

    auto is_ident_char = [](const char c) {
        return c >= '0' && c <= '9'
            || c >= 'a' && c <= 'z'
            || c >= 'A' && c <= 'Z';
    };

    auto map_modkey = [](const std::string& s) {
        if (_stricmp(s.c_str(), "ctrl") == 0)
            return MOD_CONTROL;
        else if (_stricmp(s.c_str(), "shift") == 0)
            return MOD_SHIFT;
        else if (_stricmp(s.c_str(), "alt") == 0)
            return MOD_ALT;
        else
            return 0;
    };

    auto map_vcode = [](const std::string& s) {
        auto p = s.c_str();

        // 0-9
        if (p[0] >= '0' && p[0] <= '9') {
            if (!p[1]) {
                return (unsigned int)p[0];
            }
        }
        // A-Z
        else if ((p[0] >= 'A' && p[0] <= 'Z' || p[0] >= 'a' && p[0] <= 'z') && !p[1]) {
            return (unsigned int)p[0] & ~0x20;
        }
        // F1-F12
        else if (p[0] == 'f' || p[0] == 'F') {
            int i;
            if (sscanf(&p[1], "%d", &i) == 1) {
                if (i >= 1 && i <= 12) {
                    return (unsigned int)VK_F1 + i - 1;
                }
            }
        }

        return (unsigned int)0;
    };

    *mods = 0;
    *vk = 0;

    for (auto p = hotstr.c_str();;) {
        if (is_ident_char(*p)) {
            std::string id;
            while (is_ident_char(*p)) {
                id += *p;
                ++p;
            }

            while (is_blank(*p))
                ++p;

            if (*p) { // modifier
                unsigned int mod = map_modkey(id);
                if (mod == 0 || *mods & mod) {
                    if(err) *err = mod == 0 ? "无效辅助键" : "辅助键重复";
                    return false;
                }
                else {
                    *mods |= mod;
                }

                if (*p == '+') {
                    ++p;
                }
                else {
                    if(err) *err = "辅助键后存在无效字符，应该为“+”";
                    return false;
                }
            }
            else { // keycode
                unsigned int keycode = map_vcode(id);
                if (!keycode) {
                    if (err) *err = "无效键码";
                    return false;
                }
                else {
                    *vk = keycode;
                }
                break;
            }
        }
        else if (is_blank(*p)) {
            ++p;
            continue;
        }
        else {
            if(err) *err = "无效序列（空白串）。";
            return false;
        }
    }

    return true;
}

int get_directory_files(const char* root, const char* extname, std::vector<std::string>* matches) {
    std::string folder = root;
    if(folder.back() != '/' && folder.back() != '\\')
        folder.append(1, '\\');

    WIN32_FIND_DATA wfd;
    std::string pattern = folder + '*' + extname;
    if(is_wow64())
        ::Wow64DisableWow64FsRedirection(nullptr);
    HANDLE hfind = ::FindFirstFile(pattern.c_str(), &wfd);
    if(hfind != INVALID_HANDLE_VALUE) {
        do {
            std::string file = folder + wfd.cFileName;
            matches->push_back(std::move(file));
        } while(::FindNextFile(hfind, &wfd));
        ::FindClose(hfind);
    }
    if(taoexec::shell::is_wow64())
        ::Wow64EnableWow64FsRedirection(TRUE);

    return (int)matches->size();
}

std::string exe_dir() {
    char path[2048];
    path[GetModuleFileName(nullptr, path, _countof(path))] = '\0';
    *strrchr(path, '\\') = '\0';
    return path;
}


// ----- which -----

which::which()
{
    _init();
}

std::string which::call(const std::string& cmd, bool usecache) {
    std::string result;

    if (usecache) {
        auto it = _cache.find(cmd);
        if (it != _cache.cend() && !it->second.empty()) {
            return it->second;
        }
    }

    // 先特殊处理，然后按照操作系统搜索顺序搜索，找不到才会到App Paths中去找
    if (result.empty())
        result = _from_special(cmd, usecache);
    if (result.empty())
        result = _from_search(cmd, usecache);
    if(result.empty())
        result = _from_apppaths(cmd, usecache);

    _cache[cmd] = result;

    return result;
}

std::string which::_from_search(const std::string& cmd, bool usecache) {
    std::string result;
    std::vector<std::string> matches;
    bool has_match = false;
    for(auto& dir : _dirs) {
        if(dir.empty())
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
        if(taoexec::shell::is_wow64()) ::Wow64EnableWow64FsRedirection(TRUE); // BUG: remember to disable
    }

_exit_for:
    if(has_match) {
        return matches.back();
    }

    return "";
}

void which::add_dir(const std::string& dir) {
    _dirs.push_back(dir);
}

void which::_init() {
    auto init_sys_path = [&]() {
        char path[MAX_PATH];

        // the exe directory
        if (GetModuleFileName(NULL, path, _countof(path)) > 0) {
            *strrchr(path, '\\') = '\0';
            add_dir(path);
        }

        // the Current Directory
        path[GetCurrentDirectory(_countof(path), path)] = '\0';
        if (path[3] == '\0') path[2] = '\0'; // on root drive, removes backslash
        add_dir(path);

        // the System directory
        path[GetSystemDirectory(path, _countof(path))] = '\0';
        add_dir(path);

        // the Windows directory
        path[GetWindowsDirectory(path, _countof(path))] = '\0';
        add_dir(path);

        // The directories that are listed in the PATH environment variable
        const int var_size = 32 * 1024;
        std::unique_ptr<char[]> envstr(new char[var_size]);
        if(::GetEnvironmentVariable("PATH", envstr.get(), var_size)) {
            auto spath = envstr.get();
            auto end = spath;
            while(*end) {
                auto begin = end;
                while(*end && *end != ';')
                    end++;
                if(end > begin)
                    add_dir({ begin, end });
                if(*end == ';')
                    end++;
            }
        }
    };

    init_sys_path();
}

/*
HKCU\Software\Microsoft\Windows\CurrentVersion\App Paths\
HKCU\Software\Wow6432Node\Microsoft\Windows\CurrentVersion\App Paths\
HKLM\Software\Microsoft\Windows\CurrentVersion\App Paths\
HKLM\Software\Wow6432Node\Microsoft\Windows\CurrentVersion\App Paths\
*/
std::string which::_from_apppaths(const std::string& cmd, bool usecache) {
    std::string result;

    std::string key1(R"(Software\Microsoft\Windows\CurrentVersion\App Paths\)" + cmd + ".exe");
    std::string key2(R"(Software\Wow6432Node\Microsoft\Windows\CurrentVersion\App Paths\)" + cmd + ".exe");

    if(result.empty())
        result = query_registry(HKEY_CURRENT_USER, key1, "", nullptr, false);
    if(result.empty() && is_wow64())
        result = query_registry(HKEY_CURRENT_USER, key2, "", nullptr, true);
    if(result.empty())
        result = query_registry(HKEY_LOCAL_MACHINE, key1, "", nullptr, false);
    if(result.empty() && is_wow64())
        result = query_registry(HKEY_LOCAL_MACHINE, key2, "", nullptr, true);

    return std::move(result);
}

std::string which::_from_special(const std::string& cmd, bool usecache /*= true*/) {
    std::string result;

    if (cmd.size() > 2 && cmd[0] == '\\' && cmd[1] == '\\')
        result = cmd;

    return result;
}

// ----- which -----

}
}
