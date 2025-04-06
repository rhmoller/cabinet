#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int main() {
    lua_State *L = luaL_newstate();  // Create a new Lua state
    luaL_openlibs(L);                // Load standard libraries

    if (luaL_dostring(L, "print('Hello from embedded Lua!')") != LUA_OK) {
        fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1); // Remove error
    }

    lua_close(L);  // Close the Lua state
    return 0;
}
