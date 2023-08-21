// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh.h"
#include "koh_metaloader.h"
#include "koh_set.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include "munit.h"
#include "raylib.h"
#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MunitResult test_new_free(
    const MunitParameter params[], void* data
) {
    MetaLoader *ml = metaloader_new();
    metaloader_free(ml);

    return MUNIT_OK;
}

// Возвращает указатель требующий вызова free()
static Rectangle *load_rects(const char *fname, int *rects_num) {
    assert(fname);
    assert(rects_num);

    int cap = 10;
    Rectangle *rects = calloc(cap, sizeof(rects[0]));
    lua_State *l = luaL_newstate();
    luaL_openlibs(l);
    munit_assert_true(luaL_dofile(l, fname) == LUA_OK);
    lua_call(l, 0, 1);
    munit_assert(lua_type(l, -1) == LUA_TTABLE);

    /*
    lua_pushnil(l);
    while (lua_next(l, -1)) {
        trace(
            "load_rects: %s - %s\n",
            lua_tostring(l, -2),
            lua_tostring(l, -1)
        );
        
        lua_pop(l, 1);
    }
    */

    lua_close(l);
    return rects;
}

static MunitResult test_load(
    const MunitParameter params[], void* data
) {
    const char *example_fname = "example.lua";
    MetaLoader *ml = metaloader_new();
    munit_assert_true(metaloader_load(ml, example_fname));

    int rects_num = 0;
    Rectangle *rects = load_rects(example_fname, &rects_num);
    /*koh_Set *set_rects = set_new();*/

    for (int i = 0; i < rects_num; ++i) {
    }

    if (rects)
        free(rects);

    metaloader_free(ml);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    {
        (char*) "/metaloader_new_free",
        test_new_free,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },
    {
        (char*) "/metaloader_load",
        test_load,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
  (char*) "metaloader", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char **argv) {
    return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);
}
