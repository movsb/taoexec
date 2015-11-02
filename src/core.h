#pragma once

#include <regex>
#include <cctype>
#include <map>
#include <vector>
#include <functional>
#include <string>
#include <sstream>

#include "charset.h"

#include <windows.h>
#include <shellapi.h>
#include <ShlObj.h>
#include <assert.h>

namespace nbsg {
    namespace core {

        class env_var_t {
        public:

            env_var_t() {

            }

            void set(const std::string& envstr) {
                _vars.clear();
                patch(envstr);
            }

            void patch(const std::string& envstr) {
                const char* p = envstr.c_str();
                for(; *p;) {
                    auto key_begin = p;
                    while(*p != '=') p++;
                    auto key_end = p++;
                    std::string key(key_begin, key_end);

                    std::string val;
                    while(*p && *p !='\r' && *p != '\n') {
                        if(*p == '%') {
                            auto var_start = ++p;
                            while(*p != '%') p++; // buggy
                            auto var_end = p++;
                            std::string var(var_start, var_end);
                            if(_vars.count(var))
                                val += _vars[var];
                        } else {
                            val += *p++;
                        }
                    }

                    if(key.size() == 0)
                        _nameless.push_back(std::move(val));
                    else
                        _vars[std::move(key)] = std::move(val);

                    while(*p == '\r' || *p == '\n') 
                        p++;
                    if(!*p) p++;
                }
            }

            std::string serialize() const {
                std::ostringstream oss;
                for(auto& v : _nameless) {
                    oss << '=' << v;
                    oss.write("", 1);
                }
                for(auto it = _vars.cbegin(), end = _vars.cend(); it != end; it++) {
                    oss << it->first << '=' << it->second;
                    oss.write("", 1);
                }
                oss.write("", 1);
                return std::move(oss.str());
            }

        protected:
            std::vector<std::string>            _nameless;
            std::map<std::string, std::string>  _vars;
        };

        static std::map<std::string, std::string> g_variables;
        typedef std::vector<std::string> func_args;
        static std::map<std::string, std::function<std::string(func_args& args)>> g_functions;

        static bool is_64bit() {
            BOOL b64;
            return IsWow64Process(GetCurrentProcess(), &b64)
                && b64 != FALSE;
        }

        static void add_user_variable(const std::string& var, const std::string& val) {
            g_variables[var] = val;
        }

        static void initialize_globals() {
            auto initialize_variables = []() {
                char path[MAX_PATH];

                // the exe directory
                if(GetModuleFileName(NULL, path, _countof(path)) > 0) {
                    *strrchr(path, '\\') = '\0';
                    g_variables["exe_dir"] = path;
                }

                // the Windows directory
                path[GetWindowsDirectory(path, _countof(path))] = '\0';
                if(path[3] == '\0') path[2] = '\0'; // on root drive, removes backslash
                g_variables["windows"] = path;

                // the Current Directory
                path[GetCurrentDirectory(_countof(path), path)] = '\0';
                if(path[3] == '\0') path[2] = '\0'; // on root drive, removes backslash
                g_variables["cd"] = path;

                // the System directory
                path[GetSystemDirectory(path, _countof(path))] = '\0';
                if(path[3] == '\0') path[2] = '\0'; // on root drive, removes backslash
                g_variables["system"] = path;

                // the AppData
                if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
                    g_variables["appdata"] = path;

                // the home
                if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, path)))
                    g_variables["home"] = path;

                // the desktop
                if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, path)))
                    g_variables["desktop"] = path;

                // the Program Files (x86)
                if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, path)))
                    g_variables["program_x86"] = path;

                // the Program Files
                if(GetEnvironmentVariable("ProgramW6432", path, _countof(path)) > 0)
                    g_variables["program"] = path;
            };

            auto initialize_functions = []() {
                g_functions["reg"] = [](func_args& args)->std::string {
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
                            if(is_64bit()) {
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

                g_functions["env"] = [](func_args& args)->std::string {
                    std::string result;
                    if(args.size() >= 1) {
                        char buf[2048];
                        if(GetEnvironmentVariable(args[0].c_str(), buf, _countof(buf)) > 0) {
                            result = buf;
                        }
                    }

                    return result;
                };

                g_functions["app_path"] = [](func_args& args)->std::string {
                    std::string result;
                    auto reg = g_functions["reg"];

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

        static std::string expand_variable(const std::string& var) {
            if(g_variables.count(var) > 0)
                return g_variables[var];

            return "";
        }

        static std::string expand_function(const std::string& fn, func_args& args) {
            if(g_functions.count(fn))
                return g_functions[fn](args);

            return "";
        }

        static std::string expand(const std::string& raw) {
            std::string result;
            const char* p = raw.c_str();

            auto read_arg = [](const char*& p) {
                std::string s;
                while(*p && *p != ',' && *p != ')') {
                    s += *p;
                    p++;
                }

                return s;
            };

            auto read_ident = [](const char*& p) {
                std::string s;
                while(::isalnum(*p) || strchr("_", *p)) {
                    s += *p;
                    p++;
                }

                return s;
            };

            while(*p != '\0') {
                if(*p == '$') {
                    p++;
                    if(*p == '{') {
                        p++;
                        auto var = read_ident(p);
                        if(*p++ != '}') {
                            // assert(0);
                        }

                        result += expand_variable(var);
                    } else if(::isalpha(*p)) {
                        auto fn = read_ident(p);
                        if(*p++ != '(') {
                            // assert(0);
                        }

                        func_args args;
                        while((true)) {
                            std::string arg(read_arg(p));
                            args.push_back(arg);

                            if(*p == ',') {
                                p++;
                                continue;
                            } else if(*p == ')') {
                                break;
                            }
                        }

                        if(*p++ != ')') {
                            // assert(0);
                        }

                        result += expand_function(fn, args);
                    } else {
                        // assert(0);
                        p++;
                    }
                } else {
                    result += *p;
                    p++;
                }
            }
            return result;
        }

        static bool parse_args(const std::string& argstr, std::string* const cmd, std::string* const arg) {
            auto p = argstr.c_str();
            char c;
            for(;;) {
                c = *p;
                if(c == ' ' || c == '\t') {
                    p++;
                    continue;
                }
                else if(::isalnum(c)) {
                    auto& refcmd = *cmd;
                    refcmd += c;
                    p++;
                    while(::isalnum(*p)) {
                        refcmd += *p;
                        p++;
                    }

                    for(;;) {
                        c = *p;
                        if(c == ' ' || c == '\t') {
                            p++;
                            continue;
                        }
                        else if(c == '\0') {
                            break;
                        }
                        else {
                            auto begin = p;
                            while(*p)
                                p++;
                            --p;
                            while(*p == ' ' || *p == '\t')
                                --p;
                            arg->assign(begin, ++p);
                            break;
                        }
                    }
                }
                break;
            }
            return true;
        }

        static std::string expand_args(const std::string& params, const std::string& args) {
            std::string result;
            result.reserve(params.size() + args.size() + 1);

            bool arg0_used = false;
            auto p = params.c_str();
            while(*p) {
                if(*p == '$') {
                    p++;
                    if(*p == '{') {
                        auto begin = p++;
                        int argn = 0;
                        while(::isdigit(*p)) {
                            argn *= 10;
                            argn += *p - '0';
                            p++;
                        }

                        if(begin == p) { // not like ${0}, ${123}

                        }

                        if(argn == 0) {
                            result += args;
                            arg0_used = true;
                        }
                        else {  // currently, error

                        }

                        if(*p != '}') { // not enclosed

                        }

                        p++;    // skip `}`
                    }
                    else if(*p == '$') {
                        result += '$';
                        p++;
                    }
                    else { // unexpected

                    }
                }
                else if(*p) {
                    result += *p;
                    p++;
                }
                else {
                    break;
                }
            }

            if(!arg0_used)
                result += ' ' + args;

            return result;
        }

        static void execute(HWND hwnd, const std::string& path,
            const std::string& params, const std::string& args,
            const std::string& wd_, const std::string& env_,
            std::function<void(const std::string& err)> cb)
        {
            // not specified as absolute path or as relative path to a file.
            if(std::regex_match(path, std::regex(R"(shell:::\{.{36}\})"))   // shell clsid
                || std::regex_match(path, std::regex(R"(shell:[^:/]+)"))    // shell command
                || std::regex_match(path, std::regex(R"(https?://.*)"))     // http(s) protocol
                ) {
                ::ShellExecute(hwnd, "open", path.c_str(), nullptr, nullptr, SW_SHOW);
                if(cb) cb("ok");
                return;
            }

            std::string path_expanded = expand(path);

            // cmdline
            std::string cmdline([&]() {
                std::string s = '"' + path_expanded + '"';
                std::string param_args = expand_args(params, args);
                if(param_args.size())
                    s += " " + param_args;
                return std::move(s);
            }());

            // working directory
            // absolute:    c:\windows\notepad.exe  -> c:\windows
            // relative:    ./notepad.exe           -> current directory + ./
            // unspecified: notepad                 -> ${desktop}
            std::string wd([&]() {
                std::string wd2 = std::move(wd_);
                if(wd2.size() && wd2.back() != '\\' &&  wd2.back() != '/')
                    wd2 += '\\';
                std::string ts = wd2.size() ? wd2 : path_expanded;
                bool is_abs = ts.size() > 3 /* C:\ */
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
                    result = expand("${desktop}\\");

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

                std::string def_env([]() {
                    auto def_env = ::GetEnvironmentStrings();
                    auto p = def_env;
                    while(*p) while(*p++);
                    std::string def_env_str(def_env, p + 1);
                    ::FreeEnvironmentStrings(def_env);
                    return std::move(def_env_str);
                }());

                env_var_t env_var;
                env_var.set(def_env);
                env_var.patch(env2.append(2, '\0'));

                return std::move(env_var.serialize());
            }());

            ::STARTUPINFO si = {sizeof(si)};
            ::PROCESS_INFORMATION pi;

            if(::CreateProcess(nullptr, (char*)cmdline.c_str(),
                nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE,
                (void*)(env.size() ? env.c_str() : nullptr), wd.c_str(),
                &si, &pi)) {
                ::CloseHandle(pi.hThread);
                ::CloseHandle(pi.hProcess);
                if(cb) cb("ok");
            } else {
                if(cb) cb("fail");
            }
        }

        static void init() {
            initialize_globals();
        }

        static void uninit() {

        }
    }
}
