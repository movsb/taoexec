#include <ShObjIdl.h>
#include <ShlGuid.h>

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

}
}
