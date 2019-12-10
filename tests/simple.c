#include <utest.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <pal.h>
/* TODO: This relative path include is fragile */
#include "../src/base/pal_base_private.h"
#include <common.h>

#include "simple.h"

#ifndef FUNCTION
#error FUNCTION must be defined
#endif

#define GOLD_PATH XSTRING(gold/FUNCTION.gold.h)
#include GOLD_PATH

#if !(defined(IS_UNARY) || defined(IS_BINARY))
#error IS_UNARY or IS_BINARY must be defined
#endif

#if (P_FLOAT_TYPE == P_FLOAT_SINGLE)
# define PSCANFMT "%f"
#else
# define PSCANFMT "%lf"
#endif

#define BUILTIN_GOLD_SIZE ARRAY_SIZE(builtin_gold)

struct gold *gold = builtin_gold;
size_t gold_size = ARRAY_SIZE(builtin_gold);
char *gold_file = NULL;

PTYPE *ai, *bi, *res, *ref;

#ifdef __epiphany__
/* Compile time allocate buffers */
PTYPE _ai[BUILTIN_GOLD_SIZE];
PTYPE _bi[BUILTIN_GOLD_SIZE];
PTYPE _ref[BUILTIN_GOLD_SIZE];

/* Allocate one extra element for res and add end marker so overwrites can
 * be detected */
# ifdef SCALAR_OUTPUT
PTYPE _res[2];
# else
PTYPE _res[BUILTIN_GOLD_SIZE + 1];
# endif
#endif

bool generate_gold_flag = false;

/* For detecting erroneous overwrites */
#define OUTPUT_END_MARKER ((PTYPE)60189537703610376.0)

__attribute__((weak))
bool compare(PTYPE x, PTYPE y)
{
    PTYPE err;

    if (fabs(x - y) <= EPSILON_MAX)
        return true;

    if (fabs(x) > fabs(y))
        err = fabs((x - y) / x);
    else
        err = fabs((x - y) / y);

    return err <= EPSILON_RELMAX;
}

#ifdef __epiphany__
static int allocate_buffers()
{
    /* Buffers are compile time allocated */
    ai = _ai;
    bi = _bi;
    ref = _ref;
    res = _res;

    return 0;
}
#else
static int allocate_buffers()
{
    ai = calloc(gold_size, sizeof(PTYPE));
    bi = calloc(gold_size, sizeof(PTYPE));
    ref = calloc(gold_size, sizeof(PTYPE));

    if (!ai || !bi || !ref)
        return ENOMEM;

    /* Allocate one extra element for res and add end marker so overwrites can
     * be detected */
# ifdef SCALAR_OUTPUT
    res = calloc(2, sizeof(PTYPE));
    if (!res)
        return ENOMEM;
# else
    res = calloc(gold_size + 1, sizeof(PTYPE));
    if (!res)
        return ENOMEM;
# endif

    return 0;
}
#endif

int setup(struct ut_suite *suite)
{
    size_t i, err;

    (void) suite;

    err = allocate_buffers();
    if (err)
        return err;

#ifdef SCALAR_OUTPUT
    res[1] = OUTPUT_END_MARKER;
#else
    res[gold_size] = OUTPUT_END_MARKER;
#endif

    for (i = 0; i < gold_size; i++) {
        ai[i] = gold[i].ai;
        bi[i] = gold[i].bi;
    }

    return 0;
}

#ifdef __epiphany__
static void free_buffers()
{
}
#else
static void free_buffers()
{
    free(ai);
    free(bi);
    free(res);
    free(ref);

    /* Need to free if we're not using built-in gold data */
    if (gold_file)
        free(gold);
}
#endif

int teardown(struct ut_suite *suite)
{
    (void) suite;

    free_buffers();

    return 0;
}

#ifndef __epiphany__
int tc_print_gold_e(struct ut_suite *suite, struct ut_tcase *tcase)
{
    size_t i;
    FILE *ofp;

    (void) suite;
    (void) tcase;

    ofp = fopen(XSTRING(FUNCTION.res), "w");
    for (i = 0; i < gold_size; i++)
        fprintf(ofp, "%f,%f,%f,%f\n", ai[i], bi[i], 0.0f, res[i]);
    fclose(ofp);

    fprintf(stdout, "Gold data written to: %s\n", XSTRING(FUNCTION.res));
    fprintf(stdout, "You need to manually copy it to gold/%s\n",
            XSTRING(FUNCTION.dat));

    return 0;
}
#endif

int tc_against_gold_e(struct ut_suite *suite, struct ut_tcase *tcase)
{
    /* Run FUNCTION against gold input */
#if IS_UNARY
    FUNCTION(ai, res, gold_size);
#else /* Binary */
    FUNCTION(ai, bi, res, gold_size);
#endif

    return 0;
}

int tc_against_gold_v(struct ut_suite *suite, struct ut_tcase *tcase)
{
    size_t i;

    /* Skip test if we're generating gold data */
    if (generate_gold_flag)
        return UT_SKIP;

    for (i = 0; i < gold_size; i++) {
#if IS_UNARY
        ut_assert_msg(compare(res[i], gold[i].gold), "%s(%f): %f != %f",
                      XSTRING(FUNCTION), ai[i], res[i], gold[i].gold);
#else
        ut_assert_msg(compare(res[i], gold[i].gold), "%s(%f, %f): %f != %f",
                      XSTRING(FUNCTION), ai[i], bi[i], res[i], gold[i].gold);
#endif
#ifdef SCALAR_OUTPUT /* Scalar output so only first address is valid */
        i++;
        break;
#endif
    }
    ut_assert_msg(res[i] == OUTPUT_END_MARKER,
                  "Output end marker was overwritten");

    return 0;
}

/* Default to using gold data as reference function output */
__attribute__((weak))
void generate_ref(PTYPE *out, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++)
        out[i] = gold[i].gold;
}

int tc_against_ref_v(struct ut_suite *suite, struct ut_tcase *tcase)
{
    size_t i;

    generate_ref(ref, gold_size);

    for (i = 0; i < gold_size; i++) {
#if IS_UNARY
        ut_assert_msg(compare(res[i], ref[i]), "%s(%f): %f != %f",
                      XSTRING(FUNCTION), ai[i], res[i], ref[i]);
#else
        ut_assert_msg(compare(res[i], ref[i]), "%s(%f, %f): %f != %f",
                      XSTRING(FUNCTION), ai[i], bi[i], res[i], ref[i]);
#endif
#ifdef SCALAR_OUTPUT /* Scalar output so only first address is valid */
        i++;
        break;
#endif
    }

    return 0;
}

#ifndef __epiphany__
void parse_options_or_die(int argc, char *argv[])
{
    /* At some point we might to want to use getopt if this gets too messy. */
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            if (gold_file)
                goto usage;

            gold_file = argv[i];
        } else if (!strncmp(argv[i], "--gold", ARRAY_SIZE("--gold")) ||
            !strncmp(argv[i], "-g", ARRAY_SIZE("-g"))) {
            generate_gold_flag = true;
        } else {
            goto usage;
        }
    }
    return;

usage:
    fprintf(stderr,
"Usage: %s [OPTIONS] [GOLD FILE]\n"
"\n"
"OPTIONS\n"
"\n"
"\t-g, --gold\tInstead of running test, generate gold data\n", argv[0]);
    exit(EXIT_FAILURE);
}
#endif

#ifndef __epiphany__
void read_gold_file_or_die(char *progname)
{
    PTYPE ai, bi, res, expect;
    FILE *fp;
    int ret;

    gold = NULL;
    gold_size = 0;

    if (!(fp = fopen(gold_file, "r"))) {
        fprintf(stderr, "%s: Cannot open %s. %s\n",
                progname, gold_file, strerror(errno));
        goto fail;
    }

    while (true) {
retry:
        errno = 0;
        ret = fscanf(fp, PSCANFMT","PSCANFMT","PSCANFMT","PSCANFMT"\n",
                     &ai, &bi, &res, &expect);
        if (ret == EOF) {
            if (errno == EINTR)
                goto retry;

            if (errno) {
                perror("ERROR");
                goto fail;
            }

            if (!gold_size)
                fprintf(stderr, "WARNING: No gold data\n");

            return;
        }
        if (ret != 4) {
                fprintf(stderr, "ERROR: Failed parsing %s at line %zu\n",
                        gold_file, gold_size);
                goto fail;
        }

        gold = realloc(gold, sizeof(*gold) * (gold_size + 1));
        if (!gold) {
                fprintf(stderr, "ERROR: Out of memory\n");
                goto fail;
        }

        gold[gold_size].ai = ai;
        gold[gold_size].bi = bi;
        gold[gold_size].res = res;
        gold[gold_size].gold = expect;

        gold_size++;
    }

fail:
    if (gold)
        free(gold);
    exit(EXIT_FAILURE);
}
#endif

DECLARE_UT_TCASE(tc_against_gold, tc_against_gold_e, tc_against_gold_v, NULL);
DECLARE_UT_TCASE(tc_against_ref, NULL, tc_against_ref_v, NULL);

DECLARE_UT_TCASE_LIST(tcases, &tc_against_gold, &tc_against_ref);

#define FUNCTION_SUITE XCONCAT2(FUNCTION,_suite)
DECLARE_UT_SUITE(FUNCTION_SUITE, setup, teardown, false, tcases, NULL);

#ifndef __epiphany__
#define PRINT_GOLD_SUITE XCONCAT2(FUNCTION,_print_gold_suite)
DECLARE_UT_TCASE(tc_print_gold, tc_print_gold_e, NULL, NULL);
DECLARE_UT_TCASE_LIST(print_gold, &tc_against_gold, &tc_print_gold);
DECLARE_UT_SUITE(PRINT_GOLD_SUITE, setup, teardown, false, print_gold, NULL);
#endif

#ifdef __epiphany__
struct status {
    uint32_t done;
    uint32_t _pad1;
    int32_t  returncode;
    uint32_t _pad2;
} __attribute__((packed));

volatile struct status *epiphany_status = (struct status *) 0x8f200000;
volatile char *epiphany_results = (char *) 0x8f300000;
#endif

int main(int argc, char *argv[])
{
    int ret;
    char buf[1024] = { 0 };

    struct ut_suite *suite;

#ifndef __epiphany__
    parse_options_or_die(argc, argv);

    if (gold_file)
        read_gold_file_or_die(argv[0]);

    if (generate_gold_flag) {
        suite = &PRINT_GOLD_SUITE;
    } else {
#endif
        suite = &FUNCTION_SUITE;
#ifndef __epiphany__
    }
#endif

    ret = ut_run(suite);

    ut_report(buf, ARRAY_SIZE(buf), suite, true);
#ifdef __epiphany__
    memcpy((void *) epiphany_results, buf, sizeof(buf));
    epiphany_status->returncode = ret;
    epiphany_status->done = 1;
#else
    printf("%s", buf);
#endif

    return ret;
}
