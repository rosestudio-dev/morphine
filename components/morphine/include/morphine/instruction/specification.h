//
// Created by why-iskra on 18.08.2024.
//

mis_instruction_args0(YIELD)                                  //                          yield
mis_instruction_args2(LOAD, constant_index, slot)             // [src, dest]              get from constant by (src) and set to (dest)
mis_instruction_args2(MOVE, slot, slot)                       // [src, dest]              get from (src) and set to (dest)
mis_instruction_args2(PARAM, slot, param_index)               // [src, dest]              get from (src) and set to params by (dest)
mis_instruction_args2(ARG, argument_index, slot)              // [arg, dest]              get from args by (arg) and set to (dest)

mis_instruction_args1(ENV, slot)                              // [dest]                   move env to (dest)
mis_instruction_args1(SELF, slot)                             // [dest]                   move self to (dest)
mis_instruction_args1(INVOKED, slot)                          // [dest]                   move invoked callable to (dest)

mis_instruction_args2(VECTOR, slot, size)                     // [dest, size]             create vector in (dest) with (size)
mis_instruction_args1(TABLE, slot)                            // [dest]                   create table in (dest)
mis_instruction_args3(GET, slot, slot, slot)                  // [container, key, dest]   get from (container) by (key) to (dest)
mis_instruction_args3(SET, slot, slot, slot)                  // [container, key, src]    set (src) to (container) by (key)

mis_instruction_args2(ITERATOR, slot, slot)                   // [container, dest]        create iterator from (container) to (dest)
mis_instruction_args3(ITERATOR_INIT, slot, slot, slot)        // [iterator, key, value]   init (iterator) with name (key) for key and (value) for value
mis_instruction_args2(ITERATOR_HAS, slot, slot)               // [iterator, dest]         check next value of (iterator) to (dest)
mis_instruction_args2(ITERATOR_NEXT, slot, slot)              // [iterator, dest]         get next value of (iterator) to (dest)

mis_instruction_args1(JUMP, position)                         // [position]               jump to (position)
mis_instruction_args3(JUMP_IF, slot, position, position)      // [condition, if, else]    if (condition) is true jump to (if) else jump to (else)

mis_instruction_args3(GET_STATIC, slot, static_index, slot)   // [callable, static, dest] get static by (index) from (callable) to (dest)
mis_instruction_args3(SET_STATIC, slot, static_index, slot)   // [callable, static, src]  set (src) to static of (callable) by (index)
mis_instruction_args3(GET_CLOSURE, slot, closure_index, slot) // [closure, closure, dest] get closure by (index) from (closure) to (dest)
mis_instruction_args3(SET_CLOSURE, slot, closure_index, slot) // [closure, closure, src]  set (src) to (closure) by (index)

mis_instruction_args3(CLOSURE, slot, size, slot)              // [function, size, dest]   create closure for (function) with (size) in (dest)
mis_instruction_args2(CALL, slot, params_count)               // [function, params]       call (function) with count (params)
mis_instruction_args3(SCALL, slot, params_count, slot)        // [function, params, self] call (function) with count (params) change (self)
mis_instruction_args1(LEAVE, slot)                            // [return]                 leave with (return)
mis_instruction_args1(RESULT, slot)                           // [dest]                   set result value to (dest)

mis_instruction_args3(ADD, slot, slot, slot)                  // [lvalue, rvalue, dest]   (lvalue) operator (rvalue) to (dest)
mis_instruction_args3(SUB, slot, slot, slot)
mis_instruction_args3(MUL, slot, slot, slot)
mis_instruction_args3(DIV, slot, slot, slot)
mis_instruction_args3(MOD, slot, slot, slot)
mis_instruction_args3(EQUAL, slot, slot, slot)
mis_instruction_args3(LESS, slot, slot, slot)
mis_instruction_args3(AND, slot, slot, slot)
mis_instruction_args3(OR, slot, slot, slot)
mis_instruction_args3(CONCAT, slot, slot, slot)

mis_instruction_args2(TYPE, slot, slot)                       // [value, dest]            get string type of (value) to (dest)
mis_instruction_args2(NEGATIVE, slot, slot)                   // [value, dest]            operator (lvalue) to (dest)
mis_instruction_args2(NOT, slot, slot)
mis_instruction_args2(REF, slot, slot)
mis_instruction_args2(DEREF, slot, slot)
mis_instruction_args2(LENGTH, slot, slot)
