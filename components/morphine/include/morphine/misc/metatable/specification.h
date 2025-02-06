//
// Created by why-iskra on 31.10.2024.
//

// operations
mspec_metatable_field(TYPE, type)            // (callable | value)
mspec_metatable_field(CALL, call)            // (callable)
mspec_metatable_field(GET, get)              // (callable | value)
mspec_metatable_field(SET, set)              // (callable)
mspec_metatable_field(ADD, add)              // (callable | value)
mspec_metatable_field(SUB, sub)              // (callable | value)
mspec_metatable_field(MUL, mul)              // (callable | value)
mspec_metatable_field(DIV, div)              // (callable | value)
mspec_metatable_field(MOD, mod)              // (callable | value)
mspec_metatable_field(EQUAL, equal)          // (callable | value)
mspec_metatable_field(LESS, less)            // (callable | value)
mspec_metatable_field(OR, or)                // (callable | value)
mspec_metatable_field(AND, and)              // (callable | value)
mspec_metatable_field(CONCAT, concat)        // (callable | value)
mspec_metatable_field(NEGATE, negate)        // (callable | value)
mspec_metatable_field(NOT, not)              // (callable | value)
mspec_metatable_field(LENGTH, length)        // (callable | value)
mspec_metatable_field(REF, ref)              // (callable | value)
mspec_metatable_field(DEREF, deref)          // (callable | value)
mspec_metatable_field(ITERATOR, iter)        // (callable | value)
mspec_metatable_field(ITERATOR_INIT, itinit) // (callable)
mspec_metatable_field(ITERATOR_HAS, ithas)   // (callable | value)
mspec_metatable_field(ITERATOR_NEXT, itnext) // (callable | value)

// table
mspec_metatable_field(TO_STRING, tostr)      // (callable | value)
mspec_metatable_field(HASH, hash)            // (callable | value)
mspec_metatable_field(COMPARE, compare)      // (callable | value)

// exception
mspec_metatable_field(MESSAGE, message)      // (value)

// control
mspec_metatable_field(MASK, mask)            // (value)
mspec_metatable_field(LOCK, lock)            // (value)
mspec_metatable_field(GC, gc)                // (callable)
