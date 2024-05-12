package morphine.bytecode

enum class Opcode {
    YIELD,           //                                                               yield
    LOAD,            // [src (const), dest (slot)]                                    get from (src) and set to (dest)
    MOVE,            // [src (slot), dest (slot)]                                     get from (src) and set to (dest)
    PARAM,           // [src (slot), dest (param)]                                    get from (src) and set to (param)
    ARG,             // [src (arg), dest (slot)]                                      get from (src) and set to (dest)

    ENV,             // [dest (slot)]                                                 move env to (dest)
    SELF,            // [dest (slot)]                                                 move self to (dest)
    RECURSION,       // [dest (slot)]                                                 move callable to (dest)

    VECTOR,          // [dest (slot), size (num)]                                     create vector in (dest) with (size)
    TABLE,           // [dest (slot)]                                                 create table in (dest)
    GET,             // [container (slot), key (slot), dest (slot)]                   get from (container) by (key) to (dest)
    SET,             // [container (slot), key (slot), src (slot)]                    set (src) to (container) by (key)

    ITERATOR,        // [container (slot), dest (slot)]                               create iterator from (container) to (dest)
    ITERATOR_INIT,   // [iterator (slot)]                                             init (iterator)
    ITERATOR_HAS,    // [iterator (slot), dest (slot)]                                check next value of (iterator) to (dest)
    ITERATOR_NEXT,   // [iterator (slot), dest (slot)]                                get next value of (iterator) to (dest)

    JUMP,            // [position (pos)]                                              jump to (position)
    JUMP_IF,         // [condition (slot), if_position (pos), else_position (pos)]    if (condition) is true jump to (if_position) else jump to (else_position)

    GET_STATIC,      // [callable (slot), static (index), dest (slot)]                get static by (index) from (callable) to (dest)
    SET_STATIC,      // [callable (slot), static (index), src (slot)]                 set (src) to static of (callable) by (index)

    GET_CLOSURE,     // [closure (slot), closure (index), dest (slot)]                get closure by (index) from (closure) to (dest)
    SET_CLOSURE,     // [closure (slot), closure (index), src (slot)]                 set (src) to (closure) by (index)

    CLOSURE,         // [function (slot), params (num), dest (slot)]                  create closure for (function) with params (size) in (dest)
    CALL,            // [function (slot), params (num)]                               call (function) with params (size) and self as nil
    SCALL,           // [function (slot), params (num), self (slot)]                  call (function) with params (size) and (self)
    LEAVE,           // [result (slot)]                                               leave with (result)
    RESULT,          // [dest (slot)]                                                 set result value to (dest)

    ADD,             // [lvalue (slot), rvalue (slot), dest (slot)]                   (lvalue) operator (rvalue) to (dest)
    SUB,
    MUL,
    DIV,
    MOD,
    EQUAL,
    LESS,
    LESS_EQUAL,
    AND,
    OR,
    CONCAT,

    TYPE,            // [value (slot), dest (slot)]                                   get string type of (value) to (dest)
    NEGATIVE,        // [value (slot), dest (slot)]                                   operator (lvalue) to (dest)
    NOT,
    REF,
    DEREF,
    LENGTH,
}