#pragma once

#include <string>
#include <functional>

#include <windows.h>
#include <shellapi.h>

#include "types.hpp"

namespace taoexec {
namespace shell {

class env_var_t
{
public:
    env_var_t() { }
    void set(const std::string& envstr);
    void patch(const std::string& envstr);
    void patch_current();
    std::string serialize() const;
    const taoexec::strstrimap& get_vars() const {
        return _vars;
    }

protected:
    std::vector<std::string>    _nameless;
    taoexec::strstrimap         _vars;
};

std::string exe_dir();

inline bool is_wow64() {
    BOOL b64;
    return IsWow64Process(GetCurrentProcess(), &b64)
        && b64 != FALSE;
}

std::string query_registry(HKEY root, const std::string& subkey, const std::string& name, bool* has_name = nullptr);

struct link_info {
    std::string path;
    std::string args;
    std::string desc;
    std::string wd;
};

bool parse_link_file(const std::string& path, link_info* info);
bool is_ext_link(const std::string& ext);

class drop_files {
public:
    drop_files(HDROP hdrop)
        : _hdrop(hdrop) 
    {}
    ~drop_files() {
        ::DragFinish(_hdrop);
    }

    void for_each(std::function<void(int i, const std::string& path)> dumper) {
        int count = ::DragQueryFile(_hdrop, 0xffffffff, nullptr, 0);
        if(count > 0) {
            char path[1024];
            for(int i = 0; i < count; ++i) {
                if(::DragQueryFile(_hdrop, (UINT)i, &path[0], _countof(path)) > 0) {
                    dumper(i, path);
                }
            }
        }
    }

protected:
    HDROP _hdrop;
};

std::string ext(const std::string& file);

enum class file_type
{
    error = -1,
    file,
    directory,
};

file_type type(const char* file);

bool parse_hotkey_string(const std::string& hotstr, unsigned int* mods, unsigned int* vk, const char** err = nullptr);

int get_directory_files(const char* root, const char* extname, std::vector<std::string>* matches);

class which {
public:
    which(const std::string& cmd, bool usecache = true);

    operator std::string() {
        return _which();
    }

    void add_dir(const std::string& dir);

private:
    void _init();
    std::string _which();

protected:
    const std::string&          _cmd;
    bool                        _usecache;
    std::vector<std::string>    _dirs;
    taoexec::strstrimap         _cache;
};

}
}
