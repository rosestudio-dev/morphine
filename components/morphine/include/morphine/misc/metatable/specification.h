//
// Created by why-iskra on 31.10.2024.
//

// value
mspec_metatable_field(TYPE, _mf_type)              // (callable | value)
mspec_metatable_field(CALL, _mf_call)              // (callable)
mspec_metatable_field(GET, _mf_get)                // (callable | value)
mspec_metatable_field(SET, _mf_set)                // (callable)
mspec_metatable_field(ADD, _mf_add)                // (callable | value)
mspec_metatable_field(SUB, _mf_sub)                // (callable | value)
mspec_metatable_field(MUL, _mf_mul)                // (callable | value)
mspec_metatable_field(DIV, _mf_div)                // (callable | value)
mspec_metatable_field(MOD, _mf_mod)                // (callable | value)
mspec_metatable_field(EQUAL, _mf_equal)            // (callable | value)
mspec_metatable_field(LESS, _mf_less)              // (callable | value)
mspec_metatable_field(OR, _mf_or)                  // (callable | value)
mspec_metatable_field(AND, _mf_and)                // (callable | value)
mspec_metatable_field(CONCAT, _mf_concat)          // (callable | value)
mspec_metatable_field(NEGATE, _mf_negate)          // (callable | value)
mspec_metatable_field(NOT, _mf_not)                // (callable | value)
mspec_metatable_field(LENGTH, _mf_length)          // (callable | value)
mspec_metatable_field(REF, _mf_ref)                // (callable | value)
mspec_metatable_field(DEREF, _mf_deref)            // (callable | value)
mspec_metatable_field(ITERATOR, _mf_iter)          // (callable | value)
mspec_metatable_field(ITERATOR_INIT, _mf_iterinit) // (callable)
mspec_metatable_field(ITERATOR_HAS, _mf_iterhas)   // (callable | value)
mspec_metatable_field(ITERATOR_NEXT, _mf_iternext) // (callable | value)

mspec_metatable_field(TO_STRING, _mf_tostr)        // (callable | value) library
mspec_metatable_field(HASH, _mf_hash)              // (callable | value) library
mspec_metatable_field(COMPARE, _mf_compare)        // (callable | value) library

// control
mspec_metatable_field(MASK, _mf_mask)              // (value)
mspec_metatable_field(GC, _mf_gc)                  // (callable)
