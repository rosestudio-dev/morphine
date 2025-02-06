//
// Created by whyiskra on 14.02.24.
//

#pragma once

#include <morphine/config/version.h>
#include <morphine/config/module.h>

#define MPARAM_TABLE_GROW_PERCENT  (80)
#define MPARAM_TABLE_TREES         (131070)
#define MPARAM_VECTOR_AMORTIZATION (24)
#define MPARAM_SSO_HASHTABLE_ROWS  (256)
#define MPARAM_SSO_HASHTABLE_COLS  (8)
#define MPARAM_SSO_MAX_LEN         (48)
#define MPARAM_STACKTRACE_COUNT    (16)
#define MPARAM_STACKTRACE_LINE_LEN (256)
#define MPARAM_USERTYPE_NAME_LIMIT (256)
#define MPARAM_CALLABLE_ARGS_LIMIT (256)
#define MPARAM_STACK_GROW          (32)
#define MPARAM_CALL_CACHE_LIMIT    (32)
#define MPARAM_MAIN_COROUTINE_NAME ("main")
