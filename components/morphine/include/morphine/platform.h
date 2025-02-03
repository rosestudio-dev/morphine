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
typedef void (*morphine_yield_t)(void *);
#endif

// throw

typedef void (*morphine_try_t)(void *);
typedef void (*morphine_catch_t)(void *);

// native

typedef void (*morphine_native_t)(morphine_coroutine_t);

// userdata

typedef void (*morphine_userdata_constructor_t)(morphine_instance_t, void *);
typedef void (*morphine_userdata_destructor_t)(morphine_instance_t, void *);
typedef int (*morphine_userdata_compare_t)(morphine_instance_t, void *, void *);
typedef ml_hash (*morphine_userdata_hash_t)(morphine_instance_t, void *);

// stream

typedef enum {
    MORPHINE_STREAM_SEEK_MODE_SET,
    MORPHINE_STREAM_SEEK_MODE_CUR,
    MORPHINE_STREAM_SEEK_MODE_PRV,
    MORPHINE_STREAM_SEEK_MODE_END,
} morphine_stream_seek_mode_t;

typedef void (*morphine_stream_open_t)(morphine_instance_t, void *data, void *args);
typedef void (*morphine_stream_close_t)(morphine_instance_t, void *data);
typedef size_t (*morphine_stream_read_t)(morphine_instance_t, void *data, uint8_t *buffer, size_t size);
typedef size_t (*morphine_stream_write_t)(morphine_instance_t, void *data, const uint8_t *buffer, size_t size);
typedef void (*morphine_stream_flush_t)(morphine_instance_t, void *data);
typedef bool (*morphine_stream_seek_t)(morphine_instance_t, void *data, size_t, morphine_stream_seek_mode_t);
typedef size_t (*morphine_stream_tell_t)(morphine_instance_t, void *data);
typedef bool (*morphine_stream_eos_t)(morphine_instance_t, void *data);

typedef struct {
    size_t data_size;
    morphine_stream_open_t open;
    morphine_stream_close_t close;
    morphine_stream_read_t read;
    morphine_stream_write_t write;
    morphine_stream_flush_t flush;
    morphine_stream_seek_t seek;
    morphine_stream_tell_t tell;
    morphine_stream_eos_t eos;
} morphine_stream_interface_t;

// platform

typedef void *(*morphine_malloc_t)(void *data, size_t size);
typedef void *(*morphine_realloc_t)(void *data, void *pointer, size_t size);
typedef void (*morphine_free_t)(void *data, void *pointer);
typedef void (*morphine_signal_t)(morphine_instance_t, void *data, bool is_panic) morphine_noret;

typedef struct {
    morphine_malloc_t alloc;
    morphine_realloc_t realloc;
    morphine_free_t free;
} morphine_platform_memory_t;

typedef struct {
    morphine_stream_interface_t io;
    morphine_stream_interface_t err;
} morphine_platform_stream_t;

typedef struct {
    morphine_signal_t signal;
    morphine_platform_memory_t memory;
    morphine_platform_stream_t stream;

#ifdef MORPHINE_ENABLE_INTERPRETER_YIELD
    morphine_yield_t yield;
#endif
} morphine_platform_t;

// settings

typedef struct {
    size_t limit;     // bytes
    size_t threshold; // bytes
    size_t pause;     // 2^n bytes
    size_t grow;      // percentage
    size_t deal;      // percentage
} morphine_settings_gc_t;

typedef struct {
    struct {
        ml_size limit;
    } stack;
} morphine_settings_coroutines_t;

typedef struct {
    morphine_settings_gc_t gc;
    morphine_settings_coroutines_t coroutines;
} morphine_settings_t;

// library

typedef void (*morphine_library_init_t)(morphine_coroutine_t);

typedef struct {
    const char *name;
    const char *sharedkey;
    morphine_library_init_t init;
} morphine_library_t;
