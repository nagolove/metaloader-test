// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh.h"
#include "koh_common.h"
#include "koh_metaloader.h"
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
#include "koh_logger.h"

//#define DBG

#ifdef TEST_DEBUG
__attribute__((__format__ (__printf__, 1, 2)))
static int _printf(const char *s, ...) {
    va_list ap;
    va_start(ap, s);
    int ret = vprintf(s, ap); // warning
    va_end(ap);
    return ret;
}
#else
static int _printf(const char *s, ...) {
    (void)s;
    return 0;
}
#endif

static const struct MetaLoaderSetup ml_setup = {
    .verbose = false,
};

static MunitResult test_new_free(
    const MunitParameter params[], void* data
) {
    MetaLoader *ml = metaloader_new(&ml_setup);
    metaloader_free(ml);

    return MUNIT_OK;
}

struct MetaObject {
    Rectangle   rect;
    char        name[128];
};

static koh_Set *control_set_alloc(struct MetaLoaderObjects objs) {
    koh_Set *set_control = set_new();
    assert(set_control);

    for (int i = 0; i < objs.num; ++i) {
        struct MetaObject mobject = {};
        strncpy(mobject.name, objs.names[i], sizeof(mobject.name) - 1);
        mobject.rect = objs.rects[i];
        _printf(
            "control_set_alloc: mobject '%s', %s\n",
            mobject.name, rect2str(mobject.rect)
        );
        set_add(set_control, &mobject, sizeof(mobject));
    }
    _printf("control_set_alloc: objs.num %d\n", objs.num);

    return set_control;
}

static koh_Set *meta_set_alloc(const char *fname, const char *luacode) {
    MetaLoader *ml = metaloader_new(&ml_setup);
    munit_assert(ml != NULL);
    munit_assert_true(metaloader_load_s(ml, fname, luacode));

    const char *fname_noext = extract_filename(fname, ".lua");
    struct MetaLoaderObjects objects = metaloader_objects_get(ml, fname_noext);

    koh_Set *set_meta = set_new();
    for (int j = 0; j < objects.num; ++j) {
        struct MetaObject mobject = {};
        if (objects.names[j])
            strncpy(mobject.name, objects.names[j], sizeof(mobject.name) - 1);
        //mobject.name[1] = 's';
        //objects.rects[j].x += 0.00000000000000000001;
        //objects.rects[j].x += 0.01;
        mobject.rect = objects.rects[j];
        set_add(set_meta, &mobject, sizeof(mobject));
        _printf(
            "meta_set_alloc: mobject '%s', %s\n",
            mobject.name, rect2str(mobject.rect)
        );
    }
    _printf("control_set_alloc: objs.num %d\n", objects.num);
    metaloader_objects_shutdown(&objects);
    metaloader_free(ml);
    return set_meta;
}

static koh_Set *meta_set_alloc_f(const char *fname) {
    MetaLoader *ml = metaloader_new(&ml_setup);
    munit_assert(ml != NULL);
    munit_assert_true(metaloader_load_f(ml, fname));

    const char *fname_noext = extract_filename(fname, ".lua");
    struct MetaLoaderObjects objects = metaloader_objects_get(ml, fname_noext);

    koh_Set *set_meta = set_new();
    for (int j = 0; j < objects.num; ++j) {
        struct MetaObject mobject = {};
        if (objects.names[j])
            strncpy(mobject.name, objects.names[j], sizeof(mobject.name) - 1);
        //mobject.name[1] = 's';
        //objects.rects[j].x += 0.00000000000000000001;
        //objects.rects[j].x += 0.01;
        mobject.rect = objects.rects[j];
        set_add(set_meta, &mobject, sizeof(mobject));
        _printf(
            "meta_set_alloc: mobject '%s', %s\n",
            mobject.name, rect2str(mobject.rect)
        );
    }
    _printf("control_set_alloc: objs.num %d\n", objects.num);
    metaloader_objects_shutdown(&objects);
    metaloader_free(ml);
    return set_meta;
}

static void load_f(
    const char *fname,
    struct MetaLoaderObjects control_objects
) {
    koh_Set *set_meta = meta_set_alloc_f(fname);
    koh_Set *set_control = control_set_alloc(control_objects);
    munit_assert_true(set_compare(set_control, set_meta));
    if (set_control)
        set_free(set_control);
    if (set_meta)
        set_free(set_meta);
}

static void load_s(
    const char *fname_noext,
    const char *luacode,
    struct MetaLoaderObjects control_objects
) {
    koh_Set *set_meta = meta_set_alloc(fname_noext, luacode);
    koh_Set *set_control = control_set_alloc(control_objects);
    munit_assert_true(set_compare(set_control, set_meta));
    if (set_control)
        set_free(set_control);
    if (set_meta)
        set_free(set_meta);
}

static MunitResult test_load_arr(
    const MunitParameter params[], void* data
) {
    load_s(
        "example.lua", 

        "return {\n"
        "    1, 2,\n"
        "}\n",

        (struct MetaLoaderObjects) {
            .names = {},
            .rects = {},
            .num = 0,
        }
    );
    return MUNIT_OK;
}

static MunitResult test_load_s_typed_by_type(
    const MunitParameter params[], void* data
) {

    {
        MetaLoader *ml = metaloader_new(&ml_setup);
        const char *code;

        code = 
            "return {\n"
            "   wheel_100 = {\n"
            "       type = \"rect\",\n"
            "       rect = {0, 0, 100, 100,}\n"
            "   },\n"
            "}\n";

        const char *fname_noext = "MSTA";
        if (!metaloader_load_s(ml, "MSTA.lua", code)) {
            _printf("Could not load code\n");
        }

        struct MetaLoaderReturn *o = metaloader_get2(
            ml, fname_noext, "wheel_100"
        );

        munit_assert_not_null(o);
        munit_assert(o->type == MLT_RECTANGLE);
        const struct MetaLoaderRectangle *rect = (void*)o;
        munit_assert(rect->rect.x == 0.);
        munit_assert(rect->rect.y == 0.);
        munit_assert(rect->rect.width == 100.);
        munit_assert(rect->rect.height == 100.);
        metaloader_return_shutdown(o);
        free(o);
        metaloader_free(ml);
    }

    {
        MetaLoader *ml = metaloader_new(&ml_setup);
        const char *code;

        code = 
            "return {\n"
            "    cabina = {\n"
            "        type = \"rect_oriented\",\n"
            "        rect = {\n"
            "            0, 0, 100, 100,\n"
            "        },\n"
            "        angle = 10.,\n"
            "    },\n"
            "}\n";

        const char *fname_noext = "MSTA";
        if (!metaloader_load_s(ml, "MSTA.lua", code)) {
            _printf("Could not load code\n");
        }

        struct MetaLoaderReturn *o = metaloader_get2(
            ml, fname_noext, "cabina"
        );
        munit_assert_not_null(o);
        munit_assert(o->type == MLT_RECTANGLE_ORIENTED);
        const struct MetaLoaderRectangleOriented *or = (void*)o;
        munit_assert(or->a == 10.);
        munit_assert(or->rect.x == 0.);
        munit_assert(or->rect.y == 0.);
        munit_assert(or->rect.width == 100.);
        munit_assert(or->rect.height == 100.);
        metaloader_return_shutdown(o);
        metaloader_free(ml);
        free(o);
    }

    {
        MetaLoader *ml = metaloader_new(&ml_setup);
        const char *code;

        code = 
            "return {\n"
            "    detal = {\n"
            "        type = \"polyline\",\n"
            "        points = {\n"
            "            5, -5,\n"
            "            4, -4,\n"
            "            3, -3,\n"
            "            2, -2,\n"
            "            1, -1,\n"
            "            0, -0\n"
            "        },\n"
            "    },\n"
            "}\n";

        const char *fname_noext = "MSTA";
        if (!metaloader_load_s(ml, "MSTA.lua", code)) {
            _printf("Could not load code\n");
        }

        struct MetaLoaderReturn *o = metaloader_get2(
            ml, fname_noext, "detal"
        );
        munit_assert_not_null(o);
        munit_assert(o->type == MLT_POLYLINE);
        const struct MetaLoaderPolyline *pl = (void*)o;
        munit_assert(pl->num == 6);
        munit_assert(pl->points[0].x == 5.);
        munit_assert(pl->points[0].y == -5.);
        munit_assert(pl->points[1].x == 4.);
        munit_assert(pl->points[1].y == -4.);
        munit_assert(pl->points[2].x == 3.);
        munit_assert(pl->points[2].y == -3.);
        munit_assert(pl->points[3].x == 2.);
        munit_assert(pl->points[3].y == -2.);
        munit_assert(pl->points[4].x == 1.);
        munit_assert(pl->points[4].y == -1.);
        munit_assert(pl->points[5].x == 0.);
        munit_assert(pl->points[5].y == 0.);
        metaloader_return_shutdown((void*)pl);
        metaloader_free(ml);
        free(o);
    }

    {
        MetaLoader *ml = metaloader_new(&ml_setup);
        const char *code;

        code = 
            "return {\n"
            "   drugaya_detal = {\n"
            "       type = \"sector\",\n"
            "       radius = 100.,\n"
            "       a1 = 0.,\n"
            "       a2 = 3.1415,\n"
            "    },\n"
            "}\n";

        const char *fname_noext = "MSTA";
        if (!metaloader_load_s(ml, "MSTA.lua", code)) {
            _printf("Could not load code\n");
        }

        struct MetaLoaderReturn *o = metaloader_get2(
            ml, fname_noext, "drugaya_detal"
        );
        munit_assert_not_null(o);
        munit_assert(o->type == MLT_SECTOR);
        const struct MetaLoaderSector *sec = (void*)o;
        munit_assert(sec->angle1 == 0.);
        munit_assert(sec->angle2 - 3.1415 <= FLT_EPSILON);
        munit_assert(sec->radius == 100.);
        metaloader_return_shutdown((void*)sec);
        metaloader_free(ml);
        free(o);
    }

    return MUNIT_OK;
}

static MunitResult test_load_s_typed(
    const MunitParameter params[], void* data
) {
    MetaLoader *ml = metaloader_new(&ml_setup);
    const char *code;

    code = 
        "return {\n"
        "    cabina = {\n"
        "        type = \"rect_oriented\",\n"
        "        rect = {\n"
        "            0, 0, 100, 100,\n"
        "        },\n"
        "        angle = 10.,\n"
        "    },\n"
        "}\n";

    const char *fname_noext = "MSTA";
    if (!metaloader_load_s(ml, "MSTA.lua", code)) {
        _printf("Could not load code\n");
    }

    {
        struct MetaLoaderReturn *o = metaloader_get2(
            ml, fname_noext, "cabina"
        );
        munit_assert_not_null(o);
        munit_assert(o->type == MLT_RECTANGLE_ORIENTED);
        const struct MetaLoaderRectangleOriented *or = (void*)o;
        munit_assert(or->a == 10.);
        munit_assert(or->rect.x == 0.);
        munit_assert(or->rect.y == 0.);
        munit_assert(or->rect.width == 100.);
        munit_assert(or->rect.height == 100.);
        metaloader_return_shutdown(o);
        free(o);
    }

    struct MetaLoaderObjects2 object = metaloader_objects_get2(ml, fname_noext);
    for (int i = 0; i < object.num; ++i) {
        struct MetaLoaderObject2Str o2s;
        o2s = metaloader_object2str(object.objs[i]);
        _printf(
           "test_load_f_typed: %s - %s\n", object.names[i], o2s.s
        );
        if (o2s.is_allocated)
            free((char*)o2s.s);
    }
    metaloader_objects_shutdown2(&object);

    metaloader_free(ml);
    return MUNIT_OK;
}

static MunitResult test_load_f_typed(
    const MunitParameter params[], void* data
) {
    MetaLoader *ml = metaloader_new(&ml_setup);
    const char *fname = "typed.lua" ;
    const char *fname_noext = extract_filename(fname, ".lua");
    if (!metaloader_load_f(ml, fname)) {
        _printf("Could not load '%s' file\n", fname);
    }

    struct MetaLoaderObjects2 object = metaloader_objects_get2(ml, fname_noext);
    for (int i = 0; i < object.num; ++i) {
        struct MetaLoaderObject2Str o2s;
        o2s = metaloader_object2str(object.objs[i]);
        _printf(
           "test_load_f_typed: %s - %s\n", object.names[i], o2s.s
        );
        if (o2s.is_allocated)
            free((char*)o2s.s);
    }
    metaloader_objects_shutdown2(&object);

    metaloader_free(ml);
    return MUNIT_OK;
}


static MunitResult test_load_f(
    const MunitParameter params[], void* data
) {
    load_f(
        "example.lua", 
        (struct MetaLoaderObjects) {
            .names = { 
                "wheel1", "mine"  , "wheel2", "wheel3", "wheel4", "wheel5", 
            },
            .rects = {
                { 0, 0, 100, 100, },
                { 2156, 264, 407, 418 },
                { 2, 20, 43, 43, },
                { 2000, 20, 43, 43, },
                { -20, 20, 43, 43, },
                { 0, 0, 0, 0},
            },
            .num = 6,
        }
    );
    return MUNIT_OK;
}

static MunitResult test_load_mixed(
    const MunitParameter params[], void* data
) {
    load_s(
        "example.lua", 

        "return {\n"
        "    -- return x, y, width, height\n"
        "    wheel1 = { 0, 0, 100, 100, },\n"
        "    1, 2,\n"
        "    mine = { 2156, 264, 407, 418 },\n"
        "    wheel2 = { 2, 20, 43, 43, },\n"
        "    wheel3 = { 2000, 20, 43, 43, },\n"
        "    wheel4 = { -20, 20, 43, 43, },\n"
        "    wheel5 = { 0, 0, 0, 0},\n"
        "}\n",

        (struct MetaLoaderObjects) {
            .names = { 
                "wheel1", "mine"  , "wheel2", "wheel3", "wheel4", "wheel5", 
            },
            .rects = {
                { 0, 0, 100, 100, },
                { 2156, 264, 407, 418 },
                { 2, 20, 43, 43, },
                { 2000, 20, 43, 43, },
                { -20, 20, 43, 43, },
                { 0, 0, 0, 0},
            },
            .num = 6,
        }
    );
    return MUNIT_OK;
}

static void _get(const char *fname, const char *luacode, struct MetaLoaderObjects objects_control) {
    struct MetaLoader *ml = metaloader_new(&ml_setup);
    const char *fname_noext = extract_filename(fname, ".lua");
    //munit_assert_string_equal(fname_noext, fname_only_name);
    munit_assert_true(metaloader_load_s(ml, fname, luacode));
    for (int k = 0; k < objects_control.num; ++k) {
        Rectangle *rect = metaloader_get_fmt(
            ml, fname_noext, "%s", objects_control.names[k]
        );
        munit_assert_ptr_not_null(rect);
        munit_assert(rect_cmp_hard(*rect, objects_control.rects[k]));
    }
    metaloader_free(ml);
}

static MunitResult test_get(
    const MunitParameter params[], void* data
) {

    _get(
        "example.lua",
        "return {\n"
        "    -- return x, y, width, height\n"
        "    wheel1 = { 0, 0, 100, 100, },\n"
        "    mine = { 2156, 264, 407, 418 },\n"
        "    wheel2 = { 2, 20, 43, 43, },\n"
        "    wheel3 = { 2000, 20, 43, 43, },\n"
        "    wheel4 = { -20, 20, 43, 43, },\n"
        "    wheel5 = { 0, 0, 0, 0},\n"
        "}\n",
        (struct MetaLoaderObjects) {
            .names = { 
                "wheel1", "mine"  , "wheel2", "wheel3", "wheel4", "wheel5", 
            },
            .rects = {
                { 0, 0, 100, 100, },
                { 2156, 264, 407, 418 },
                { 2, 20, 43, 43, },
                { 2000, 20, 43, 43, },
                { -20, 20, 43, 43, },
                { 0, 0, 0, 0},
            },
            .num = 6,
        }
    );

    _get(
        "example.lua",
        "return {\n"
        "    -- return x, y, width, height\n"
        "    wheel1 = { 0, 0, 100, 100, },\n"
        "    wheel5 = {},\n"
        "}\n",
        (struct MetaLoaderObjects) {
            .names = { 
                "wheel1"
            },
            .rects = {
                { 0, 0, 100, 100, },
            },
            .num = 1,
        }
    );

    return MUNIT_OK;
}

static MunitResult test_load_normal(
    const MunitParameter params[], void* data
) {
    load_s(
        "example.lua", 

        "return {\n"
        "    -- return x, y, width, height\n"
        "    wheel1 = { 0, 0, 100, 100, },\n"
        "    mine = { 2156, 264, 407, 418 },\n"
        "    wheel2 = { 2, 20, 43, 43, },\n"
        "    wheel3 = { 2000, 20, 43, 43, },\n"
        "    wheel4 = { -20, 20, 43, 43, },\n"
        "    wheel5 = { 0, 0, 0, 0},\n"
        "}\n",

        (struct MetaLoaderObjects) {
            .names = { 
                "wheel1", "mine"  , "wheel2", "wheel3", "wheel4", "wheel5", 
            },
            .rects = {
                { 0, 0, 100, 100, },
                { 2156, 264, 407, 418 },
                { 2, 20, 43, 43, },
                { 2000, 20, 43, 43, },
                { -20, 20, 43, 43, },
                { 0, 0, 0, 0},
            },
            .num = 6,
        }
    );
    return MUNIT_OK;
}

static MunitResult test_load_empty(
    const MunitParameter params[], void* data
) {
    load_s(
        "example.lua", 

        "return {\n"
        "}\n",

        (struct MetaLoaderObjects) {
            .names = {},
            .rects = {},
            .num = 0,
        }
    );
    return MUNIT_OK;
}

static MunitResult test_load_empty_sub(
    const MunitParameter params[], void* data
) {
    load_s(
        "example.lua", 

        "return {\n"
        "xx = {},\n"
        //"xx = { 0, 0, 0, 0},\n"
        "A = { 0, 0, 100, 100},\n"
        "xyx = {},\n"
        //"xyx = { 0, 0, 0, 0},\n"
        "}\n",

        (struct MetaLoaderObjects) {
            .names = { "A", },
            .rects = {
                { 0., 0., 100., 100. },
            },
            .num = 1,
        }
    );

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    {
        (char*) "/new_free",
        test_new_free,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },

    {
        (char*) "/load_normal",
        test_load_normal,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },

    {
        (char*) "/get",
        test_get,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },

    {
        (char*) "/load_empty_sub",
        test_load_empty_sub,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },

    {
        (char*) "/load_empty",
        test_load_empty,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },

    {
        (char*) "/load_arr",
        test_load_arr,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },

    {
        (char*) "/load_mixed",
        test_load_mixed,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },

    {
        (char*) "/load_f",
        test_load_f,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },

    {
        (char*) "/load_s_typed_by_type",
        test_load_s_typed_by_type,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },

    {
        (char*) "/load_s_typed",
        test_load_s_typed,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },

    {
        (char*) "/load_f_typed",
        test_load_f_typed,
        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL
    },


    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
  (char*) "metaloader", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char **argv) {
#ifndef DEBUG
    trace_enable(false);
#endif
    return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);
}
