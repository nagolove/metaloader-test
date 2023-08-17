// vim: set colorcolumn=85
// vim: fdm=marker

#include "libsmallregex.h"
#include "koh_lua_tools.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include "munit.h"
#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MunitResult test_dump2str_fail(
    const MunitParameter params[], void* data
) {
    lua_State *lua = luaL_newstate();
    luaL_openlibs(lua);

    const char *code =  "return { &&&% }\n";

    luaL_loadstring(lua, code);
    /*printf("[%s]\n", stack_dump(lua));*/
    lua_call(lua, 0, LUA_MULTRET);
    /*printf("[%s]\n", stack_dump(lua));*/

    char *dump = table_dump2allocated_str(lua);
    /*char *dump = "XXX";*/
    lua_close(lua);
    if (!dump) {
        return MUNIT_FAIL;
    }

    /*
    {
        lua_State *l = luaL_newstate();
        luaL_openlibs(l);
        luaL_loadstring(l, dump);
        lua_call(l, 0, 1);

        {
            lua_pushstring(l, "f");
            munit_assert(lua_gettable(l, -2) == LUA_TTABLE);

            for (int i = 1; i <= 3; i++) {
                munit_assert(lua_geti(l, -1, i) == LUA_TNUMBER);
                double n = lua_tonumber(l, -1);
                munit_assert(n == i);
                lua_remove(l, -1);
            }

            lua_remove(l, -1);
        }

        {
            lua_pushstring(l, "a");
            munit_assert(lua_gettable(l, -2) == LUA_TTABLE);

            const char *values[] = { "her", "var", "par", NULL };
            char **pvalue = (char**)values;

            lua_pushnil(l);
            while (lua_next(l, -2)) {
                munit_assert(lua_type(l, -1) == LUA_TSTRING);

                if (!pvalue) {
                    free(dump);
                    lua_close(l);
                    return MUNIT_FAIL;
                }

                munit_assert(!strcmp(*pvalue, lua_tostring(l, -1)));
                pvalue++;
                lua_remove(l, -1);
            }
        }

        lua_close(l);
    }
    */

    free(dump);

    return MUNIT_OK;
}

static MunitResult test_dump2str(
    const MunitParameter params[], void* data
) {
    lua_State *lua = luaL_newstate();
    luaL_openlibs(lua);

    const char *code =  "return {\n"
                        "   f = {\n"
                        "       1, 2, 3\n"
                        "   },\n"
                        "   a = {\n"
                        "       'her', 'var', 'par'\n"
                        "   },\n"
                        "}\n";

	luaL_loadstring(lua, code);
    lua_call(lua, 0, LUA_MULTRET);

    char *dump = table_dump2allocated_str(lua);
    lua_close(lua);
    if (!dump) {
        return MUNIT_FAIL;
    }

    {
        lua_State *l = luaL_newstate();
        luaL_openlibs(l);
        luaL_loadstring(l, dump);
        /*printf("[%s]\n", stack_dump(l));*/
        lua_call(l, 0, 1);
        /*printf("[%s]\n", stack_dump(l));*/

        {
            lua_pushstring(l, "f");
            munit_assert(lua_gettable(l, -2) == LUA_TTABLE);

            for (int i = 1; i <= 3; i++) {
                munit_assert(lua_geti(l, -1, i) == LUA_TNUMBER);
                double n = lua_tonumber(l, -1);
                munit_assert(n == i);
                lua_remove(l, -1);
            }

            lua_remove(l, -1);
        }

        {
            lua_pushstring(l, "a");
            munit_assert(lua_gettable(l, -2) == LUA_TTABLE);

            const char *values[] = { "her", "var", "par", NULL };
            char **pvalue = (char**)values;

            lua_pushnil(l);
            while (lua_next(l, -2)) {
                /*printf("[%s]\n", stack_dump(l));*/
                munit_assert(lua_type(l, -1) == LUA_TSTRING);

                if (!pvalue) {
                    free(dump);
                    lua_close(l);
                    return MUNIT_FAIL;
                }

                munit_assert(!strcmp(*pvalue, lua_tostring(l, -1)));
                pvalue++;
                lua_remove(l, -1);
            }
        }

        lua_close(l);
    }

    free(dump);

    return MUNIT_OK;
}

// Дублирование элементов на стеке
static MunitResult test_stack_dup(
    const MunitParameter params[], void* data
) {
    lua_State *lua;

    lua = luaL_newstate();
    printf("[%s]\n", stack_dump(lua));

    lua_createtable(lua, 0, 0);
    munit_assert_int(lua_gettop(lua), ==, 1);
    munit_assert(lua_type(lua, 1) == LUA_TTABLE);
    lua_pushvalue(lua, -1);
    munit_assert_int(lua_gettop(lua), ==, 2);
    munit_assert(lua_type(lua, 1) == LUA_TTABLE);
    munit_assert(lua_type(lua, 2) == LUA_TTABLE);

    lua_pushinteger(lua, 11);
    munit_assert(lua_type(lua, -1) == LUA_TNUMBER);
    lua_pushvalue(lua, -1);
    munit_assert(lua_type(lua, -1) == LUA_TNUMBER);
    munit_assert(lua_type(lua, -2) == LUA_TNUMBER);

    printf("[%s]\n", stack_dump(lua));

    lua_close(lua);

    return MUNIT_OK;
}

static MunitResult test_table_printing(
    const MunitParameter params[], void* data
) {
    lua_State *lua;

    lua = luaL_newstate();

    printf("\n");

    luaL_openlibs(lua);
    lua_createtable(lua, 0, 0);

    // Для кода luaL_dostring()
    lua_pushvalue(lua, -1);
    lua_setglobal(lua, "XX");

    float v1 = 1.5, v2 = 2;
    const char *key1 = "x", *key2 = "y";

    lua_pushstring(lua, key1);
    lua_pushnumber(lua, v1);
    lua_rawset(lua, 1);

    lua_pushstring(lua, key2);
    lua_pushnumber(lua, v2);
    lua_rawset(lua, 1);

    const char *tbl_str = table_get_print(lua, 1, &(struct TablePrintOpts) {
        .tabulate = true,
    });

    float printed_v1 = 0, printed_v2 = 0;
    char printed_key1[32] = {}, printed_key2[32] = {};

    munit_assert(regex_match("y = 2", tbl_str) > 0);
    munit_assert(regex_match("x = 1.5", tbl_str) > 0);

    //munit_assert(regex_match("    y = 2", tbl_str) > 0);
    //munit_assert(regex_match("   x = 1.5", tbl_str) > 0);
    // XXX: Почему падает на строке с четырьмя пробелами?
    //munit_assert(regex_match("   x = 1.5", tbl_str) > 0);

    printf("printed_key1 %s, printed_v1 %lf\n", printed_key1, printed_v1);
    printf("printed_key2 %s, printed_v2 %lf\n", printed_key2, printed_v2);

    printf("{\n%s}\n", tbl_str); 
    print_table(lua, 1);

    int errcode = luaL_dostring(lua,    "for k, v in pairs(XX) do"
                                        "   print(k, v)"
                                        "end");
    (void)(errcode);

    lua_close(lua);

    return MUNIT_OK;
}


static MunitTest test_suite_tests[] = {
  {
    (char*) "/table_printing",
    test_table_printing,
    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
  },
  {
    (char*) "/dump2str_fail",
    test_dump2str_fail,
    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
  },
  {
    (char*) "/dump2str",
    test_dump2str,
    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
  },
  {
    (char*) "/stack_dup",
    test_stack_dup,
    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
  },
  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
  (char*) "lua_tools", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char **argv) {
    return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);
}
