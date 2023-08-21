// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh.h"
#include "koh_common.h"
#include "koh_lua_tools.h"
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

struct MetaObject {
    Rectangle   rect;
    char        name[128];
};

// Возвращает указатель требующий вызова free()
static struct MetaObject *load_rects(
    const char *fname, int *objects_num
) {
    assert(fname);
    assert(objects_num);

    if (!objects_num) {
        printf("objects_num should not be a NULL\n");
        exit(EXIT_FAILURE);
    }
    *objects_num = 0;

    int capacity = 10;
    struct MetaObject *objects = calloc(capacity, sizeof(objects[0]));
    lua_State *l = luaL_newstate();
    luaL_openlibs(l);
    munit_assert_true(luaL_dofile(l, fname) == LUA_OK);
    //printf("load_rect: %s\n", stack_dump(l));

    /* {{{
    lua stack:
    nil, table, string, bool, function, number

    [1.]
    [1., 0., ]
    [1., 0., "D"]
    }}} */

    munit_assert(lua_type(l, -1) == LUA_TTABLE);
    munit_assert(lua_gettop(l));

    lua_pushnil(l);
    while (lua_next(l, 1)) {
        /*
        trace(
            "load_rects: %s - %s\n",
            lua_tostring(l, -2),
            lua_tostring(l, -1)
        );
        */
        struct MetaObject object = {};
        const char *id = lua_tostring(l, -2);
        if (!id)
            goto _next;
        strncpy(object.name, id, sizeof(object.name));

        //trace("[%s]\n", stack_dump(l));
        lua_pushnil(l);
        //trace("[%s]\n", stack_dump(l));
        //printf("gettop() %d\n", lua_gettop(l) - 1);
        int i = 0;
        float values[4] = {};
        while (lua_next(l, lua_gettop(l) - 1)) {
            //printf("%f ", lua_tonumber(l, -2));
            //printf("%f ", lua_tonumber(l, -1));
            if (i < 4)
                values[i] = lua_tonumber(l, -1);
            i++;
            lua_pop(l, 1);
        }
        printf("\n");

        object.rect = rect_from_arr(values);

        if (*objects_num + 1 == capacity) {
            capacity += 1;
            capacity *= 2;
            void *new_mem = realloc(objects, sizeof(objects[0]) * capacity);
            if (!new_mem) {
                printf("bad with realloc()\n");
                exit(EXIT_FAILURE);
            }
            assert(new_mem);
            objects = new_mem;
        }

        objects[(*objects_num)++] = object;
        
_next:  
        lua_pop(l, 1);
    }
    // */

    lua_close(l);
    return objects;
}

static koh_Set *get_control_objects(const char *fname) {
    int objects_num = 0;
    struct MetaObject *objects = load_rects(fname, &objects_num);
    koh_Set *set_control = set_new();
    assert(set_control);

    for (int i = 0; i < objects_num; ++i) {
        /*
        printf(
                "object '%s', %s\n",
                objects[i].name,
                rect2str(objects[i].rect)
              );
        */
        set_add(set_control, &objects[i], sizeof(objects[0]));
    }

    if (objects)
        free(objects);

    return set_control;
}

static MunitResult test_load(
    const MunitParameter params[], void* data
) {
    const char *example_fname = "example.lua";
    MetaLoader *ml = metaloader_new();
    munit_assert_true(metaloader_load(ml, example_fname));

    struct MetaLoaderObjects objects = metaloader_objects_get(
        ml, extract_filename(example_fname, ".lua")
    );

    koh_Set *set_meta = set_new();
    for (int j = 0; j < objects.num; ++j) {
        struct MetaObject mobject = {};
        if (objects.names[j])
            strncpy(mobject.name, objects.names[j], sizeof(mobject.name));
        mobject.rect = objects.rects[j];
        set_add(set_meta, &mobject, sizeof(mobject));

        printf("mobject '%s', %s\n", mobject.name, rect2str(mobject.rect));
    }
    /*metaloader_objects_shutdown(&objects);*/
    metaloader_free(ml);

    koh_Set *set_control = get_control_objects(example_fname);
    if (set_control)
        set_free(set_control);

    if (set_meta)
        set_free(set_meta);

    //munit_assert_true(set_compare(set_control, set_meta));

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
