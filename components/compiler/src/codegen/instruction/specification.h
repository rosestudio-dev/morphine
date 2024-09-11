//
// Created by why-iskra on 18.08.2024.
//

instruction_args0(YIELD)
instruction_args2(LOAD, constant_index, slot)
instruction_args2(MOVE, slot, slot)
instruction_args2(PARAM, slot, param_index)
instruction_args2(ARG, argument_index, slot)

instruction_args1(ENV, slot)
instruction_args1(SELF, slot)
instruction_args1(INVOKED, slot)

instruction_args2(VECTOR, slot, count)
instruction_args1(TABLE, slot)
instruction_args3(GET, slot, slot, slot)
instruction_args3(SET, slot, slot, slot)

instruction_args2(ITERATOR, slot, slot)
instruction_args3(ITERATOR_INIT, slot, slot, slot)
instruction_args2(ITERATOR_HAS, slot, slot)
instruction_args2(ITERATOR_NEXT, slot, slot)

instruction_args1(JUMP, anchor)
instruction_args3(JUMP_IF, slot, anchor, anchor)

instruction_args3(GET_STATIC, slot, static_index, slot)
instruction_args3(SET_STATIC, slot, static_index, slot)
instruction_args3(GET_CLOSURE, slot, closure_index, slot)
instruction_args3(SET_CLOSURE, slot, closure_index, slot)

instruction_args3(CLOSURE, slot, count, slot)
instruction_args2(CALL, slot, params_count)
instruction_args3(SCALL, slot, params_count, slot)
instruction_args1(LEAVE, slot)
instruction_args1(RESULT, slot)

instruction_args3(ADD, slot, slot, slot)
instruction_args3(SUB, slot, slot, slot)
instruction_args3(MUL, slot, slot, slot)
instruction_args3(DIV, slot, slot, slot)
instruction_args3(MOD, slot, slot, slot)
instruction_args3(EQUAL, slot, slot, slot)
instruction_args3(LESS, slot, slot, slot)
instruction_args3(AND, slot, slot, slot)
instruction_args3(OR, slot, slot, slot)
instruction_args3(CONCAT, slot, slot, slot)

instruction_args2(TYPE, slot, slot)
instruction_args2(NEGATIVE, slot, slot)
instruction_args2(NOT, slot, slot)
instruction_args2(REF, slot, slot)
instruction_args2(DEREF, slot, slot)
instruction_args2(LENGTH, slot, slot)
