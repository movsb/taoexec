#include <lua/lua.hpp>

#include "script.h"


namespace taoexec {
namespace script {

scriptable::scriptable() {
    _state = luaL_newstate();
}

scriptable::~scriptable() {
    lua_close(_state);
    _state = nullptr;
}

} // script
} // taoexec
