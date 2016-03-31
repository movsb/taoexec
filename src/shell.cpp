#include <ShObjIdl.h>
#include <ShlGuid.h>
#include <windows.h>

#include <vector>
#include <string>

#include "shell.h"
#include "charset.h"

namespace taoexec {
namespace shell {

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

std::string query_registry(HKEY root, const std::string& subkey, const std::string& name, bool* has_name) {
    std::string result;
    char value[2048];
    DWORD cb = sizeof(value);

    if(has_name)
        *has_name = false;

    REGSAM sam = KEY_READ;
    if(is_wow64())
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

}
}
