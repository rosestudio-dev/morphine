//
// Created by why-iskra on 30.09.2024.
//

#pragma once

#include "morphine/core/value.h"

typedef enum {
    EXCEPTION_KIND_USER,
    EXCEPTION_KIND_OFM,
    EXCEPTION_KIND_AF,
    EXCEPTION_KIND_UNDEF,
} exception_kind_t;

struct stacktrace_parsed_element {
    const char *type;
    struct string *name;
    ml_line line;
    ml_size state;
};

struct stacktrace_element {
    struct value callable;
    struct {
        ml_size position;
        ml_size state;
    } pc;
};

struct exception {
    struct object header;

    exception_kind_t kind;
    struct value value;

    struct {
        bool recorded;
        bool printable;
        ml_size size;
        struct string *name;
        struct stacktrace_element *elements;
    } stacktrace;
};

struct exception *exceptionI_create(morphine_instance_t, struct value, exception_kind_t);
void exceptionI_free(morphine_instance_t, struct exception *);

const char *exceptionI_kind2str(morphine_instance_t, exception_kind_t);

struct string *exceptionI_message(morphine_instance_t, struct exception *);
void exceptionI_error_print(morphine_instance_t, struct exception *, struct stream *);
void exceptionI_stacktrace_print(morphine_instance_t, struct exception *, struct stream *, ml_size);
void exceptionI_stacktrace_record(morphine_instance_t, struct exception *, morphine_coroutine_t);
void exceptionI_stacktrace_stub(morphine_instance_t, struct exception *);

struct stacktrace_parsed_element exceptionI_stacktrace_element(
    morphine_instance_t,
    struct exception *,
    ml_size
);