//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include <stdarg.h>
#include "config.h"
#include "platform.h"

MORPHINE_API const char *mapi_version(void);
MORPHINE_API int mapi_version_code(void);

MORPHINE_API morphine_instance_t mapi_open(struct platform, struct settings, void *data);
MORPHINE_API void *mapi_instance_data(morphine_instance_t);
MORPHINE_API void mapi_interpreter(morphine_instance_t);
MORPHINE_API void mapi_close(morphine_instance_t);
MORPHINE_API void mapi_userlibs(morphine_instance_t, struct require_loader *table);

MORPHINE_API FILE *mapi_io_out(morphine_instance_t);
MORPHINE_API FILE *mapi_io_in(morphine_instance_t);

MORPHINE_API morphine_instance_t mapi_instance(morphine_coroutine_t);
MORPHINE_API morphine_coroutine_t mapi_coroutine(morphine_instance_t);

MORPHINE_API void mapi_call(morphine_coroutine_t, size_t argc);
MORPHINE_API void mapi_calli(morphine_coroutine_t, size_t argc);
MORPHINE_API void mapi_callself(morphine_coroutine_t, size_t argc);
MORPHINE_API void mapi_callselfi(morphine_coroutine_t, size_t argc);
MORPHINE_API void mapi_push_result(morphine_coroutine_t);
MORPHINE_API void mapi_return(morphine_coroutine_t);
MORPHINE_API void mapi_leave(morphine_coroutine_t);
MORPHINE_API void mapi_continue(morphine_coroutine_t, size_t callstate);
MORPHINE_API size_t mapi_callstate(morphine_coroutine_t);
MORPHINE_API size_t mapi_args(morphine_coroutine_t);
MORPHINE_API void mapi_push_arg(morphine_coroutine_t, size_t index);
MORPHINE_API void mapi_push_env(morphine_coroutine_t);
MORPHINE_API void mapi_push_self(morphine_coroutine_t);
MORPHINE_API void mapi_change_env(morphine_coroutine_t);

MORPHINE_API morphine_noret void mapi_errorf(morphine_coroutine_t, const char *, ...);
MORPHINE_API morphine_noret void mapi_error(morphine_coroutine_t, const char *);
MORPHINE_API void mapi_catchable(morphine_coroutine_t, size_t callstate);
MORPHINE_API void mapi_push_thrown(morphine_coroutine_t);
MORPHINE_API const char *mapi_get_panic_message(morphine_instance_t);

MORPHINE_API void mapi_rload(morphine_coroutine_t, size_t size, const uint8_t *vector);
MORPHINE_API void mapi_load(
    morphine_coroutine_t,
    morphine_init_t init,
    morphine_read_t read,
    morphine_finish_t finish,
    void *args
);

MORPHINE_API void mapi_push_nil(morphine_coroutine_t);
MORPHINE_API void mapi_push_integer(morphine_coroutine_t, ml_integer value);
MORPHINE_API void mapi_push_size(morphine_coroutine_t, size_t value);
MORPHINE_API void mapi_push_index(morphine_coroutine_t, size_t value);
MORPHINE_API void mapi_push_decimal(morphine_coroutine_t, ml_decimal value);
MORPHINE_API void mapi_push_boolean(morphine_coroutine_t, bool value);
MORPHINE_API void mapi_push_raw(morphine_coroutine_t, void *value);

MORPHINE_API ml_integer mapi_get_integer(morphine_coroutine_t);
MORPHINE_API ml_size mapi_get_size(morphine_coroutine_t);
MORPHINE_API ml_size mapi_get_index(morphine_coroutine_t);
MORPHINE_API ml_decimal mapi_get_decimal(morphine_coroutine_t);
MORPHINE_API bool mapi_get_boolean(morphine_coroutine_t);
MORPHINE_API void *mapi_get_raw(morphine_coroutine_t);

MORPHINE_API void mapi_to_integer(morphine_coroutine_t);
MORPHINE_API void mapi_to_decimal(morphine_coroutine_t);
MORPHINE_API void mapi_to_boolean(morphine_coroutine_t);
MORPHINE_API void mapi_to_string(morphine_coroutine_t);

MORPHINE_API const char *mapi_type(morphine_coroutine_t);
MORPHINE_API bool mapi_is(morphine_coroutine_t, const char *type);
MORPHINE_API bool mapi_is_type(morphine_coroutine_t, const char *type);
MORPHINE_API bool mapi_is_callable(morphine_coroutine_t);
MORPHINE_API bool mapi_is_metatype(morphine_coroutine_t);
MORPHINE_API bool mapi_is_iterable(morphine_coroutine_t);

MORPHINE_API ml_size mapi_csize2size(morphine_coroutine_t, size_t value);
MORPHINE_API ml_size mapi_csize2index(morphine_coroutine_t, size_t value);

MORPHINE_API void mapi_push_string(morphine_coroutine_t, const char *str);
MORPHINE_API void mapi_push_stringn(morphine_coroutine_t, const char *str, size_t size);
MORPHINE_API void mapi_push_stringf(morphine_coroutine_t, const char *str, ...);
MORPHINE_API void mapi_push_stringv(morphine_coroutine_t, const char *str, va_list args);
MORPHINE_API const char *mapi_get_string(morphine_coroutine_t);
MORPHINE_API ml_size mapi_string_len(morphine_coroutine_t);
MORPHINE_API void mapi_string_concat(morphine_coroutine_t);

MORPHINE_API void mapi_push_table(morphine_coroutine_t);
MORPHINE_API void mapi_table_clear(morphine_coroutine_t);
MORPHINE_API void mapi_table_copy(morphine_coroutine_t);
MORPHINE_API void mapi_table_mode_mutable(morphine_coroutine_t, bool is_mutable);
MORPHINE_API void mapi_table_mode_fixed(morphine_coroutine_t, bool is_fixed);
MORPHINE_API void mapi_table_mode_lock_metatable(morphine_coroutine_t);
MORPHINE_API void mapi_table_mode_lock(morphine_coroutine_t);
MORPHINE_API bool mapi_table_mode_is_mutable(morphine_coroutine_t);
MORPHINE_API bool mapi_table_mode_is_fixed(morphine_coroutine_t);
MORPHINE_API bool mapi_table_mode_is_locked(morphine_coroutine_t);
MORPHINE_API void mapi_table_set(morphine_coroutine_t);
MORPHINE_API bool mapi_table_get(morphine_coroutine_t);
MORPHINE_API bool mapi_table_remove(morphine_coroutine_t);
MORPHINE_API void mapi_table_getoe(morphine_coroutine_t);
MORPHINE_API void mapi_table_removeoe(morphine_coroutine_t);
MORPHINE_API ml_size mapi_table_len(morphine_coroutine_t);

MORPHINE_API void mapi_push_vector(morphine_coroutine_t, ml_size);
MORPHINE_API void mapi_vector_resize(morphine_coroutine_t, ml_size);
MORPHINE_API void mapi_vector_clear(morphine_coroutine_t);
MORPHINE_API void mapi_vector_copy(morphine_coroutine_t);
MORPHINE_API void mapi_vector_mode_mutable(morphine_coroutine_t, bool is_mutable);
MORPHINE_API void mapi_vector_mode_fixed(morphine_coroutine_t, bool is_fixed);
MORPHINE_API void mapi_vector_mode_lock(morphine_coroutine_t);
MORPHINE_API bool mapi_vector_mode_is_mutable(morphine_coroutine_t);
MORPHINE_API bool mapi_vector_mode_is_fixed(morphine_coroutine_t);
MORPHINE_API bool mapi_vector_mode_is_locked(morphine_coroutine_t);
MORPHINE_API void mapi_vector_set(morphine_coroutine_t, ml_size index);
MORPHINE_API void mapi_vector_get(morphine_coroutine_t, ml_size index);
MORPHINE_API void mapi_vector_add(morphine_coroutine_t, ml_size index);
MORPHINE_API void mapi_vector_remove(morphine_coroutine_t, ml_size index);
MORPHINE_API void mapi_vector_push(morphine_coroutine_t);
MORPHINE_API void mapi_vector_pop(morphine_coroutine_t);
MORPHINE_API void mapi_vector_peek(morphine_coroutine_t);
MORPHINE_API void mapi_vector_push_front(morphine_coroutine_t);
MORPHINE_API void mapi_vector_pop_front(morphine_coroutine_t);
MORPHINE_API void mapi_vector_peek_front(morphine_coroutine_t);
MORPHINE_API ml_size mapi_vector_len(morphine_coroutine_t);

MORPHINE_API void mapi_push_ref(morphine_coroutine_t);
MORPHINE_API void mapi_ref_set(morphine_coroutine_t);
MORPHINE_API void mapi_ref_get(morphine_coroutine_t);

MORPHINE_API void mapi_push_native(morphine_coroutine_t, const char *name, morphine_native_t native);
MORPHINE_API const char *mapi_native_name(morphine_coroutine_t);
MORPHINE_API morphine_native_t mapi_native_function(morphine_coroutine_t);

MORPHINE_API void *mapi_push_userdata(
    morphine_coroutine_t, const char *type,
    size_t size, morphine_free_t free
);
MORPHINE_API void *mapi_push_userdata_vec(
    morphine_coroutine_t, const char *type,
    size_t count, size_t size, morphine_free_t free
);
MORPHINE_API void mapi_userdata_mode_lock_metatable(morphine_coroutine_t);
MORPHINE_API void *mapi_userdata_resize(morphine_coroutine_t, size_t size);
MORPHINE_API void *mapi_userdata_resize_vec(morphine_coroutine_t, size_t count, size_t size);
MORPHINE_API const char *mapi_userdata_type(morphine_coroutine_t);
MORPHINE_API void *mapi_userdata_pointer(morphine_coroutine_t);

MORPHINE_API morphine_coroutine_t mapi_push_coroutine(morphine_coroutine_t);
MORPHINE_API morphine_coroutine_t mapi_get_coroutine(morphine_coroutine_t);
MORPHINE_API void mapi_attach(morphine_coroutine_t);
MORPHINE_API void mapi_coroutine_suspend(morphine_coroutine_t);
MORPHINE_API void mapi_coroutine_kill(morphine_coroutine_t);
MORPHINE_API void mapi_coroutine_resume(morphine_coroutine_t);
MORPHINE_API void mapi_coroutine_priority(morphine_coroutine_t, priority_t priority);
MORPHINE_API const char *mapi_coroutine_status(morphine_coroutine_t);
MORPHINE_API bool mapi_coroutine_is_alive(morphine_coroutine_t);

MORPHINE_API bool mapi_op(morphine_coroutine_t, const char *op);

MORPHINE_API void *mapi_allocator_uni(morphine_instance_t, void *, size_t size);
MORPHINE_API void *mapi_allocator_vec(morphine_instance_t, void *, size_t n, size_t size);
MORPHINE_API void mapi_allocator_free(morphine_instance_t, void *);

MORPHINE_API void mapi_gc_full(morphine_instance_t);
MORPHINE_API void mapi_gc_work(morphine_instance_t);
MORPHINE_API void mapi_gc_force(morphine_instance_t);
MORPHINE_API bool mapi_gc_is_running(morphine_instance_t);
MORPHINE_API void mapi_gc_enable(morphine_instance_t);
MORPHINE_API void mapi_gc_disable(morphine_instance_t);
MORPHINE_API bool mapi_gc_is_enabled(morphine_instance_t);
MORPHINE_API const char *mapi_gc_status(morphine_instance_t);
MORPHINE_API size_t mapi_gc_allocated(morphine_instance_t);
MORPHINE_API size_t mapi_gc_max_allocated(morphine_instance_t);
MORPHINE_API void mapi_gc_reset_max_allocated(morphine_instance_t I);
MORPHINE_API void mapi_gc_change_threshold(morphine_instance_t, size_t value);
MORPHINE_API void mapi_gc_change_grow(morphine_instance_t, uint16_t value);
MORPHINE_API void mapi_gc_change_deal(morphine_instance_t, uint16_t value);
MORPHINE_API void mapi_gc_change_pause(morphine_instance_t, uint8_t value);
MORPHINE_API void mapi_gc_change_cache_callinfo_holding(morphine_instance_t, size_t value);
MORPHINE_API void mapi_gc_change_finalizer_stack_limit(morphine_instance_t, size_t value);
MORPHINE_API void mapi_gc_change_finalizer_stack_grow(morphine_instance_t, size_t value);
MORPHINE_API void mapi_gc_change_stack_limit(morphine_coroutine_t, size_t value);
MORPHINE_API void mapi_gc_change_stack_grow(morphine_coroutine_t, size_t value);

MORPHINE_API bool mapi_metatable_test(morphine_coroutine_t, const char *field);
MORPHINE_API void mapi_set_metatable(morphine_coroutine_t);
MORPHINE_API void mapi_get_metatable(morphine_coroutine_t);
MORPHINE_API void mapi_set_default_metatable(morphine_coroutine_t, const char *type);
MORPHINE_API void mapi_get_default_metatable(morphine_coroutine_t, const char *type);

MORPHINE_API void mapi_rotate(morphine_coroutine_t, size_t count);
MORPHINE_API void mapi_pop(morphine_coroutine_t, size_t size);
MORPHINE_API void mapi_peek(morphine_coroutine_t, size_t offset);
MORPHINE_API void mapi_move(morphine_coroutine_t, morphine_coroutine_t to);
MORPHINE_API void mapi_copy(morphine_coroutine_t, morphine_coroutine_t to, size_t offset);
MORPHINE_API void mapi_stack_reset(morphine_coroutine_t);
MORPHINE_API size_t mapi_stack(morphine_coroutine_t);

MORPHINE_API void mapi_registry_set_key(morphine_coroutine_t);
MORPHINE_API bool mapi_registry_get(morphine_coroutine_t);
MORPHINE_API void mapi_registry_getoe(morphine_coroutine_t);
MORPHINE_API void mapi_registry_set(morphine_coroutine_t);
MORPHINE_API void mapi_registry_clear(morphine_coroutine_t);

MORPHINE_API void mapi_iterator(morphine_coroutine_t);
MORPHINE_API void mapi_iterator_init(morphine_coroutine_t);
MORPHINE_API bool mapi_iterator_has(morphine_coroutine_t);
MORPHINE_API void mapi_iterator_next(morphine_coroutine_t);
