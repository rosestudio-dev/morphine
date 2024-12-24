//
// Created by why-iskra on 24.12.2024.
//

#include "morphine/misc/isolate.h"
#include "morphine/misc/packer.h"
#include "morphine/object/sio.h"
#include "morphine/object/vector.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/userdata.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/safe.h"
#include "morphine/core/instance.h"
#include "morphine/utils/overflow.h"
#include <setjmp.h>

#define BRIDGE_FACTOR (16)

struct isolate {
    morphine_instance_t original_instance;

    morphine_instance_t isolate_instance;
    bool need_close;
    jmp_buf jump;

    morphine_isolate_config_t config;

    struct {
        size_t allocated;
        size_t size;
        size_t pointer;
        uint8_t *data;
    } bridge;
};

// sio bridge functions

static size_t bridge_read(morphine_instance_t I, void *data, uint8_t *buffer, size_t size) {
    (void) I;
    struct isolate *isolate = data;

    size_t read = 0;
    for (size_t i = 0; i < size; i++) {
        if (isolate->bridge.pointer >= isolate->bridge.size) {
            buffer[i] = 0;
        } else {
            buffer[i] = isolate->bridge.data[isolate->bridge.pointer];
            isolate->bridge.pointer++;
            read++;
        }
    }

    return read;
}

static size_t bridge_write(morphine_instance_t I, void *data, const uint8_t *buffer, size_t size) {
    struct isolate *isolate = data;

    for (size_t i = 0; i < size; i++) {
        if (isolate->bridge.pointer >= isolate->bridge.size) {
            overflow_add(isolate->bridge.size, 1, SIZE_MAX) {
                throwI_error(I, "bridge overflow");
            }

            isolate->bridge.size++;
        }

        if (isolate->bridge.size >= isolate->bridge.allocated) {
            overflow_add(isolate->bridge.allocated, BRIDGE_FACTOR, SIZE_MAX) {
                throwI_error(I, "bridge overflow");
            }

            size_t new_size = isolate->bridge.allocated + BRIDGE_FACTOR;
            isolate->bridge.data = allocI_uni(isolate->original_instance, isolate->bridge.data, new_size);
            isolate->bridge.allocated = new_size;
        }

        isolate->bridge.data[isolate->bridge.pointer] = buffer[i];
        isolate->bridge.pointer++;
    }

    return size;
}

static void bridge_reset(struct isolate *isolate) {
    isolate->bridge.pointer = 0;
}

static void bridge_clear(struct isolate *isolate) {
    allocI_free(isolate->original_instance, isolate->bridge.data);
    isolate->bridge.data = NULL;

    isolate->bridge.allocated = 0;
    isolate->bridge.size = 0;
    isolate->bridge.pointer = 0;
}

static struct sio *create_bridge_sio(morphine_instance_t I, struct isolate *isolate, struct userdata *userdata) {
    morphine_sio_interface_t interface = {
        .open = NULL,
        .read = bridge_read,
        .write = bridge_write,
        .flush = NULL,
        .seek = NULL,
        .tell = NULL,
        .eos = NULL,
        .close = NULL
    };

    gcI_safe_enter(I);
    struct sio *sio = gcI_safe_obj(I, sio, sioI_create(I, interface));
    if (userdata != NULL) {
        sioI_hold(I, sio, valueI_object(userdata));
    }

    sioI_open(I, sio, isolate);
    gcI_safe_exit(I);

    return sio;
}

// instance functions

static size_t isolate_io_read(morphine_instance_t I, void *data, uint8_t *buffer, size_t size) {
    (void) I;
    struct isolate *isolate = data;
    if (isolate->config.custom.flags.sio) {
        return isolate->config.custom.sio.io.read(
            isolate->original_instance,
            isolate->config.custom.data,
            buffer,
            size
        );
    } else {
        return sioI_read(isolate->original_instance, isolate->original_instance->sio.io, buffer, size);
    }
}

static size_t isolate_io_write(morphine_instance_t I, void *data, const uint8_t *buffer, size_t size) {
    (void) I;
    struct isolate *isolate = data;
    if (isolate->config.custom.flags.sio) {
        return isolate->config.custom.sio.io.write(
            isolate->original_instance,
            isolate->config.custom.data,
            buffer,
            size
        );
    } else {
        return sioI_write(isolate->original_instance, isolate->original_instance->sio.io, buffer, size);
    }
}

static void isolate_io_flush(morphine_instance_t I, void *data) {
    (void) I;
    struct isolate *isolate = data;
    if (isolate->config.custom.flags.sio) {
        isolate->config.custom.sio.io.flush(isolate->original_instance, isolate->config.custom.data);
    } else {
        sioI_flush(isolate->original_instance, isolate->original_instance->sio.io);
    }
}

static bool isolate_io_seek(morphine_instance_t I, void *data, size_t offset, morphine_sio_seek_mode_t mode) {
    (void) I;
    struct isolate *isolate = data;
    if (isolate->config.custom.flags.sio) {
        return isolate->config.custom.sio.io.seek(
            isolate->original_instance,
            isolate->config.custom.data,
            offset,
            mode
        );
    } else {
        switch (mode) {
            case MORPHINE_SIO_SEEK_MODE_SET:
                return sioI_seek_set(isolate->original_instance, isolate->original_instance->sio.io, offset);
            case MORPHINE_SIO_SEEK_MODE_CUR:
                return sioI_seek_cur(isolate->original_instance, isolate->original_instance->sio.io, offset);
            case MORPHINE_SIO_SEEK_MODE_PRV:
                return sioI_seek_prv(isolate->original_instance, isolate->original_instance->sio.io, offset);
            case MORPHINE_SIO_SEEK_MODE_END:
                return sioI_seek_end(isolate->original_instance, isolate->original_instance->sio.io, offset);
        }

        return false;
    }
}

static size_t isolate_io_tell(morphine_instance_t I, void *data) {
    (void) I;
    struct isolate *isolate = data;
    if (isolate->config.custom.flags.sio) {
        return isolate->config.custom.sio.io.tell(isolate->original_instance, isolate->config.custom.data);
    } else {
        return sioI_tell(isolate->original_instance, isolate->original_instance->sio.io);
    }
}

static bool isolate_io_eos(morphine_instance_t I, void *data) {
    (void) I;
    struct isolate *isolate = data;
    if (isolate->config.custom.flags.sio) {
        return isolate->config.custom.sio.io.eos(isolate->original_instance, isolate->config.custom.data);
    } else {
        return sioI_eos(isolate->original_instance, isolate->original_instance->sio.io);
    }
}

static size_t isolate_err_read(morphine_instance_t I, void *data, uint8_t *buffer, size_t size) {
    (void) I;
    struct isolate *isolate = data;
    if (isolate->config.custom.flags.sio) {
        return isolate->config.custom.sio.err.read(
            isolate->original_instance,
            isolate->config.custom.data,
            buffer,
            size
        );
    } else {
        return sioI_read(isolate->original_instance, isolate->original_instance->sio.err, buffer, size);
    }
}

static size_t isolate_err_write(morphine_instance_t I, void *data, const uint8_t *buffer, size_t size) {
    (void) I;
    struct isolate *isolate = data;
    if (isolate->config.custom.flags.sio) {
        return isolate->config.custom.sio.err.write(
            isolate->original_instance,
            isolate->config.custom.data,
            buffer,
            size
        );
    } else {
        return sioI_write(isolate->original_instance, isolate->original_instance->sio.err, buffer, size);
    }
}

static void isolate_err_flush(morphine_instance_t I, void *data) {
    (void) I;
    struct isolate *isolate = data;
    if (isolate->config.custom.flags.sio) {
        isolate->config.custom.sio.err.flush(isolate->original_instance, isolate->config.custom.data);
    } else {
        sioI_flush(isolate->original_instance, isolate->original_instance->sio.err);
    }
}

static bool isolate_err_seek(morphine_instance_t I, void *data, size_t offset, morphine_sio_seek_mode_t mode) {
    (void) I;
    struct isolate *isolate = data;
    if (isolate->config.custom.flags.sio) {
        return isolate->config.custom.sio.err.seek(
            isolate->original_instance,
            isolate->config.custom.data,
            offset,
            mode
        );
    } else {
        switch (mode) {
            case MORPHINE_SIO_SEEK_MODE_SET:
                return sioI_seek_set(isolate->original_instance, isolate->original_instance->sio.err, offset);
            case MORPHINE_SIO_SEEK_MODE_CUR:
                return sioI_seek_cur(isolate->original_instance, isolate->original_instance->sio.err, offset);
            case MORPHINE_SIO_SEEK_MODE_PRV:
                return sioI_seek_prv(isolate->original_instance, isolate->original_instance->sio.err, offset);
            case MORPHINE_SIO_SEEK_MODE_END:
                return sioI_seek_end(isolate->original_instance, isolate->original_instance->sio.err, offset);
        }

        return false;
    }
}

static size_t isolate_err_tell(morphine_instance_t I, void *data) {
    (void) I;
    struct isolate *isolate = data;
    if (isolate->config.custom.flags.sio) {
        return isolate->config.custom.sio.err.tell(isolate->original_instance, isolate->config.custom.data);
    } else {
        return sioI_tell(isolate->original_instance, isolate->original_instance->sio.err);
    }
}

static bool isolate_err_eos(morphine_instance_t I, void *data) {
    (void) I;
    struct isolate *isolate = data;
    if (isolate->config.custom.flags.sio) {
        return isolate->config.custom.sio.err.eos(isolate->original_instance, isolate->config.custom.data);
    } else {
        return sioI_eos(isolate->original_instance, isolate->original_instance->sio.err);
    }
}

static void *isolate_alloc(void *data, size_t size) {
    struct isolate *isolate = data;
    if (isolate->config.custom.flags.memory) {
        return isolate->config.custom.memory.alloc(isolate->config.custom.data, size);
    } else {
        return allocI_uni(isolate->original_instance, NULL, size);
    }
}

static void *isolate_realloc(void *data, void *pointer, size_t size) {
    struct isolate *isolate = data;
    if (isolate->config.custom.flags.memory) {
        return isolate->config.custom.memory.realloc(isolate->config.custom.data, pointer, size);
    } else {
        return allocI_uni(isolate->original_instance, pointer, size);
    }
}

static void isolate_free(void *data, void *pointer) {
    struct isolate *isolate = data;
    if (isolate->config.custom.flags.memory) {
        isolate->config.custom.memory.free(isolate->config.custom.data, pointer);
    } else {
        allocI_free(isolate->original_instance, pointer);
    }
}

morphine_noret static void isolate_signal(morphine_instance_t I, void *data, bool is_panic) {
    struct isolate *isolate = data;
    if (I == NULL) {
        throwI_error(isolate->original_instance, "allocation isolate instance failed");
    }

    if (throwI_is_nested_signal(I) || is_panic) {
        throwI_panicf(isolate->original_instance, "isolate panic (%s)", throwI_message(I));
    }

    if (isolate->config.error_propagation) {
        throwI_error(isolate->original_instance, throwI_message(I));
    } else {
        sioI_printf(
            isolate->original_instance,
            isolate->original_instance->sio.err,
            "morphine isolate error: %s\n",
            throwI_message(I)
        );

        longjmp(isolate->jump, 1);
    }
}

static const morphine_sio_interface_t isolate_io_interface = {
    .open = NULL,
    .close = NULL,
    .read = isolate_io_read,
    .write = isolate_io_write,
    .flush = isolate_io_flush,
    .seek = isolate_io_seek,
    .tell = isolate_io_tell,
    .eos = isolate_io_eos
};

static const morphine_sio_interface_t isolate_error_interface = {
    .open = NULL,
    .close = NULL,
    .read = isolate_err_read,
    .write = isolate_err_write,
    .flush = isolate_err_flush,
    .seek = isolate_err_seek,
    .tell = isolate_err_tell,
    .eos = isolate_err_eos
};

static const morphine_platform_t isolate_platform = {
    .signal = isolate_signal,
    .memory.alloc = isolate_alloc,
    .memory.realloc = isolate_realloc,
    .memory.free = isolate_free,
    .sio.io = isolate_io_interface,
    .sio.err = isolate_error_interface
};

// isolate

static void isolate_run(struct isolate *isolate) {
    morphine_instance_t I = isolate->isolate_instance;

    if (isolate->config.init != NULL) {
        isolate->config.init(I);
    }

    gcI_safe_enter(I);

    struct value env = valueI_object(I->env);
    struct string *name = gcI_safe_obj(I, string, stringI_create(I, "isolate"));
    morphine_coroutine_t U = gcI_safe_obj(I, coroutine, coroutineI_create(I, name, env));

    struct sio *bridge_sio = gcI_safe_obj(I, sio, create_bridge_sio(I, isolate, NULL));

    ml_size size;
    {
        gcI_safe_enter(I);
        struct value value_vector = gcI_safe(I, packerI_from(I, bridge_sio));
        struct vector *vector = valueI_as_vector_or_error(I, value_vector);
        size = vectorI_size(I, vector);
        for (ml_size i = 0; i < size; i++) {
            struct value value = vectorI_get(I, vector, i);
            stackI_push(U, value);
        }

        bridge_clear(isolate);
        gcI_safe_exit(I);
    }

    callstackI_call_from_api(U, false, false, size - 1);

    if (isolate->config.error_propagation) {
        throwI_crashable(U);
    }

    coroutineI_attach(U);

    interpreterI_run(I);

    struct value result = gcI_safe(I, callstackI_result(U));
    packerI_to(I, bridge_sio, result);

    gcI_safe_exit(I);
}

static void isolate_close(struct isolate *isolate) {
    if (isolate->need_close && isolate->isolate_instance != NULL) {
        isolate->need_close = false;
        instanceI_close(isolate->isolate_instance);
    }
}

static void isolate_destructor(morphine_instance_t I, void *data) {
    (void) I;
    struct isolate *isolate = data;
    isolate_close(isolate);
    bridge_clear(isolate);
}

struct value isolateI_call(
    morphine_instance_t I,
    morphine_isolate_config_t config,
    struct value value,
    struct value *args,
    ml_size size
) {
    gcI_safe_enter(I);

    struct value result = valueI_nil;

    struct sio *bridge;
    struct isolate *isolate;
    {
        struct userdata *userdata = gcI_safe_obj(I, userdata, userdataI_create(I, sizeof(struct isolate)));
        isolate = userdata->data;
        *isolate = (struct isolate) {
            .original_instance = I,
            .isolate_instance = NULL,
            .need_close = true,
            .config = config,
            .bridge.allocated = 0,
            .bridge.size = 0,
            .bridge.pointer = 0,
            .bridge.data = NULL
        };

        userdataI_set_destructor(I, userdata, isolate_destructor);

        bridge = gcI_safe_obj(I, sio, create_bridge_sio(I, isolate, userdata));
    }

    if (setjmp(isolate->jump) == 1) {
        result = valueI_nil;
        goto exit;
    }

    {
        gcI_safe_enter(I);
        overflow_add(size, 1, MLIMIT_SIZE_MAX) {
            throwI_error(I, "arguments overflow");
        }

        struct vector *vector = gcI_safe_obj(I, vector, vectorI_create(I, size + 1));
        vectorI_set(I, vector, 0, value);
        for (ml_size i = 0; i < size; i++) {
            vectorI_set(I, vector, i + 1, args[i]);
        }

        packerI_to(I, bridge, valueI_object(vector));
        bridge_reset(isolate);
        gcI_safe_exit(I);
    }

    isolate->isolate_instance = instanceI_open(isolate_platform, I->settings, isolate);
    isolate_run(isolate);
    isolate_close(isolate);

    bridge_reset(isolate);
    result = packerI_from(I, bridge);

exit:
    gcI_safe_exit(I);
    return result;
}
