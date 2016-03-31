#pragma once

#include <lua/lua.hpp>

namespace taoexec {
namespace script {

    class scriptable
    {
    public:
        scriptable();
        ~scriptable();

        lua_State* get_state();

        void init();
        void load_all();
        void add_lib(const char* name, lua_CFunction openfunc);

    private:
        lua_State* _state;
    };

} // script
} // taoexec
