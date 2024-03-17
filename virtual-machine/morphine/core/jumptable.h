//
// Created by whyiskra on 27.12.23.
//

#undef sp_dispatch
#undef sp_case
#undef sp_continue

#define sp_dispatch(x) goto *sp_dispatch_tab[x];
#define sp_case(l) OL_##l:
#define sp_continue() morphinem_blk_start sp_fetch(); sp_dispatch(instruction.opcode) morphinem_blk_end

static const void *const sp_dispatch_tab[OPCODES_COUNT] = {
    // generated
    &&OL_OPCODE_YIELD,
    &&OL_OPCODE_LOAD,
    &&OL_OPCODE_MOVE,
    &&OL_OPCODE_PARAM,
    &&OL_OPCODE_ARG,
    &&OL_OPCODE_ENV,
    &&OL_OPCODE_SELF,
    &&OL_OPCODE_RECURSION,
    &&OL_OPCODE_TABLE,
    &&OL_OPCODE_GET,
    &&OL_OPCODE_SET,
    &&OL_OPCODE_JUMP,
    &&OL_OPCODE_JUMP_IF,
    &&OL_OPCODE_GET_STATIC,
    &&OL_OPCODE_SET_STATIC,
    &&OL_OPCODE_GET_CLOSURE,
    &&OL_OPCODE_SET_CLOSURE,
    &&OL_OPCODE_CLOSURE,
    &&OL_OPCODE_CALL,
    &&OL_OPCODE_SCALL,
    &&OL_OPCODE_LEAVE,
    &&OL_OPCODE_RESULT,
    &&OL_OPCODE_ADD,
    &&OL_OPCODE_SUB,
    &&OL_OPCODE_MUL,
    &&OL_OPCODE_DIV,
    &&OL_OPCODE_MOD,
    &&OL_OPCODE_EQUAL,
    &&OL_OPCODE_LESS,
    &&OL_OPCODE_LESS_EQUAL,
    &&OL_OPCODE_AND,
    &&OL_OPCODE_OR,
    &&OL_OPCODE_CONCAT,
    &&OL_OPCODE_TYPE,
    &&OL_OPCODE_NEGATIVE,
    &&OL_OPCODE_NOT,
    &&OL_OPCODE_REF,
    &&OL_OPCODE_DEREF,
    &&OL_OPCODE_LENGTH
};
