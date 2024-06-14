//
// Created by whyiskra on 30.12.23.
//

#include <math.h>
#include "morphinenostdlibs/math.h"

#define PI_CONST 3.14159265358979323846
#define E_CONST  2.7182818284590452354

#define math_func_1(name) \
static void math_##name(morphine_coroutine_t U) { \
    maux_nb_function(U) \
        maux_nb_init \
            maux_expect_args(U, 1); \
            mapi_push_arg(U, 0); \
            mapi_to_decimal(U); \
            ml_decimal a = mapi_get_decimal(U); \
            mapi_pop(U, 1); \
            mapi_push_decimal(U, name(a)); \
            maux_nb_return(); \
    maux_nb_end \
}

#define math_func_2(name) \
static void math_##name(morphine_coroutine_t U) { \
    maux_nb_function(U) \
        maux_nb_init \
            maux_expect_args(U, 2); \
            mapi_push_arg(U, 0); \
            mapi_to_decimal(U); \
            ml_decimal a = mapi_get_decimal(U); \
            mapi_push_arg(U, 1); \
            mapi_to_decimal(U); \
            ml_decimal b = mapi_get_decimal(U); \
            mapi_pop(U, 2); \
            mapi_push_decimal(U, name(a, b)); \
            maux_nb_return(); \
    maux_nb_end \
}

/*
 * cos   Compute cosine
 * sin   Compute sine
 * tan   Compute tangent
 * acos  Compute arc cosine
 * asin  Compute arc sine
 * atan  Compute arc tangent
 * atan2 Compute arc tangent with two parameters
 * cosh  Compute hyperbolic cosine
 * sinh  Compute hyperbolic sine
 * tanh  Compute hyperbolic tangent
 * acosh Compute area hyperbolic cosine
 * asinh Compute area hyperbolic sine
 * atanh Compute area hyperbolic tangent
 */

math_func_1(cos)

math_func_1(sin)

math_func_1(tan)

math_func_1(acos)

math_func_1(asin)

math_func_1(atan)

math_func_2(atan2)

math_func_1(cosh)

math_func_1(sinh)

math_func_1(tanh)

math_func_1(acosh)

math_func_1(asinh)

math_func_1(atanh)

/*
 * exp   Compute exponential function
 * log   Compute natural logarithm
 * log10 Compute common logarithm
 * exp2  Compute binary exponential function
 * log2  Compute binary logarithm
 */

math_func_1(exp)

math_func_1(exp2)

math_func_1(log)

math_func_1(log10)

math_func_1(log2)

/*
 * pow    Raise to power
 * sqrt   Compute square root
 * cbrt   Compute cubic root
 * hypot  Compute hypotenuse
 * erf    Compute error function
 * erfc   Compute complementary error function
 * tgamma Compute gamma function
 * lgamma Compute log-gamma function
 */

math_func_2(pow)

math_func_1(sqrt)

math_func_1(cbrt)

math_func_2(hypot)

math_func_1(erf)

math_func_1(erfc)

math_func_1(tgamma)

math_func_1(lgamma)

/*
 * ceil  Round up value
 * floor Round down value
 * trunc Truncate value
 * round Round to nearest
 * rint  Round to integral value
 */

math_func_1(ceil)

math_func_1(floor)

math_func_1(trunc)

math_func_1(round)

math_func_1(rint)

/*
 * fabs Compute absolute value
 */

math_func_1(fabs)

/*
 * modf Break into fractional and integral parts
 */

static void math_modf(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            mapi_to_decimal(U);
            ml_decimal a = mapi_get_decimal(U);
            mapi_pop(U, 1);

            ml_decimal integral = 0;
            ml_decimal fraction = modf(a, &integral);

            mapi_push_table(U);

            mapi_push_stringf(U, "integral");
            mapi_push_decimal(U, integral);
            mapi_table_set(U);

            mapi_push_stringf(U, "fraction");
            mapi_push_decimal(U, fraction);
            mapi_table_set(U);

            maux_nb_return();
    maux_nb_end
}

static void math_rad(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            mapi_to_decimal(U);
            ml_decimal a = mapi_get_decimal(U);
            mapi_pop(U, 1);

            mapi_push_decimal(U, a * PI_CONST / 180);
            maux_nb_return();
    maux_nb_end
}

static void math_deg(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            mapi_to_decimal(U);
            ml_decimal a = mapi_get_decimal(U);
            mapi_pop(U, 1);

            mapi_push_decimal(U, a * 180 / PI_CONST);
            maux_nb_return();
    maux_nb_end
}

static void math_imax(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            mapi_to_integer(U);
            ml_integer a = mapi_get_integer(U);

            mapi_push_arg(U, 1);
            mapi_to_integer(U);
            ml_integer b = mapi_get_integer(U);

            mapi_pop(U, 2);

            mapi_push_integer(U, a > b ? a : b);
            maux_nb_return();
    maux_nb_end
}

static void math_imin(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            mapi_to_integer(U);
            ml_integer a = mapi_get_integer(U);

            mapi_push_arg(U, 1);
            mapi_to_integer(U);
            ml_integer b = mapi_get_integer(U);

            mapi_pop(U, 2);

            mapi_push_integer(U, a > b ? b : a);
            maux_nb_return();
    maux_nb_end
}

static void math_dmax(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            mapi_to_decimal(U);
            ml_decimal a = mapi_get_decimal(U);

            mapi_push_arg(U, 1);
            mapi_to_decimal(U);
            ml_decimal b = mapi_get_decimal(U);

            mapi_pop(U, 2);

            mapi_push_decimal(U, a > b ? a : b);
            maux_nb_return();
    maux_nb_end
}

static void math_dmin(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            mapi_to_decimal(U);
            ml_decimal a = mapi_get_decimal(U);

            mapi_push_arg(U, 1);
            mapi_to_decimal(U);
            ml_decimal b = mapi_get_decimal(U);

            mapi_pop(U, 2);

            mapi_push_decimal(U, a > b ? b : a);
            maux_nb_return();
    maux_nb_end
}

static void math_isnan(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            mapi_to_decimal(U);
            ml_decimal a = mapi_get_decimal(U);

            mapi_pop(U, 1);

            mapi_push_boolean(U, isnan(a));
            maux_nb_return();
    maux_nb_end
}

static void math_isinf(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            mapi_to_decimal(U);
            ml_decimal a = mapi_get_decimal(U);

            mapi_pop(U, 1);

            mapi_push_boolean(U, isinf(a));
            maux_nb_return();
    maux_nb_end
}

static struct maux_construct_field table[] = {
    { "cos",    math_cos },
    { "sin",    math_sin },
    { "tan",    math_tan },

    { "acos",   math_acos },
    { "asin",   math_asin },
    { "atan",   math_atan },
    { "atan2",  math_atan2 },

    { "cosh",   math_cosh },
    { "sinh",   math_sinh },
    { "tanh",   math_tanh },

    { "acosh",  math_acosh },
    { "asinh",  math_asinh },
    { "atanh",  math_atanh },

    { "exp",    math_exp },
    { "exp2",   math_exp2 },
    { "log",    math_log },
    { "log10",  math_log10 },
    { "log2",   math_log2 },

    { "pow",    math_pow },
    { "sqrt",   math_sqrt },
    { "cbrt",   math_cbrt },
    { "hypot",  math_hypot },
    { "erf",    math_erf },
    { "erfc",   math_erfc },
    { "tgamma", math_tgamma },
    { "lgamma", math_lgamma },

    { "ceil",   math_ceil },
    { "floor",  math_floor },
    { "trunc",  math_trunc },
    { "round",  math_round },
    { "rint",   math_rint },

    { "rad",    math_rad },
    { "deg",    math_deg },

    { "abs",    math_fabs },
    { "dcomp",  math_modf },

    { "max",    math_imax },
    { "min",    math_imin },
    { "dmax",   math_dmax },
    { "dmin",   math_dmin },

    { "isnan",   math_isnan },
    { "isinf",   math_isinf },

    { NULL, NULL }
};

MORPHINE_LIB void mnostdlib_math_loader(morphine_coroutine_t U) {
    maux_construct(U, table, "math.");

    mapi_push_stringf(U, "pi");
    mapi_push_decimal(U, PI_CONST);
    mapi_table_set(U);

    mapi_push_stringf(U, "e");
    mapi_push_decimal(U, E_CONST);
    mapi_table_set(U);

    mapi_push_stringf(U, "nan");
    mapi_push_decimal(U, (ml_decimal) NAN);
    mapi_table_set(U);

    mapi_push_stringf(U, "inf");
    mapi_push_decimal(U, (ml_decimal) INFINITY);
    mapi_table_set(U);
}

MORPHINE_LIB void mnostdlib_math_call(morphine_coroutine_t U, const char *name, ml_size argc) {
    maux_construct_call(U, table, "math.", name, argc);
}
