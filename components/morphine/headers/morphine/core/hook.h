//
// Created by whyiskra on 31.01.24.
//

#pragma once

#include "morphine/config/components.h"

#ifdef MORPHINE_ENABLE_DEBUGGER

#include "morphine/platform.h"

#define pdbg_hook_gc_step_enter(I)       pdbg_private__hook_gc_step_enter((I))
#define pdbg_hook_gc_step_middle(I)      pdbg_private__hook_gc_step_middle((I))
#define pdbg_hook_gc_step_exit(I)        pdbg_private__hook_gc_step_exit((I))

#define pdbg_hook_gc_full_enter(I)       pdbg_private__hook_gc_full_enter((I))
#define pdbg_hook_gc_full_exit(I)        pdbg_private__hook_gc_full_exit((I))

#define pdbg_hook_call(I, S)             pdbg_private__hook_call((I), (S))
#define pdbg_hook_interpreter_step(I, S) pdbg_private__hook_interpreter_step((I), (S))
#define pdbg_hook_error(I, S)            pdbg_private__hook_error((I), (S))

void pdbg_private__hook_gc_step_enter(morphine_instance_t);
void pdbg_private__hook_gc_step_middle(morphine_instance_t);
void pdbg_private__hook_gc_step_exit(morphine_instance_t);
void pdbg_private__hook_gc_full_enter(morphine_instance_t);
void pdbg_private__hook_gc_full_exit(morphine_instance_t);
void pdbg_private__hook_call(morphine_instance_t, morphine_state_t);
void pdbg_private__hook_interpreter_step(morphine_instance_t, morphine_state_t);
void pdbg_private__hook_error(morphine_instance_t, morphine_state_t);

#else

#include "morphine/utils/semicolon.h"

#define pdbg_hook_gc_step_enter(I) morphinem_blk_start morphinem_blk_end
#define pdbg_hook_gc_step_middle(I) morphinem_blk_start morphinem_blk_end
#define pdbg_hook_gc_step_exit(I) morphinem_blk_start morphinem_blk_end

#define pdbg_hook_gc_full_enter(I) morphinem_blk_start morphinem_blk_end
#define pdbg_hook_gc_full_exit(I) morphinem_blk_start morphinem_blk_end

#define pdbg_hook_call(I, S) morphinem_blk_start morphinem_blk_end
#define pdbg_hook_interpreter_step(I, S) morphinem_blk_start morphinem_blk_end
#define pdbg_hook_error(I, S) morphinem_blk_start morphinem_blk_end

#endif