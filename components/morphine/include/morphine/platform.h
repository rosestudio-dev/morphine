//
// Created by whyiskra on 28.04.23.
//

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "config.h"
#include "limits.h"

// common

typedef uint32_t ml_version;

typedef struct coroutine *morphine_coroutine_t;
typedef struct instance *morphine_instance_t;

// native

typedef void (*morphine_native_t)(morphine_coroutine_t);

// userdata

typedef void (*morphine_userdata_init_t)(morphine_instance_t, void *);
typedef void (*morphine_userdata_free_t)(morphine_instance_t, void *);
typedef int (*morphine_userdata_compare_t)(morphine_instance_t, void *, void *);
typedef ml_hash (*morphine_userdata_hash_t)(morphine_instance_t, void *);

// sio

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
    size_t limit;     // bytes
    size_t threshold; // bytes
    uint16_t grow;    // percentage / 10
    uint16_t deal;    // percentage / 10
    uint8_t pause;    // 2^n bytes

    struct {
        size_t callinfo;
    } cache;
} morphine_settings_gc_t;

typedef struct {
    struct {
        size_t limit;
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
