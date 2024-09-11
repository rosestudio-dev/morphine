//
// Created by why-iskra on 18.08.2024.
//

mis_instruction_args0(YIELD,         yield)                              //                          yield
mis_instruction_args2(LOAD,          load,    constant_index, slot)      // [src, dest]              get from constant by (src) and set to (dest)
mis_instruction_args2(MOVE,          move,    slot, slot)                // [src, dest]              get from (src) and set to (dest)
mis_instruction_args2(PARAM,         param,   slot, param_index)         // [src, dest]              get from (src) and set to params by (dest)
mis_instruction_args2(ARG,           arg,     argument_index, slot)      // [arg, dest]              get from args by (arg) and set to (dest)

mis_instruction_args1(ENV,           env,     slot)                      // [dest]                   move env to (dest)
mis_instruction_args1(SELF,          self,    slot)                      // [dest]                   move self to (dest)
mis_instruction_args1(INVOKED,       invoked, slot)                      // [dest]                   move invoked callable to (dest)

mis_instruction_args2(VECTOR,        vector,  slot, size)                // [dest, size]             create vector in (dest) with (size)
mis_instruction_args1(TABLE,         table,   slot)                      // [dest]                   create table in (dest)
mis_instruction_args3(GET,           get,     slot, slot, slot)          // [container, key, dest]   get from (container) by (key) to (dest)
mis_instruction_args3(SET,           set,     slot, slot, slot)          // [container, key, src]    set (src) to (container) by (key)

mis_instruction_args2(ITERATOR,      iter,    slot, slot)                // [container, dest]        create iterator from (container) to (dest)
mis_instruction_args3(ITERATOR_INIT, itinit,  slot, slot, slot)          // [iterator, key, value]   init (iterator) with name (key) for key and (value) for value
mis_instruction_args2(ITERATOR_HAS,  ithas,   slot, slot)                // [iterator, dest]         check next value of (iterator) to (dest)
mis_instruction_args2(ITERATOR_NEXT, itnext,  slot, slot)                // [iterator, dest]         get next value of (iterator) to (dest)

mis_instruction_args1(JUMP,          jmp,     position)                  // [position]               jump to (position)
mis_instruction_args3(JUMP_IF,       jmpif,   slot, position, position)  // [condition, if, else]    if (condition) is true jump to (if) else jump to (else)

mis_instruction_args3(GET_STATIC,    getstc,  slot, static_index, slot)  // [callable, static, dest] get static by (index) from (callable) to (dest)
mis_instruction_args3(SET_STATIC,    setstc,  slot, static_index, slot)  // [callable, static, src]  set (src) to static of (callable) by (index)
mis_instruction_args3(GET_CLOSURE,   getcls,  slot, closure_index, slot) // [closure, closure, dest] get closure by (index) from (closure) to (dest)
mis_instruction_args3(SET_CLOSURE,   setcls,  slot, closure_index, slot) // [closure, closure, src]  set (src) to (closure) by (index)

mis_instruction_args3(CLOSURE,       closure, slot, size, slot)          // [function, size, dest]   create closure for (function) with (size) in (dest)
mis_instruction_args2(CALL,          call,    slot, params_count)        // [function, params]       call (function) with count (params)
mis_instruction_args3(SCALL,         scall,   slot, params_count, slot)  // [function, params, self] call (function) with count (params) change (self)
mis_instruction_args1(LEAVE,         leave,   slot)                      // [return]                 leave with (return)
mis_instruction_args1(RESULT,        result,  slot)                      // [dest]                   set result value to (dest)

mis_instruction_args3(ADD,           add,     slot, slot, slot)          // [lvalue, rvalue, dest]   (lvalue) operator (rvalue) to (dest)
mis_instruction_args3(SUB,           sub,     slot, slot, slot)
mis_instruction_args3(MUL,           mul,     slot, slot, slot)
mis_instruction_args3(DIV,           div,     slot, slot, slot)
mis_instruction_args3(MOD,           mod,     slot, slot, slot)
mis_instruction_args3(EQUAL,         eq,      slot, slot, slot)
mis_instruction_args3(LESS,          less,    slot, slot, slot)
mis_instruction_args3(AND,           and,     slot, slot, slot)
mis_instruction_args3(OR,            or,      slot, slot, slot)
mis_instruction_args3(CONCAT,        concat,  slot, slot, slot)

mis_instruction_args2(TYPE,          type,    slot, slot)                // [value, dest]            get string type of (value) to (dest)
mis_instruction_args2(NEGATIVE,      neg,     slot, slot)                // [value, dest]            operator (lvalue) to (dest)
mis_instruction_args2(NOT,           not,     slot, slot)
mis_instruction_args2(REF,           ref,     slot, slot)
mis_instruction_args2(DEREF,         deref,   slot, slot)
mis_instruction_args2(LENGTH,        len,     slot, slot)
