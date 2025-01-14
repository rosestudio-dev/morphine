//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include <stdarg.h>
#include "config.h"
#include "platform.h"
#include "misc/instruction/type.h"
#include "misc/metatable/type.h"
#include "core/type/constants.h"

MORPHINE_API const char *mapi_version_name(void);
MORPHINE_API ml_version mapi_version(void);
MORPHINE_API ml_version mapi_bytecode_version(void);

MORPHINE_API morphine_instance_t mapi_open(morphine_platform_t, morphine_settings_t, void *data);
MORPHINE_API void *mapi_instance_data(morphine_instance_t);
MORPHINE_API void mapi_interpreter(morphine_instance_t);
MORPHINE_API bool mapi_interpreter_step(morphine_instance_t);
MORPHINE_API void mapi_close(morphine_instance_t);

MORPHINE_API morphine_instance_t mapi_instance(morphine_coroutine_t);
MORPHINE_API morphine_coroutine_t mapi_coroutine(morphine_instance_t);

// library

MORPHINE_API void mapi_library_load(morphine_instance_t, morphine_library_t);
MORPHINE_API void mapi_library(morphine_coroutine_t, const char *);

// callstack

MORPHINE_API void mapi_call(morphine_coroutine_t, ml_size argc);
MORPHINE_API void mapi_ecall(morphine_coroutine_t, ml_size argc);
MORPHINE_API void mapi_scall(morphine_coroutine_t, ml_size argc);
MORPHINE_API void mapi_escall(morphine_coroutine_t, ml_size argc);
MORPHINE_API void mapi_push_callable(morphine_coroutine_t);
MORPHINE_API void mapi_push_recursion(morphine_coroutine_t);
MORPHINE_API void mapi_extract_callable(morphine_coroutine_t);
MORPHINE_API void mapi_push_result(morphine_coroutine_t);
MORPHINE_API void mapi_result(morphine_coroutine_t);
MORPHINE_API void mapi_return(morphine_coroutine_t);
MORPHINE_API void mapi_leave(morphine_coroutine_t);
MORPHINE_API void mapi_continue(morphine_coroutine_t, size_t callstate);
MORPHINE_API size_t mapi_callstate(morphine_coroutine_t);
MORPHINE_API ml_size mapi_args(morphine_coroutine_t);
MORPHINE_API void mapi_push_arg(morphine_coroutine_t, ml_size index);
MORPHINE_API void mapi_push_env(morphine_coroutine_t);
MORPHINE_API void mapi_push_self(morphine_coroutine_t);

// error

MORPHINE_API morphine_noret void mapi_error(morphine_coroutine_t, const char *);
MORPHINE_API morphine_noret void mapi_ierror(morphine_instance_t, const char *);
MORPHINE_API morphine_noret morphine_printf(2, 3) void mapi_errorf(morphine_coroutine_t, const char *, ...);
MORPHINE_API morphine_noret morphine_printf(2, 3) void mapi_ierrorf(morphine_instance_t, const char *, ...);
MORPHINE_API morphine_noret void mapi_provide_error(morphine_coroutine_t);
MORPHINE_API void mapi_catchable(morphine_coroutine_t, size_t callstate);
MORPHINE_API void mapi_crashable(morphine_coroutine_t);
MORPHINE_API void mapi_uncatch(morphine_coroutine_t);
MORPHINE_API void mapi_protect(morphine_instance_t, morphine_try_t, morphine_catch_t, void *, bool catch_provide);
MORPHINE_API void mapi_exception(morphine_coroutine_t);
MORPHINE_API const char *mapi_signal_message(morphine_instance_t);
MORPHINE_API bool mapi_is_nested_signal(morphine_instance_t);

// values

MORPHINE_API void mapi_push_nil(morphine_coroutine_t);
MORPHINE_API void mapi_push_integer(morphine_coroutine_t, ml_integer value);
MORPHINE_API void mapi_push_size(morphine_coroutine_t, size_t value, const char *name);
MORPHINE_API void mapi_push_decimal(morphine_coroutine_t, ml_decimal value);
MORPHINE_API void mapi_push_boolean(morphine_coroutine_t, bool value);
MORPHINE_API void mapi_push_raw(morphine_coroutine_t, void *value);
MORPHINE_API void mapi_push_hash(morphine_coroutine_t, ml_hash);

MORPHINE_API ml_integer mapi_get_integer(morphine_coroutine_t);
MORPHINE_API ml_size mapi_get_size(morphine_coroutine_t, const char *name);
MORPHINE_API ml_decimal mapi_get_decimal(morphine_coroutine_t);
MORPHINE_API bool mapi_get_boolean(morphine_coroutine_t);
MORPHINE_API uintptr_t mapi_get_raw(morphine_coroutine_t);
MORPHINE_API ml_hash mapi_get_hash(morphine_coroutine_t);

MORPHINE_API void mapi_to_integer(morphine_coroutine_t);
MORPHINE_API void mapi_to_size(morphine_coroutine_t, const char *name);
MORPHINE_API void mapi_to_based_integer(morphine_coroutine_t, ml_size base);
MORPHINE_API void mapi_to_based_size(morphine_coroutine_t, ml_size base, const char *name);
MORPHINE_API void mapi_to_decimal(morphine_coroutine_t);
MORPHINE_API void mapi_to_boolean(morphine_coroutine_t);
MORPHINE_API void mapi_to_string(morphine_coroutine_t);

MORPHINE_API ml_hash mapi_hash(morphine_coroutine_t);
MORPHINE_API int mapi_compare(morphine_coroutine_t);
MORPHINE_API bool mapi_equal(morphine_coroutine_t);

MORPHINE_API ml_size mapi_csize2size(morphine_coroutine_t, size_t value, const char *name);

// type

MORPHINE_API void mapi_type_declare(
    morphine_instance_t,
    const char *name,
    size_t allocate,
    bool require_metatable,
    morphine_userdata_constructor_t,
    morphine_userdata_destructor_t,
    morphine_userdata_compare_t,
    morphine_userdata_hash_t
);
MORPHINE_API bool mapi_type_is_declared(morphine_instance_t, const char *name);
MORPHINE_API const char *mapi_type(morphine_coroutine_t);
MORPHINE_API bool mapi_is(morphine_coroutine_t, const char *type);
MORPHINE_API bool mapi_is_type(morphine_coroutine_t, const char *type);
MORPHINE_API bool mapi_is_callable(morphine_coroutine_t);
MORPHINE_API bool mapi_is_metatype(morphine_coroutine_t);
MORPHINE_API bool mapi_is_iterable(morphine_coroutine_t);
MORPHINE_API bool mapi_is_size(morphine_coroutine_t);

// string

MORPHINE_API void mapi_push_string(morphine_coroutine_t, const char *);
MORPHINE_API void mapi_push_stringn(morphine_coroutine_t, const char *, size_t);
MORPHINE_API morphine_printf(2, 3) void mapi_push_stringf(morphine_coroutine_t, const char *str, ...);
MORPHINE_API void mapi_push_stringv(morphine_coroutine_t, const char *str, va_list);
MORPHINE_API const char *mapi_get_string(morphine_coroutine_t);
MORPHINE_API const char *mapi_get_cstr(morphine_coroutine_t);
MORPHINE_API ml_size mapi_string_len(morphine_coroutine_t);
MORPHINE_API void mapi_string_concat(morphine_coroutine_t);
MORPHINE_API int mapi_string_compare(morphine_coroutine_t);
MORPHINE_API int mapi_string_cstr_compare(morphine_coroutine_t, const char *);
MORPHINE_API bool mapi_string_is_cstr_compatible(morphine_coroutine_t);

// table

MORPHINE_API void mapi_push_table(morphine_coroutine_t);
MORPHINE_API ml_size mapi_table_len(morphine_coroutine_t);
MORPHINE_API void mapi_table_clear(morphine_coroutine_t);
MORPHINE_API void mapi_table_copy(morphine_coroutine_t);
MORPHINE_API void mapi_table_concat(morphine_coroutine_t);

MORPHINE_API void mapi_table_mode_mutable(morphine_coroutine_t, bool);
MORPHINE_API void mapi_table_mode_fixed(morphine_coroutine_t, bool);
MORPHINE_API void mapi_table_mode_accessible(morphine_coroutine_t, bool);
MORPHINE_API void mapi_table_mode_lock_metatable(morphine_coroutine_t);
MORPHINE_API void mapi_table_mode_lock(morphine_coroutine_t);

MORPHINE_API bool mapi_table_mode_is_mutable(morphine_coroutine_t);
MORPHINE_API bool mapi_table_mode_is_fixed(morphine_coroutine_t);
MORPHINE_API bool mapi_table_mode_is_accessible(morphine_coroutine_t);
MORPHINE_API bool mapi_table_mode_metatable_is_locked(morphine_coroutine_t);
MORPHINE_API bool mapi_table_mode_is_locked(morphine_coroutine_t);

MORPHINE_API void mapi_table_set(morphine_coroutine_t);
MORPHINE_API bool mapi_table_has(morphine_coroutine_t);
MORPHINE_API bool mapi_table_get(morphine_coroutine_t);
MORPHINE_API bool mapi_table_remove(morphine_coroutine_t);

MORPHINE_API void mapi_table_idx_set(morphine_coroutine_t, ml_size);
MORPHINE_API bool mapi_table_idx_get(morphine_coroutine_t, ml_size);
MORPHINE_API bool mapi_table_idx_key(morphine_coroutine_t, ml_size);

MORPHINE_API void mapi_table_idx_getoe(morphine_coroutine_t, ml_size);
MORPHINE_API void mapi_table_idx_keyoe(morphine_coroutine_t, ml_size);

MORPHINE_API void mapi_table_getoe(morphine_coroutine_t);
MORPHINE_API void mapi_table_removeoe(morphine_coroutine_t);

// vector

MORPHINE_API void mapi_push_vector(morphine_coroutine_t, ml_size);
MORPHINE_API ml_size mapi_vector_len(morphine_coroutine_t);
MORPHINE_API void mapi_vector_resize(morphine_coroutine_t, ml_size);
MORPHINE_API void mapi_vector_clear(morphine_coroutine_t);
MORPHINE_API void mapi_vector_copy(morphine_coroutine_t);
MORPHINE_API void mapi_vector_sort(morphine_coroutine_t);
MORPHINE_API void mapi_vector_concat(morphine_coroutine_t);

MORPHINE_API void mapi_vector_mode_mutable(morphine_coroutine_t, bool);
MORPHINE_API void mapi_vector_mode_fixed(morphine_coroutine_t, bool);
MORPHINE_API void mapi_vector_mode_accessible(morphine_coroutine_t, bool);
MORPHINE_API void mapi_vector_mode_lock(morphine_coroutine_t);

MORPHINE_API bool mapi_vector_mode_is_mutable(morphine_coroutine_t);
MORPHINE_API bool mapi_vector_mode_is_fixed(morphine_coroutine_t);
MORPHINE_API bool mapi_vector_mode_is_accessible(morphine_coroutine_t);
MORPHINE_API bool mapi_vector_mode_is_locked(morphine_coroutine_t);

MORPHINE_API void mapi_vector_set(morphine_coroutine_t, ml_size index);
MORPHINE_API void mapi_vector_add(morphine_coroutine_t, ml_size index);
MORPHINE_API bool mapi_vector_has(morphine_coroutine_t);
MORPHINE_API void mapi_vector_get(morphine_coroutine_t, ml_size index);
MORPHINE_API void mapi_vector_remove(morphine_coroutine_t, ml_size index);

MORPHINE_API void mapi_vector_push(morphine_coroutine_t);
MORPHINE_API void mapi_vector_pop(morphine_coroutine_t);
MORPHINE_API void mapi_vector_peek(morphine_coroutine_t);

MORPHINE_API void mapi_vector_push_front(morphine_coroutine_t);
MORPHINE_API void mapi_vector_pop_front(morphine_coroutine_t);
MORPHINE_API void mapi_vector_peek_front(morphine_coroutine_t);

// reference

MORPHINE_API void mapi_push_ref(morphine_coroutine_t);
MORPHINE_API void mapi_ref_set(morphine_coroutine_t);
MORPHINE_API void mapi_ref_get(morphine_coroutine_t);

// native

MORPHINE_API void mapi_push_native(morphine_coroutine_t, morphine_native_t native);
MORPHINE_API void mapi_native_name(morphine_coroutine_t);
MORPHINE_API morphine_native_t mapi_native_function(morphine_coroutine_t);

// userdata

MORPHINE_API void *mapi_push_userdata(morphine_coroutine_t, const char *type);
MORPHINE_API void *mapi_push_userdata_uni(morphine_coroutine_t, size_t size);
MORPHINE_API void *mapi_push_userdata_vec(morphine_coroutine_t, size_t count, size_t size);
MORPHINE_API void mapi_userdata_set_destructor(morphine_coroutine_t, morphine_userdata_destructor_t);
MORPHINE_API void mapi_userdata_mode_lock_destructor(morphine_coroutine_t);
MORPHINE_API void mapi_userdata_mode_lock_metatable(morphine_coroutine_t);
MORPHINE_API void mapi_userdata_mode_lock_size(morphine_coroutine_t);
MORPHINE_API bool mapi_userdata_mode_destructor_is_locked(morphine_coroutine_t);
MORPHINE_API bool mapi_userdata_mode_metatable_is_locked(morphine_coroutine_t);
MORPHINE_API bool mapi_userdata_mode_size_is_locked(morphine_coroutine_t);
MORPHINE_API void *mapi_userdata_resize(morphine_coroutine_t, size_t size);
MORPHINE_API void *mapi_userdata_resize_vec(morphine_coroutine_t, size_t count, size_t size);
MORPHINE_API void *mapi_userdata_pointer(morphine_coroutine_t, const char *type);
MORPHINE_API bool mapi_userdata_is_typed(morphine_coroutine_t);

// coroutine

MORPHINE_API morphine_coroutine_t mapi_push_coroutine(morphine_coroutine_t);
MORPHINE_API morphine_coroutine_t mapi_get_coroutine(morphine_coroutine_t);
MORPHINE_API void mapi_current(morphine_coroutine_t);
MORPHINE_API void mapi_attach(morphine_coroutine_t);
MORPHINE_API void mapi_coroutine_suspend(morphine_coroutine_t);
MORPHINE_API void mapi_coroutine_kill(morphine_coroutine_t);
MORPHINE_API void mapi_coroutine_resume(morphine_coroutine_t);
MORPHINE_API void mapi_coroutine_priority(morphine_coroutine_t, ml_size priority);
MORPHINE_API const char *mapi_coroutine_status(morphine_coroutine_t);
MORPHINE_API bool mapi_coroutine_is_alive(morphine_coroutine_t);
MORPHINE_API void mapi_coroutine_name(morphine_coroutine_t);

// iterator

MORPHINE_API void mapi_push_iterator(morphine_coroutine_t);
MORPHINE_API void mapi_iterator_init(morphine_coroutine_t);
MORPHINE_API bool mapi_iterator_has(morphine_coroutine_t);
MORPHINE_API void mapi_iterator_next(morphine_coroutine_t);

// closure

MORPHINE_API void mapi_push_closure(morphine_coroutine_t, ml_size size);
MORPHINE_API void mapi_closure_get(morphine_coroutine_t, ml_size index);
MORPHINE_API void mapi_closure_set(morphine_coroutine_t, ml_size index);
MORPHINE_API ml_size mapi_closure_size(morphine_coroutine_t);

// function

MORPHINE_API void mapi_push_function(
    morphine_coroutine_t,
    ml_line line,
    ml_size constants_count,
    ml_size instructions_count,
    ml_size statics_count,
    ml_size arguments_count,
    ml_size slots_count,
    ml_size params_count
);

MORPHINE_API void mapi_function_copy(morphine_coroutine_t);
MORPHINE_API void mapi_function_complete(morphine_coroutine_t);
MORPHINE_API bool mapi_function_is_complete(morphine_coroutine_t);

MORPHINE_API void mapi_function_name(morphine_coroutine_t);
MORPHINE_API ml_line mapi_function_line(morphine_coroutine_t);
MORPHINE_API ml_size mapi_function_arguments(morphine_coroutine_t);
MORPHINE_API ml_size mapi_function_slots(morphine_coroutine_t);
MORPHINE_API ml_size mapi_function_params(morphine_coroutine_t);

MORPHINE_API void mapi_static_get(morphine_coroutine_t, ml_size index);
MORPHINE_API void mapi_static_set(morphine_coroutine_t, ml_size index);
MORPHINE_API ml_size mapi_static_size(morphine_coroutine_t);

MORPHINE_API void mapi_constant_get(morphine_coroutine_t, ml_size index);
MORPHINE_API void mapi_constant_set(morphine_coroutine_t, ml_size index);
MORPHINE_API ml_size mapi_constant_size(morphine_coroutine_t);

MORPHINE_API morphine_instruction_t mapi_instruction_get(morphine_coroutine_t, ml_size index);
MORPHINE_API void mapi_instruction_set(morphine_coroutine_t, ml_size index, morphine_instruction_t);
MORPHINE_API ml_size mapi_instruction_size(morphine_coroutine_t);

MORPHINE_API ml_size mapi_opcode(morphine_coroutine_t, morphine_opcode_t);

// allocation

MORPHINE_API void *mapi_allocator_uni(morphine_instance_t, void *, size_t size);
MORPHINE_API void *mapi_allocator_vec(morphine_instance_t, void *, size_t n, size_t size);
MORPHINE_API void mapi_allocator_free(morphine_instance_t, void *);

// gc

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
MORPHINE_API void mapi_gc_set_limit(morphine_instance_t, size_t);
MORPHINE_API void mapi_gc_set_threshold(morphine_instance_t, size_t);
MORPHINE_API void mapi_gc_set_grow(morphine_instance_t, uint16_t);
MORPHINE_API void mapi_gc_set_deal(morphine_instance_t, uint16_t);
MORPHINE_API void mapi_gc_set_pause(morphine_instance_t, uint8_t);
MORPHINE_API void mapi_gc_set_cache_callinfo(morphine_instance_t, size_t);

// metatable

MORPHINE_API bool mapi_metatable_test(morphine_coroutine_t, const char *);
MORPHINE_API bool mapi_metatable_builtin_test(morphine_coroutine_t, morphine_metatable_field_t);
MORPHINE_API void mapi_set_metatable(morphine_coroutine_t);
MORPHINE_API void mapi_get_metatable(morphine_coroutine_t);

// stack

MORPHINE_API void mapi_rotate(morphine_coroutine_t, size_t count);
MORPHINE_API void mapi_pop(morphine_coroutine_t, size_t size);
MORPHINE_API void mapi_peek(morphine_coroutine_t, size_t offset);
MORPHINE_API void mapi_move(morphine_coroutine_t, morphine_coroutine_t to);
MORPHINE_API void mapi_copy(morphine_coroutine_t, morphine_coroutine_t to, size_t offset);
MORPHINE_API void mapi_stack_reset(morphine_coroutine_t);
MORPHINE_API size_t mapi_stack_used(morphine_coroutine_t);
MORPHINE_API void mapi_stack_set_limit(morphine_coroutine_t, size_t);

// localstorage

MORPHINE_API void mapi_localstorage_set(morphine_coroutine_t);
MORPHINE_API bool mapi_localstorage_get(morphine_coroutine_t);
MORPHINE_API bool mapi_localstorage_remove(morphine_coroutine_t);
MORPHINE_API void mapi_localstorage_getoe(morphine_coroutine_t);
MORPHINE_API void mapi_localstorage_removeoe(morphine_coroutine_t);
MORPHINE_API void mapi_localstorage_clear(morphine_coroutine_t);

// sharedstorage

MORPHINE_API bool mapi_sharedstorage_get(morphine_coroutine_t, const char *sharedkey);
MORPHINE_API void mapi_sharedstorage_getoe(morphine_coroutine_t, const char *sharedkey);
MORPHINE_API void mapi_sharedstorage_set(morphine_coroutine_t, const char *sharedkey);
MORPHINE_API bool mapi_sharedstorage_remove(morphine_coroutine_t, const char *sharedkey);
MORPHINE_API void mapi_sharedstorage_removeoe(morphine_coroutine_t, const char *sharedkey);
MORPHINE_API void mapi_sharedstorage_clear(morphine_coroutine_t, const char *sharedkey);

// stream

MORPHINE_API void mapi_push_stream_io(morphine_coroutine_t);
MORPHINE_API void mapi_push_stream_err(morphine_coroutine_t);

MORPHINE_API void mapi_push_stream(morphine_coroutine_t, morphine_stream_interface_t, bool hold, void *args);
MORPHINE_API void mapi_stream_close(morphine_coroutine_t, bool force);
MORPHINE_API size_t mapi_stream_read(morphine_coroutine_t, uint8_t *, size_t);
MORPHINE_API size_t mapi_stream_write(morphine_coroutine_t, const uint8_t *, size_t);
MORPHINE_API void mapi_stream_flush(morphine_coroutine_t);
MORPHINE_API bool mapi_stream_seek_set(morphine_coroutine_t, size_t);
MORPHINE_API bool mapi_stream_seek_cur(morphine_coroutine_t, size_t);
MORPHINE_API bool mapi_stream_seek_prv(morphine_coroutine_t, size_t);
MORPHINE_API bool mapi_stream_seek_end(morphine_coroutine_t, size_t);
MORPHINE_API size_t mapi_stream_tell(morphine_coroutine_t);
MORPHINE_API bool mapi_stream_eos(morphine_coroutine_t);

MORPHINE_API size_t mapi_stream_print(morphine_coroutine_t, const char *);
MORPHINE_API morphine_printf(2, 3) size_t mapi_stream_printf(morphine_coroutine_t, const char *, ...);

// exception

MORPHINE_API void mapi_push_exception(morphine_coroutine_t);
MORPHINE_API void mapi_exception_value(morphine_coroutine_t);
MORPHINE_API void mapi_exception_message(morphine_coroutine_t);
MORPHINE_API void mapi_exception_error_print(morphine_coroutine_t);
MORPHINE_API void mapi_exception_stacktrace_print(morphine_coroutine_t, ml_size);
MORPHINE_API void mapi_exception_stacktrace_record(morphine_coroutine_t, morphine_coroutine_t);
MORPHINE_API void mapi_exception_stacktrace_name(morphine_coroutine_t);
MORPHINE_API ml_size mapi_exception_stacktrace_size(morphine_coroutine_t);
MORPHINE_API const char *mapi_exception_stacktrace_type(morphine_coroutine_t, ml_size);
MORPHINE_API void mapi_exception_stacktrace_callable(morphine_coroutine_t, ml_size);
MORPHINE_API ml_line mapi_exception_stacktrace_line(morphine_coroutine_t, ml_size);
MORPHINE_API size_t mapi_exception_stacktrace_state(morphine_coroutine_t, ml_size);

// operations

MORPHINE_API bool mapi_op(morphine_coroutine_t, const char *op);

// packer

MORPHINE_API void mapi_pack(morphine_coroutine_t);
MORPHINE_API void mapi_unpack(morphine_coroutine_t);

// isolate

MORPHINE_API void mapi_isolate(morphine_coroutine_t, morphine_isolate_config_t, ml_size);

// platform

MORPHINE_API bool mapi_platform_str2int(const char *, ml_integer *, ml_size base);
MORPHINE_API bool mapi_platform_str2size(const char *, ml_size *, ml_size base);
MORPHINE_API bool mapi_platform_str2dec(const char *, ml_decimal *);

// misc

MORPHINE_API ml_hash mapi_misc_hash(size_t, const uint8_t *);
