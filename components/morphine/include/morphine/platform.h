//
// Created by whyiskra on 28.04.23.
//

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "config.h"
#include "limits.h"

typedef uint32_t ml_version;

typedef struct coroutine *morphine_coroutine_t;
typedef struct instance *morphine_instance_t;
typedef void (*morphine_native_t)(morphine_coroutine_t);
typedef void (*morphine_userdata_init_t)(morphine_instance_t, void *);
typedef void (*morphine_userdata_free_t)(morphine_instance_t, void *);
typedef int (*morphine_userdata_compare_t)(morphine_instance_t, void *, void *);
typedef ml_hash (*morphine_userdata_hash_t)(morphine_instance_t, void *);

// sio types

typedef enum {
    MORPHINE_SIO_SEEK_MODE_SET,
    MORPHINE_SIO_SEEK_MODE_CUR,
    MORPHINE_SIO_SEEK_MODE_PRV,
    MORPHINE_SIO_SEEK_MODE_END,
} morphine_sio_seek_mode_t;

typedef struct sio_accessor *morphine_sio_accessor_t;
typedef void *(*morphine_sio_open_t)(morphine_sio_accessor_t, void *args);
typedef size_t (*morphine_sio_read_t)(morphine_sio_accessor_t, void *data, uint8_t *buffer, size_t size);
typedef size_t (*morphine_sio_write_t)(morphine_sio_accessor_t, void *data, const uint8_t *buffer, size_t size);
typedef void (*morphine_sio_flush_t)(morphine_sio_accessor_t, void *data);
typedef bool (*morphine_sio_seek_t)(morphine_sio_accessor_t, void *data, size_t, morphine_sio_seek_mode_t);
typedef size_t (*morphine_sio_tell_t)(morphine_sio_accessor_t, void *data);
typedef bool (*morphine_sio_eos_t)(morphine_sio_accessor_t, void *data);
typedef void (*morphine_sio_close_t)(morphine_sio_accessor_t, void *data);

typedef struct {
    morphine_sio_open_t open;
    morphine_sio_read_t read;
    morphine_sio_write_t write;
    morphine_sio_flush_t flush;
    morphine_sio_seek_t seek;
    morphine_sio_tell_t tell;
    morphine_sio_eos_t eos;
    morphine_sio_close_t close;
} morphine_sio_interface_t;

// platform

typedef struct {
    struct {
        void *(*malloc)(void *, size_t);
        void *(*realloc)(void *, void *, size_t);
        void (*free)(void *, void *);
        void (*signal)(morphine_instance_t) morphine_noret;
    } functions;

    morphine_sio_interface_t sio_io_interface;
    morphine_sio_interface_t sio_error_interface;
} morphine_platform_t;

// settings

typedef struct {
    size_t limit_bytes;
    size_t threshold;
    uint16_t grow;
    uint16_t deal;
    uint8_t pause;
    size_t cache_callinfo_holding;
} morphine_gc_settings_t;

typedef struct {
    size_t stack_limit;
    size_t stack_grow;
} morphine_coroutine_settings_t;

typedef struct {
    morphine_gc_settings_t gc;
    morphine_coroutine_settings_t states;
    morphine_coroutine_settings_t finalizer;
} morphine_settings_t;

// library

typedef struct {
    const char *name;

    struct {
        size_t allocate;
        morphine_userdata_init_t init;
        morphine_userdata_free_t free;
        morphine_userdata_compare_t compare;
        morphine_userdata_hash_t hash;
        bool require_metatable;
    } params;
} morphine_library_type_t;

typedef struct {
    const char *name;
    morphine_native_t function;
} morphine_library_function_t;

typedef struct {
    const char *name;
    ml_integer integer;
} morphine_library_integer_t;

typedef struct {
    const char *name;
    ml_decimal decimal;
} morphine_library_decimal_t;

typedef struct {
    const char *name;
    const char *string;
} morphine_library_string_t;

typedef struct {
    const char *name;

    morphine_library_type_t *types;
    morphine_library_function_t *functions;
    morphine_library_integer_t *integers;
    morphine_library_decimal_t *decimals;
    morphine_library_string_t *strings;
} morphine_library_t;
