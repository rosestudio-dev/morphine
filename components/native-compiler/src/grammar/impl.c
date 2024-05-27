//
// Created by why-iskra on 31.05.2024.
//

#include "impl.h"

struct grammar_quantum grammar[] = {
#define quantum_norm_common(t, f, a) { .type = REDUCE_TYPE_##t, .is_wrapping = false, .normal = match_##f, .assemble = assemble_##a }
#define quantum_wrap_common(t, f, a) { .type = REDUCE_TYPE_##t, .is_wrapping = true, .wrapping = match_##f, .assemble = assemble_##a }
#define quantum_norm(t, func) quantum_norm_common(t, func, func)
#define quantum_wrap(t, func) quantum_wrap_common(t, func, func)
    quantum_norm(AST, ast),
    quantum_norm(STATEMENT, statement),
    quantum_norm(WHILE, while),
    quantum_norm(DO_WHILE, do_while),
    quantum_norm(FOR, for),
    quantum_norm(ITERATOR, iterator),
    quantum_norm(DECLARATION, declaration),
    quantum_norm(ASSIGMENT, assigment),

    quantum_norm(STATEMENT_BLOCK, statement_block),
    quantum_norm(EXPRESSION_BLOCK, expression_block),
    quantum_norm(BLOCK_ELEM, block_elem),
    quantum_norm(IMPLICIT_BLOCK_ELEM, implicit_block_elem),

    quantum_norm(STATEMENT_IF, statement_if),
    quantum_norm(EXPRESSION_IF, expression_if),

    quantum_norm(EXPRESSION, expression),
    quantum_wrap_common(OR, binary_or, binary),
    quantum_wrap_common(AND, binary_and, binary),
    quantum_wrap_common(EQUAL, binary_equal, binary),
    quantum_wrap_common(CONDITION, binary_condition, binary),
    quantum_wrap_common(CONCAT, binary_concat, binary),
    quantum_wrap_common(ADDITIVE, binary_additive, binary),
    quantum_wrap_common(MULTIPLICATIVE, binary_multiplicative, binary),
    quantum_norm(PREFIX, prefix),
    quantum_wrap(POSTFIX, postfix),
    quantum_norm(PRIMARY, primary),
    quantum_wrap(VARIABLE, variable),
    quantum_norm(VALUE, value),
    quantum_norm(TABLE, table),
    quantum_norm(VECTOR, vector),
    quantum_norm(FUNCTION, function),
#undef quantum_norm
#undef quantum_wrap
#undef quantum_norm_common
#undef quantum_wrap_common
};

size_t grammar_size(void) {
    return sizeof(grammar) / sizeof(grammar[0]);
}
