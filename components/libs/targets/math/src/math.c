//
// Created by whyiskra on 30.12.23.
//

#include <math.h>
#include "morphinel/math.h"
#include "mtrand.h"

#define SHAREDKEY ("math")
#define RAND_TYPE ("math-rand")

#define INITIAL_SEED (7953)

static mtrand_t *push_rand(morphine_coroutine_t U) {
    mapi_type_declare(
        mapi_instance(U),
        RAND_TYPE,
        sizeof(mtrand_t),
        false,
        NULL,
        NULL,
        NULL,
        NULL
    );

    mtrand_t *rand = mapi_push_userdata(U, RAND_TYPE);
    *rand = mtrand_get(INITIAL_SEED);

    return rand;
}

static void rand_seed(morphine_coroutine_t U, ml_size seed) {
    mtrand_t *rand = mapi_userdata_pointer(U, RAND_TYPE);
    *rand = mtrand_get(seed);
}

static ml_size rand_gen(morphine_coroutine_t U) {
    mtrand_t *rand = mapi_userdata_pointer(U, RAND_TYPE);
    return mtrand_rand(rand);
}

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

            mapi_push_string(U, "integral");
            mapi_push_decimal(U, integral);
            mapi_table_set(U);

            mapi_push_string(U, "fraction");
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

            mapi_push_decimal(U, a * M_PI / 180);
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

            mapi_push_decimal(U, a * 180 / M_PI);
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

static void math_nan(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            mapi_push_decimal(U, nan(""));
            maux_nb_return();
    maux_nb_end
}

static void math_inf(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            mapi_push_decimal(U, HUGE_VAL);
            maux_nb_return();
    maux_nb_end
}

static void math_seed(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            ml_size seed = mapi_get_size(U, "seed");

            maux_sharedstorage_get(U, SHAREDKEY, "rand-data");
            rand_seed(U, seed);
            maux_nb_leave();
    maux_nb_end
}

static void math_rand(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            maux_sharedstorage_get(U, SHAREDKEY, "rand-data");
            mapi_push_integer(U, rand_gen(U));
            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("cos", math_cos),
    MAUX_CONSTRUCT_FUNCTION("sin", math_sin),
    MAUX_CONSTRUCT_FUNCTION("tan", math_tan),

    MAUX_CONSTRUCT_FUNCTION("acos", math_acos),
    MAUX_CONSTRUCT_FUNCTION("asin", math_asin),
    MAUX_CONSTRUCT_FUNCTION("atan", math_atan),
    MAUX_CONSTRUCT_FUNCTION("atan2", math_atan2),

    MAUX_CONSTRUCT_FUNCTION("cosh", math_cosh),
    MAUX_CONSTRUCT_FUNCTION("sinh", math_sinh),
    MAUX_CONSTRUCT_FUNCTION("tanh", math_tanh),

    MAUX_CONSTRUCT_FUNCTION("acosh", math_acosh),
    MAUX_CONSTRUCT_FUNCTION("asinh", math_asinh),
    MAUX_CONSTRUCT_FUNCTION("atanh", math_atanh),

    MAUX_CONSTRUCT_FUNCTION("exp", math_exp),
    MAUX_CONSTRUCT_FUNCTION("exp2", math_exp2),
    MAUX_CONSTRUCT_FUNCTION("log", math_log),
    MAUX_CONSTRUCT_FUNCTION("log10", math_log10),
    MAUX_CONSTRUCT_FUNCTION("log2", math_log2),

    MAUX_CONSTRUCT_FUNCTION("pow", math_pow),
    MAUX_CONSTRUCT_FUNCTION("sqrt", math_sqrt),
    MAUX_CONSTRUCT_FUNCTION("cbrt", math_cbrt),
    MAUX_CONSTRUCT_FUNCTION("hypot", math_hypot),
    MAUX_CONSTRUCT_FUNCTION("erf", math_erf),
    MAUX_CONSTRUCT_FUNCTION("erfc", math_erfc),
    MAUX_CONSTRUCT_FUNCTION("tgamma", math_tgamma),
    MAUX_CONSTRUCT_FUNCTION("lgamma", math_lgamma),

    MAUX_CONSTRUCT_FUNCTION("ceil", math_ceil),
    MAUX_CONSTRUCT_FUNCTION("floor", math_floor),
    MAUX_CONSTRUCT_FUNCTION("trunc", math_trunc),
    MAUX_CONSTRUCT_FUNCTION("round", math_round),
    MAUX_CONSTRUCT_FUNCTION("rint", math_rint),

    MAUX_CONSTRUCT_FUNCTION("rad", math_rad),
    MAUX_CONSTRUCT_FUNCTION("deg", math_deg),

    MAUX_CONSTRUCT_FUNCTION("abs", math_fabs),
    MAUX_CONSTRUCT_FUNCTION("dcomp", math_modf),

    MAUX_CONSTRUCT_FUNCTION("max", math_imax),
    MAUX_CONSTRUCT_FUNCTION("min", math_imin),
    MAUX_CONSTRUCT_FUNCTION("dmax", math_dmax),
    MAUX_CONSTRUCT_FUNCTION("dmin", math_dmin),

    MAUX_CONSTRUCT_FUNCTION("isnan", math_isnan),
    MAUX_CONSTRUCT_FUNCTION("isinf", math_isinf),

    MAUX_CONSTRUCT_FUNCTION("nan", math_nan),
    MAUX_CONSTRUCT_FUNCTION("inf", math_inf),

    MAUX_CONSTRUCT_FUNCTION("seed", math_seed),
    MAUX_CONSTRUCT_FUNCTION("rand", math_rand),

    MAUX_CONSTRUCT_DECIMAL("pi", M_PI),
    MAUX_CONSTRUCT_DECIMAL("e", M_E),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    push_rand(U);
    maux_sharedstorage_set(U, SHAREDKEY, "rand-data");
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mllib_math(void) {
    return (morphine_library_t) {
        .name = "math",
        .sharedkey = SHAREDKEY,
        .init = library_init
    };
}
