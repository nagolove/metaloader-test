// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh.h"
#include "koh_metaloader.h"
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

static MunitResult test_new_free(
    const MunitParameter params[], void* data
) {
    MetaLoader *ml = metaloader_new();
    metaloader_free(ml);

    return MUNIT_OK;
}

static MunitResult test_load(
    const MunitParameter params[], void* data
) {
    MetaLoader *ml = metaloader_new();
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
