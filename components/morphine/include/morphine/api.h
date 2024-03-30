//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include <stdarg.h>
#include "config.h"
#include "platform.h"

MORPHINE_API const char *mapi_version(void);

MORPHINE_API morphine_instance_t mapi_open(struct platform, struct params, void *userdata);
MORPHINE_API void *mapi_userdata(morphine_instance_t);
MORPHINE_API void mapi_interpreter(morphine_instance_t);
MORPHINE_API void mapi_close(morphine_instance_t);
MORPHINE_API void mapi_userlibs(morphine_instance_t, struct require_loader *table);

MORPHINE_API FILE *mapi_io_out(morphine_instance_t);
MORPHINE_API FILE *mapi_io_in(morphine_instance_t);

MORPHINE_API morphine_instance_t mapi_instance(morphine_state_t);
MORPHINE_API morphine_state_t mapi_state(morphine_instance_t);

MORPHINE_API void mapi_call(morphine_state_t, size_t argc);
MORPHINE_API void mapi_calli(morphine_state_t, size_t argc);
MORPHINE_API void mapi_callself(morphine_state_t, size_t argc);
MORPHINE_API void mapi_callselfi(morphine_state_t, size_t argc);
MORPHINE_API void mapi_push_result(morphine_state_t);
MORPHINE_API void mapi_return(morphine_state_t);
MORPHINE_API void mapi_leave(morphine_state_t);
MORPHINE_API void mapi_continue(morphine_state_t, size_t callstate);
MORPHINE_API size_t mapi_callstate(morphine_state_t);
MORPHINE_API size_t mapi_args(morphine_state_t);
MORPHINE_API void mapi_push_arg(morphine_state_t, size_t index);
MORPHINE_API void mapi_push_env(morphine_state_t);
MORPHINE_API void mapi_push_self(morphine_state_t);
MORPHINE_API void mapi_changeenv(morphine_state_t);

MORPHINE_API morphine_noret void mapi_errorf(morphine_state_t, const char *str, ...);
MORPHINE_API morphine_noret void mapi_error(morphine_state_t);
MORPHINE_API morphine_noret void mapi_panic(morphine_state_t, const char *message);
MORPHINE_API void mapi_catchable(morphine_state_t, size_t callstate);
MORPHINE_API void mapi_push_thrown(morphine_state_t);
MORPHINE_API const char *mapi_get_panic_message(morphine_instance_t);

MORPHINE_API void mapi_rload(morphine_state_t, size_t size, const uint8_t *vector);
MORPHINE_API void mapi_load(
    morphine_state_t,
    morphine_loader_init_t init,
    morphine_loader_read_t read,
    morphine_loader_finish_t finish,
    void *args
);

MORPHINE_API void mapi_push_nil(morphine_state_t);
MORPHINE_API void mapi_push_integer(morphine_state_t, morphine_integer_t value);
MORPHINE_API void mapi_push_size(morphine_state_t, size_t value);
MORPHINE_API void mapi_push_decimal(morphine_state_t, morphine_decimal_t value);
MORPHINE_API void mapi_push_boolean(morphine_state_t, bool value);
MORPHINE_API void mapi_push_raw(morphine_state_t, void *value);

MORPHINE_API morphine_integer_t mapi_get_integer(morphine_state_t);
MORPHINE_API size_t mapi_get_size(morphine_state_t);
MORPHINE_API morphine_decimal_t mapi_get_decimal(morphine_state_t);
MORPHINE_API bool mapi_get_boolean(morphine_state_t);
MORPHINE_API void *mapi_get_raw(morphine_state_t);

MORPHINE_API void mapi_to_integer(morphine_state_t);
MORPHINE_API void mapi_to_based_integer(morphine_state_t, uint8_t base);
MORPHINE_API void mapi_to_decimal(morphine_state_t);
MORPHINE_API void mapi_to_boolean(morphine_state_t);
MORPHINE_API void mapi_to_string(morphine_state_t);
MORPHINE_API bool mapi_is_callable(morphine_state_t);

MORPHINE_API const char *mapi_type(morphine_state_t);
MORPHINE_API bool mapi_checktype(morphine_state_t, const char *name);

MORPHINE_API void mapi_push_string(morphine_state_t, const char *str);
MORPHINE_API void mapi_push_stringn(morphine_state_t, const char *str, size_t size);
MORPHINE_API void mapi_push_stringf(morphine_state_t, const char *str, ...);
MORPHINE_API void mapi_push_stringv(morphine_state_t, const char *str, va_list args);
MORPHINE_API const char *mapi_get_string(morphine_state_t);
MORPHINE_API size_t mapi_string_len(morphine_state_t);
MORPHINE_API void mapi_string_concat(morphine_state_t);

MORPHINE_API void mapi_push_table(morphine_state_t);
MORPHINE_API void mapi_table_set(morphine_state_t);
MORPHINE_API bool mapi_table_get(morphine_state_t);
MORPHINE_API void mapi_table_getoe(morphine_state_t);
MORPHINE_API size_t mapi_table_len(morphine_state_t);

MORPHINE_API void mapi_push_ref(morphine_state_t);
MORPHINE_API void mapi_ref_set(morphine_state_t);
MORPHINE_API void mapi_ref_get(morphine_state_t);

MORPHINE_API void mapi_push_native(morphine_state_t, const char *name, morphine_native_t native);
MORPHINE_API const char *mapi_native_name(morphine_state_t);
MORPHINE_API morphine_native_t mapi_native_function(morphine_state_t);

MORPHINE_API void mapi_push_userdata(
    morphine_state_t,
    const char *type,
    void *pointer,
    morphine_userdata_mark_t mark,
    morphine_userdata_free_t free
);
MORPHINE_API const char *mapi_userdata_type(morphine_state_t);
MORPHINE_API void *mapi_userdata_pointer(morphine_state_t);
MORPHINE_API void mapi_userdata_link(morphine_state_t, bool soft);
MORPHINE_API bool mapi_userdata_unlink(morphine_state_t, void *pointer);

MORPHINE_API morphine_state_t mapi_push_state(morphine_state_t);
MORPHINE_API void mapi_push_current_state(morphine_state_t);
MORPHINE_API morphine_state_t mapi_get_state(morphine_state_t);
MORPHINE_API void mapi_attach(morphine_state_t);
MORPHINE_API void mapi_state_suspend(morphine_state_t);
MORPHINE_API void mapi_state_kill(morphine_state_t);
MORPHINE_API void mapi_state_resume(morphine_state_t);
MORPHINE_API void mapi_state_priority(morphine_state_t, priority_t);
MORPHINE_API const char *mapi_state_status(morphine_state_t);
MORPHINE_API bool mapi_state_checkstatus(morphine_state_t, const char *name);
MORPHINE_API bool mapi_state_isalive(morphine_state_t);

MORPHINE_API bool mapi_op(morphine_state_t, const char *op);

MORPHINE_API void *mapi_allocator_uni(morphine_instance_t, void *, size_t size);
MORPHINE_API void mapi_allocator_free(morphine_instance_t, void *);

MORPHINE_API void mapi_gc_full(morphine_instance_t);
MORPHINE_API void mapi_gc_work(morphine_instance_t);
MORPHINE_API void mapi_gc_force(morphine_instance_t);
MORPHINE_API bool mapi_gc_isrunning(morphine_instance_t);
MORPHINE_API void mapi_gc_enable(morphine_instance_t);
MORPHINE_API void mapi_gc_disable(morphine_instance_t);
MORPHINE_API bool mapi_gc_isenabled(morphine_instance_t);
MORPHINE_API const char *mapi_gc_status(morphine_instance_t);
MORPHINE_API size_t mapi_gc_allocated(morphine_instance_t);
MORPHINE_API size_t mapi_gc_max_allocated(morphine_instance_t);
MORPHINE_API void mapi_gc_change_start(morphine_instance_t, size_t value);
MORPHINE_API void mapi_gc_change_grow(morphine_instance_t, size_t value);
MORPHINE_API void mapi_gc_change_deal(morphine_instance_t, size_t value);
MORPHINE_API void mapi_gc_change_finalizer_stack_limit(morphine_instance_t I, size_t value);
MORPHINE_API void mapi_gc_change_finalizer_stack_grow(morphine_instance_t I, size_t value);
MORPHINE_API void mapi_gc_change_stack_limit(morphine_state_t, size_t value);
MORPHINE_API void mapi_gc_change_stack_grow(morphine_state_t, size_t value);

MORPHINE_API bool mapi_metatable_test(morphine_state_t, const char *field);
MORPHINE_API void mapi_set_metatable(morphine_state_t);
MORPHINE_API void mapi_get_metatable(morphine_state_t);
MORPHINE_API void mapi_set_default_metatable(morphine_state_t, const char *type);
MORPHINE_API void mapi_get_default_metatable(morphine_state_t, const char *type);

MORPHINE_API void mapi_rotate(morphine_state_t);
MORPHINE_API void mapi_pop(morphine_state_t, size_t size);
MORPHINE_API void mapi_peek(morphine_state_t, size_t offset);
MORPHINE_API void mapi_move(morphine_state_t, morphine_state_t to);
MORPHINE_API void mapi_copy(morphine_state_t, morphine_state_t to, size_t offset);
MORPHINE_API size_t mapi_stack_size(morphine_state_t);
MORPHINE_API void mapi_stack_reset(morphine_state_t);

MORPHINE_API void mapi_registry_set_key(morphine_state_t);
MORPHINE_API bool mapi_registry_get(morphine_state_t);
MORPHINE_API void mapi_registry_getoe(morphine_state_t);
MORPHINE_API void mapi_registry_set(morphine_state_t);
MORPHINE_API void mapi_registry_clear(morphine_state_t);
