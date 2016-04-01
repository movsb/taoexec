#include <lua/lua.hpp>

#include "script.h"
#include "event.h"
#include "shell.h"


namespace taoexec {
namespace script {

scriptable::scriptable() {
    _state = luaL_newstate();
}

scriptable::~scriptable() {
    lua_close(_state);
    _state = nullptr;
}

lua_State* scriptable::get_state() {
    return _state;
}

void scriptable::init() {
    luaL_openlibs(_state);
}

void scriptable::add_lib(const char* name, lua_CFunction openfunc) {
    luaL_requiref(_state, name, openfunc, 1);
    lua_pop(_state, 1);
}

void scriptable::load_all() {
    std::vector<std::string> scripts;
    shell::get_directory_files((shell::exe_dir() + "/../").c_str(), ".lua", &scripts);

    for(auto& sf : scripts) {
        if(luaL_dofile(_state, sf.c_str()) != 0) {
            _evtmgr->msgbox("执行脚本文件时有误：" + sf);
        }
    }
}

} // script
} // taoexec
