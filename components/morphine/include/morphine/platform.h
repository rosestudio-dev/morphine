//
// Created by whyiskra on 28.04.23.
//

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "config.h"
#include "attrs.h"
#include "limits.h"

// common

typedef struct coroutine *morphine_coroutine_t;
typedef struct instance *morphine_instance_t;

// yield

#ifdef MORPHINE_ENABLE_INTERPRETER_YIELD
typedef void (*mfunc_yield_t)(void *);
#endif

// throw

typedef enum {
    MTYPE_CATCH_SUCCESS,
    MTYPE_CATCH_PROVIDE,
    MTYPE_CATCH_CRASH,
} mtype_catch_t;

typedef void (*mfunc_try_t)(void *);
typedef mtype_catch_t (*mfunc_catch_t)(void *);

// native

typedef void (*mfunc_native_t)(morphine_coroutine_t);

// userdata

typedef void (*mfunc_constructor_t)(morphine_instance_t, void *);
typedef void (*mfunc_destructor_t)(morphine_instance_t, void *);
typedef int (*mfunc_compare_t)(morphine_instance_t, void *, void *);
typedef ml_hash (*mfunc_hash_t)(morphine_instance_t, void *);

// stream

typedef enum {
    MTYPE_SEEK_SET,
    MTYPE_SEEK_CUR,
    MTYPE_SEEK_PRV,
    MTYPE_SEEK_END,
} mtype_seek_t;

typedef void (*mfunc_open_t)(morphine_instance_t, void *, void *args);
typedef void (*mfunc_close_t)(morphine_instance_t, void *);
typedef size_t (*mfunc_read_t)(morphine_instance_t, void *, uint8_t *buffer, size_t size);
typedef size_t (*mfunc_write_t)(morphine_instance_t, void *, const uint8_t *buffer, size_t size);
typedef void (*mfunc_flush_t)(morphine_instance_t, void *);
typedef bool (*mfunc_seek_t)(morphine_instance_t, void *, size_t, mtype_seek_t);
typedef size_t (*mfunc_tell_t)(morphine_instance_t, void *);
typedef bool (*mfunc_eos_t)(morphine_instance_t, void *);

typedef struct {
    size_t data_size;
    mfunc_open_t open;
    mfunc_close_t close;
    mfunc_read_t read;
    mfunc_write_t write;
    mfunc_flush_t flush;
    mfunc_seek_t seek;
    mfunc_tell_t tell;
    mfunc_eos_t eos;
} morphine_stream_interface_t;

// platform

typedef void *(*mfunc_malloc_t)(void *data, size_t size);
typedef void *(*mfunc_realloc_t)(void *data, void *pointer, size_t size);
typedef void (*mfunc_free_t)(void *data, void *pointer);
typedef void (*mfunc_signal_t)(morphine_instance_t, void *data, bool is_panic) mattr_noret;

typedef struct {
    struct {
        mfunc_malloc_t alloc;
        mfunc_realloc_t realloc;
        mfunc_free_t free;
    } memory;

    struct {
        morphine_stream_interface_t io;
        morphine_stream_interface_t err;
    } stream;

    mfunc_signal_t signal;

#ifdef MORPHINE_ENABLE_INTERPRETER_YIELD
    mfunc_yield_t yield;
#endif
} morphine_platform_t;

// settings

typedef struct {
    struct {
        size_t limit;     // bytes
        size_t threshold; // bytes
        size_t pause;     // 2^n bytes
        size_t grow;      // percentage
        size_t deal;      // percentage
    } gc;

    struct {
        ml_size limit;
    } stack;
} morphine_settings_t;

// library

typedef struct {
    const char *name;
    const char *sharedkey;
    mfunc_native_t init;
} morphine_library_t;
