#pragma once
#ifndef __CHIBI_H__
#define __CHIBI_H__
#include <setjmp.h>

#define CHIBI_TEST_GIT_SHA "$Id: 72c6f3db3710d13bf1995a89992041e29ab2f43d $"

/* DATA STRUCTURES */
typedef struct _chibi_testcase {
    void (*fun)(struct _chibi_testcase *);
    const char *fname;
    struct _chibi_testcase *next;
    int success;
    char *error_msg;
    void *userdata;
    jmp_buf env;
} chibi_testcase;

typedef void (*chibi_testfunc)(chibi_testcase *);
typedef void (*chibi_fixfunc)(void *);

typedef struct _chibi_suite {
    chibi_testcase *head;
    chibi_fixfunc setup, teardown;
    void *userdata;

    /* defines the head of the child list */
    struct _chibi_suite *first_child;

    /* next member in the child list */
    struct _chibi_suite *next;
} chibi_suite;

typedef struct _chibi_summary_data {
    int num_runs;
    int num_pass;
    int num_failures;
} chibi_summary_data;

/* SUITE MANAGEMENT */
extern chibi_suite *chibi_suite_new();
extern chibi_suite *chibi_suite_new_fixture(chibi_fixfunc setup, chibi_fixfunc teardown, void *userdata);
extern void chibi_suite_delete(chibi_suite *suite);
extern void chibi_suite_run(chibi_suite *suite, chibi_summary_data *summary);
extern void chibi_suite_run_silently(chibi_suite *suite, chibi_summary_data *summary);
extern void chibi_suite_run_tap(chibi_suite *suite, chibi_summary_data *summary);

/*
 * We can nest suites. Since every suite can define a fixture, we might define
 * a number of fixtures and run them as part of a larger suite.
 */
extern void chibi_suite_add_suite(chibi_suite *suite, chibi_suite *toadd);

/* don't use this directly */
extern void _chibi_suite_add_test(chibi_suite *suite, chibi_testfunc fun, const char *fname);

/* 
 * ASSERTIONS
 * don't use these directly, use the macros instead, they are more convenient
 * to use.
 */
extern void _chibi_assert_not_null(chibi_testcase *tc, void *ptr, const char *msg,
                                   const char *srcfile, int line);
extern void _chibi_fail(chibi_testcase *tc, const char *msg, const char *srcfile, int line);
extern void _chibi_assert(chibi_testcase *tc, int cond, const char *cond_str, const char *msg,
                          const char *srcfile, int line);
extern void _chibi_assert_eq_int(chibi_testcase *tc, int expected, int value,
                                 const char *srcfile, int line);
extern void _chibi_assert_eq_cstr(chibi_testcase *tc, const char *expected, const char *value,
                                  const char *srcfile, int line);

/* MACROS */
#define chibi_suite_add_test(suite, testfun) (_chibi_suite_add_test(suite, testfun, #testfun))
#define CHIBI_TEST(funname) void funname(chibi_testcase *_tc)

#define chibi_assert_not_null(arg) (_chibi_assert_not_null(_tc, arg, "argument was NULL", __FILE__, __LINE__))
#define chibi_fail(msg) (_chibi_fail(_tc, msg, __FILE__, __LINE__))
#define chibi_assert(cond) (_chibi_assert(_tc, cond, #cond, "condition was wrong:", __FILE__, __LINE__))
#define chibi_assert_msg(cond, msg) (_chibi_assert(_tc, cond, "", msg, __FILE__, __LINE__))
#define chibi_assert_eq_int(expected, value) (_chibi_assert_eq_int(_tc, expected, value, __FILE__, __LINE__))
#define chibi_assert_eq_cstr(expected, value) (_chibi_assert_eq_cstr(_tc, expected, value, __FILE__, __LINE__))

#define TC_USERDATA (_tc->userdata)

#endif /* __CHIBI_H__ */
