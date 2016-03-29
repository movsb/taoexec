#pragma once

namespace taoexec {
namespace script {

    typedef struct lua_State lua_State;

    class scriptable
    {
    public:
        scriptable();
        ~scriptable();

    private:
        lua_State* _state;
    };

} // script
} // taoexec
