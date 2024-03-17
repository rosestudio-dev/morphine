//
// Created by whyiskra on 31.01.24.
//

#include "morphine/core/hook.h"

#ifdef MORPHINE_ENABLE_DEBUGGER

#include "morphine/core/instance.h"
#include "morphine/utils/unused.h"

#define nonnull(x) if((x) != NULL) (x)

void pdbg_private__hook_gc_step_enter(morphine_instance_t I) {
    nonnull(I->platform.debugger.hooks.gc_step_enter)(I);
}

void pdbg_private__hook_gc_step_middle(morphine_instance_t I) {
    nonnull(I->platform.debugger.hooks.gc_step_middle)(I);
}

void pdbg_private__hook_gc_step_exit(morphine_instance_t I) {
    nonnull(I->platform.debugger.hooks.gc_step_exit)(I);
}

void pdbg_private__hook_gc_full_enter(morphine_instance_t I) {
    nonnull(I->platform.debugger.hooks.gc_full_enter)(I);
}

void pdbg_private__hook_gc_full_exit(morphine_instance_t I) {
    nonnull(I->platform.debugger.hooks.gc_full_exit)(I);
}

void pdbg_private__hook_call(morphine_instance_t I, morphine_state_t S) {
    morphinem_unused(I);
    morphinem_unused(S);
}

void pdbg_private__hook_interpreter_step(morphine_instance_t I, morphine_state_t S) {
    nonnull(I->platform.debugger.hooks.interpreter_step)(I, S);
}

void pdbg_private__hook_error(morphine_instance_t I, morphine_state_t S) {
    morphinem_unused(I);
    morphinem_unused(S);
}

#endif
