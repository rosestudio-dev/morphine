//
// Created by why-iskra on 30.09.2024.
//

#pragma once

#include "morphine/core/value.h"

struct stacktrace_parsed_element {
    const char *type;
    struct string *name;
    ml_line line;
    ml_callstate state;
};

struct stacktrace_element {
    struct value callable;
    struct {
        size_t position;
        ml_callstate state;
    } pc;
};

struct exception {
    struct object header;
    struct value value;

    struct {
        bool recorded;
        bool printable;
        ml_size size;
        struct string *name;
        struct stacktrace_element *elements;
    } stacktrace;
};

struct exception *exceptionI_create(morphine_instance_t, struct value);
void exceptionI_free(morphine_instance_t, struct exception *);

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