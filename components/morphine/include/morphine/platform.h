//
// Created by whyiskra on 28.04.23.
//

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "config.h"
#include "limits.h"

typedef struct coroutine *morphine_coroutine_t;
typedef struct instance *morphine_instance_t;
typedef void (*morphine_native_t)(morphine_coroutine_t);
typedef void (*morphine_free_t)(morphine_instance_t, void *);

typedef uint8_t priority_t;

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

struct platform {
    struct {
        void *(*malloc)(size_t);
        void *(*realloc)(void *, size_t);
        void (*free)(void *);
        void (*signal)(morphine_instance_t) morphine_noret;
    } functions;

    morphine_sio_interface_t sio_io_interface;
    morphine_sio_interface_t sio_error_interface;
};

struct gc_settings {
    size_t limit_bytes;
    size_t threshold;
    uint16_t grow;
    uint16_t deal;
    uint8_t pause;
    size_t cache_callinfo_holding;
};

struct coroutine_settings {
    size_t stack_limit;
    size_t stack_grow;
};

struct settings {
    struct gc_settings gc;
    struct coroutine_settings states;
    struct coroutine_settings finalizer;
};

struct require_loader {
    const char *name;
    void (*loader)(morphine_coroutine_t);
};
