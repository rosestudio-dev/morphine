//
// Created by why-iskra on 02.09.2024.
//

#include <memory.h>
#include "morphinel/bigint/object.h"
#include "morphine/utils/overflow.h"

#define BIGINT_AMORTIZATION  32

#define max(a, b) ((a) > (b) ? (a) : (b))

#define iszero(bint) ((bint)->size == 1 && (bint)->digits[0] == 0)
#define isone(bint)  ((bint)->size == 1 && (bint)->digits[0] == 1)

typedef uint8_t digit_t;

struct mc_bigint {
    size_t size;
    size_t allocated;
    digit_t *digits;
    bool is_negative;
};

static inline int digit2char(morphine_coroutine_t U, digit_t digit) {
    if (digit > 9) {
        mapi_error(U, "broken digit");
    }

    return '0' + digit;
}

static inline digit_t char2digit(morphine_coroutine_t U, int c) {
    digit_t digit = (digit_t) (c - '0');
    if (c < '0' || c > '9' || digit > 9) {
        mapi_error(U, "broken digit char");
    }

    return digit;
}

// userdata

static void bigint_userdata_init(morphine_instance_t I, void *data) {
    (void) I;

    struct mc_bigint *bigint = data;
    (*bigint) = (struct mc_bigint) {
        .size = 0,
        .allocated = 0,
        .digits = NULL,
        .is_negative = false
    };
}

static void bigint_userdata_free(morphine_instance_t I, void *data) {
    struct mc_bigint *bigint = data;
    mapi_allocator_free(I, bigint->digits);
}

static struct mc_bigint *bigint_userdata(morphine_coroutine_t U) {
    mapi_type_declare(
        mapi_instance(U),
        MC_BIGINT_USERDATA_TYPE,
        sizeof(struct mc_bigint),
        bigint_userdata_init,
        bigint_userdata_free,
        false
    );

    struct mc_bigint *bigint = mapi_push_userdata(U, MC_BIGINT_USERDATA_TYPE);

    bigint->digits = mapi_allocator_vec(mapi_instance(U), NULL, BIGINT_AMORTIZATION, sizeof(digit_t));
    bigint->allocated = BIGINT_AMORTIZATION;
    bigint->size = 1;
    bigint->digits[0] = 0;

    return bigint;
}

static struct mc_bigint *bigint_clone(morphine_coroutine_t U, struct mc_bigint *bigint) {
    struct mc_bigint *result = bigint_userdata(U);

    size_t amortizations = (bigint->size / BIGINT_AMORTIZATION) + 1;
    size_t allocated = amortizations * BIGINT_AMORTIZATION;
    result->digits = mapi_allocator_vec(
        mapi_instance(U), result->digits, allocated, sizeof(digit_t)
    );
    result->allocated = allocated;
    result->size = bigint->size;
    result->is_negative = bigint->is_negative;

    memcpy(result->digits, bigint->digits, sizeof(digit_t) * bigint->size);

    return result;
}

static void bigint_append(morphine_coroutine_t U, struct mc_bigint *bigint, digit_t digit) {
    if (digit > 9) {
        mapi_error(U, "broken digit");
    }

    if (bigint->size > bigint->allocated) {
        mapi_error(U, "broken bigint");
    }

    if (bigint->size == bigint->allocated) {
        overflow_add(bigint->allocated, BIGINT_AMORTIZATION, SIZE_MAX) {
            mapi_error(U, "bigint overflow");
        }

        bigint->digits = mapi_allocator_vec(
            mapi_instance(U),
            bigint->digits,
            bigint->allocated + BIGINT_AMORTIZATION,
            sizeof(digit_t)
        );

        bigint->allocated += BIGINT_AMORTIZATION;
    }

    bigint->digits[bigint->size] = digit;
    bigint->size++;
}

// support

static inline digit_t bigint_raw_getorzero(struct mc_bigint *bigint, size_t size, size_t index) {
    if (index >= size) {
        return 0;
    } else {
        return bigint->digits[index];
    }
}

static inline int bigint_raw_compare(struct mc_bigint *bigintA, struct mc_bigint *bigintB) {
    if (bigintA->size > bigintB->size) {
        return 1;
    } else if (bigintA->size < bigintB->size) {
        return -1;
    }

    for (size_t i = 0; i < bigintA->size; i++) {
        digit_t digitA = bigintA->digits[bigintA->size - i - 1];
        digit_t digitB = bigintB->digits[bigintA->size - i - 1];

        if (digitA > digitB) {
            return 1;
        } else if (digitA < digitB) {
            return -1;
        }
    }

    return 0;
}

static inline void bigint_raw_add(
    morphine_coroutine_t U,
    struct mc_bigint *bigintA,
    struct mc_bigint *bigintB,
    struct mc_bigint *result
) {
    size_t sizeA = bigintA->size;
    size_t sizeB = bigintB->size;

    result->size = 0;

    int carry = 0;
    size_t maxlen = max(sizeA, sizeB);

    for (size_t i = 0; i < maxlen; i++) {
        int sum = ((int) bigint_raw_getorzero(bigintA, sizeA, i)) +
                  ((int) bigint_raw_getorzero(bigintB, sizeB, i)) +
                  carry;

        if (sum >= 10) {
            carry = sum / 10;
            sum -= carry * 10;
        } else {
            carry = 0;
        }

        bigint_append(U, result, (digit_t) sum);
    }

    if (carry > 0) {
        bigint_append(U, result, (digit_t) carry);
    }
}

static inline void bigint_raw_sub(
    morphine_coroutine_t U,
    struct mc_bigint *bigintA,
    struct mc_bigint *bigintB,
    struct mc_bigint *result
) {
    int compare = bigint_raw_compare(bigintA, bigintB);
    struct mc_bigint *a = compare > 0 ? bigintA : bigintB;
    struct mc_bigint *b = compare > 0 ? bigintB : bigintA;
    result->is_negative = compare <= 0;

    int borrow = 0;
    size_t size = 0;
    bool zeros = true;

    size_t sizeA = a->size;
    size_t sizeB = b->size;

    for (size_t i = 0; i < sizeA; i++) {
        int diff = ((int) bigint_raw_getorzero(a, sizeA, i)) -
                   ((int) bigint_raw_getorzero(b, sizeB, i)) -
                   borrow;

        if (diff < 0) {
            borrow = 1;
            diff += 10;
        } else {
            borrow = 0;
        }

        if (diff != 0) {
            size = 0;
            zeros = false;
        } else {
            zeros = true;
        }

        if (zeros) {
            size++;
        }
    }

    if (size == sizeA) {
        result->size = 1;
        result->digits[0] = 0;
        result->is_negative = false;
        return;
    } else {
        result->size = 0;
    }

    borrow = 0;
    for (size_t i = 0; i < sizeA - size; i++) {
        int diff = ((int) bigint_raw_getorzero(a, sizeA, i)) -
                   ((int) bigint_raw_getorzero(b, sizeB, i)) -
                   borrow;

        if (diff < 0) {
            borrow = 1;
            diff += 10;
        } else {
            borrow = 0;
        }

        bigint_append(U, result, (digit_t) diff);
    }
}

static inline void bigint_raw_mul_single(
    morphine_coroutine_t U,
    struct mc_bigint *result,
    struct mc_bigint *bigint,
    digit_t digit
) {
    int carry = 0;
    size_t size = bigint->size;
    for (size_t i = 0; i < size; i++) {
        int sum = ((int) bigint_raw_getorzero(bigint, size, i)) * ((int) digit) + carry;

        if (sum >= 10) {
            carry = sum / 10;
            sum -= carry * 10;
        } else {
            carry = 0;
        }

        bigint_append(U, result, (digit_t) sum);
    }

    if (carry > 0) {
        bigint_append(U, result, (digit_t) carry);
    }
}

static inline void bigint_raw_mul(
    morphine_coroutine_t U,
    struct mc_bigint *bigintA,
    struct mc_bigint *bigintB,
    struct mc_bigint *result
) {
    if (iszero(bigintA) || iszero(bigintB)) {
        result->size = 1;
        result->digits[0] = 0;
        result->is_negative = false;
        return;
    }

    if (isone(bigintA) || isone(bigintB)) {
        struct mc_bigint *from = isone(bigintA) ? bigintB : bigintA;

        result->size = 0;
        for (size_t i = 0; i < from->size; i++) {
            bigint_append(U, result, from->digits[i]);
        }

        result->is_negative = bigintA->is_negative != bigintB->is_negative;
        return;
    }

    result->size = 0;

    struct mc_bigint *temp = bigint_userdata(U);
    for (size_t i = 0; i < bigintB->size; i++) {
        temp->size = 0;

        for (size_t n = 0; n < i; n++) {
            bigint_append(U, temp, 0);
        }

        bigint_raw_mul_single(U, temp, bigintA, bigintB->digits[i]);
        bigint_raw_add(U, result, temp, result);
    }
    mapi_pop(U, 1);

    result->is_negative = bigintA->is_negative != bigintB->is_negative;
}

static inline void bigint_raw_div(
    morphine_coroutine_t U,
    struct mc_bigint *bigintA,
    struct mc_bigint *bigintB,
    struct mc_bigint *result,
    struct mc_bigint *remainder
) {
    if (iszero(bigintB)) {
        mapi_error(U, "attempt to divide by zero");
    }

    struct mc_bigint *ten = bigint_userdata(U);
    bigint_append(U, ten, 1);

    struct mc_bigint *mul_temp = bigint_userdata(U);
    struct mc_bigint *add_temp = bigint_userdata(U);

    struct mc_bigint *divs[9];
    divs[0] = bigint_clone(U, bigintB);
    divs[0]->is_negative = false;
    for (size_t i = 1; i < 9; i++) {
        struct mc_bigint *div = bigint_userdata(U);
        div->is_negative = false;
        div->size = 0;
        bigint_raw_mul_single(U, div, bigintB, (digit_t) (i + 1));
        divs[i] = div;
    }

    for (size_t i = 0; i < bigintA->size; i++) {
        digit_t digit = bigintA->digits[bigintA->size - i - 1];

        mul_temp->size = 0;
        add_temp->digits[0] = digit;
        bigint_raw_mul(U, remainder, ten, mul_temp);
        bigint_raw_add(U, mul_temp, add_temp, remainder);

        digit_t new = 0;
        for (size_t n = 1; n < 10; n++) {
            struct mc_bigint *div = divs[9 - n];
            if (bigint_raw_compare(remainder, div) >= 0) {
                bigint_raw_sub(U, remainder, div, remainder);
                new = (digit_t) (10 - n);
                break;
            }
        }

        add_temp->digits[0] = new;
        bigint_raw_mul(U, result, ten, mul_temp);
        bigint_raw_add(U, mul_temp, add_temp, result);
    }

    if (!iszero(result)) {
        result->is_negative = bigintA->is_negative != bigintB->is_negative;
    }

    if (!iszero(remainder)) {
        remainder->is_negative = bigintA->is_negative;
    }

    mapi_pop(U, 12);
}

// api

MORPHINE_API struct mc_bigint *mlapi_get_bigint(morphine_coroutine_t U) {
    return mapi_userdata_pointer(U, MC_BIGINT_USERDATA_TYPE);
}

MORPHINE_API int mlapi_bigint_compare(struct mc_bigint *bigintA, struct mc_bigint *bigintB) {
    if ((bigintA->size > 0 || bigintA->digits[bigintA->size - 1] > 0) ||
        (bigintB->size > 0 || bigintB->digits[bigintB->size - 1] > 0)) {
        if (!bigintA->is_negative && bigintB->is_negative) {
            return 1;
        } else if (bigintA->is_negative && !bigintB->is_negative) {
            return -1;
        }
    }

    if (bigintA->is_negative) {
        return bigint_raw_compare(bigintB, bigintA);
    } else {
        return bigint_raw_compare(bigintA, bigintB);
    }
}

MORPHINE_API struct mc_bigint *mlapi_bigint_add(
    morphine_coroutine_t U,
    struct mc_bigint *bigintA,
    struct mc_bigint *bigintB,
    struct mc_bigint *result
) {
    if (result == NULL) {
        result = bigint_userdata(U);
    }

    if (bigintA->is_negative == bigintB->is_negative) {
        bigint_raw_add(U, bigintA, bigintB, result);
        result->is_negative = bigintA->is_negative;
    } else if (bigintA->is_negative) {
        bigint_raw_sub(U, bigintB, bigintA, result);
    } else {
        bigint_raw_sub(U, bigintA, bigintB, result);
    }

    return result;
}

MORPHINE_API struct mc_bigint *mlapi_bigint_sub(
    morphine_coroutine_t U,
    struct mc_bigint *bigintA,
    struct mc_bigint *bigintB,
    struct mc_bigint *result
) {
    if (result == NULL) {
        result = bigint_userdata(U);
    }

    if (bigintA->is_negative == bigintB->is_negative) {
        if (bigintA->is_negative) {
            bigint_raw_sub(U, bigintB, bigintA, result);
        } else {
            bigint_raw_sub(U, bigintA, bigintB, result);
        }
    } else {
        bigint_raw_add(U, bigintB, bigintA, result);
        result->is_negative = bigintA->is_negative;
    }

    return result;
}

MORPHINE_API struct mc_bigint *mlapi_bigint_mul(
    morphine_coroutine_t U,
    struct mc_bigint *bigintA,
    struct mc_bigint *bigintB,
    struct mc_bigint *result
) {
    struct mc_bigint *bigint = bigintA;
    if (result == NULL) {
        result = bigint_userdata(U);
    } else if (bigintA == result) {
        bigint = bigint_clone(U, bigintA);
    }

    bigint_raw_mul(U, bigint, bigintB, result);

    if (bigintA == result) {
        mapi_pop(U, 1);
    }

    return result;
}

MORPHINE_API struct mc_bigint *mlapi_bigint_div(
    morphine_coroutine_t U,
    struct mc_bigint *bigintA,
    struct mc_bigint *bigintB,
    struct mc_bigint *result
) {
    struct mc_bigint *bigint = bigintA;
    if (result == NULL) {
        result = bigint_userdata(U);
    } else if (bigintA == result) {
        bigint = bigint_clone(U, bigintA);
    }

    struct mc_bigint *remainder = bigint_userdata(U);
    bigint_raw_div(U, bigint, bigintB, result, remainder);
    mapi_pop(U, 1);

    if (bigintA == result) {
        mapi_pop(U, 1);
    }

    return result;
}

MORPHINE_API struct mc_bigint *mlapi_bigint_mod(
    morphine_coroutine_t U,
    struct mc_bigint *bigintA,
    struct mc_bigint *bigintB,
    struct mc_bigint *remainder
) {
    struct mc_bigint *bigint = bigintA;
    if (remainder == NULL) {
        remainder = bigint_userdata(U);
    } else if (bigintA == remainder) {
        bigint = bigint_clone(U, bigintA);
    }

    struct mc_bigint *result = bigint_userdata(U);
    bigint_raw_div(U, bigint, bigintB, result, remainder);
    mapi_pop(U, 1);

    if (bigintA == remainder) {
        mapi_pop(U, 1);
    }

    return remainder;
}

MORPHINE_API struct mc_bigint *
mlapi_bigint_negate(morphine_coroutine_t U, struct mc_bigint *bigint, bool self) {
    struct mc_bigint *result = bigint;
    if (!self) {
        result = bigint_clone(U, bigint);
    }

    result->is_negative = !bigint->is_negative;
    return result;
}

MORPHINE_API void mlapi_bigint_tostring(morphine_coroutine_t U, struct mc_bigint *bigint) {
    if (bigint->size == 0 || (bigint->is_negative && iszero(bigint))) {
        mapi_error(U, "broken bigint");
    }

    mapi_push_string(U, "");
    for (size_t i = 0; i < bigint->size; i++) {
        mapi_push_stringf(U, "%c", digit2char(U, bigint->digits[i]));
        mapi_rotate(U, 2);
        mapi_string_concat(U);
    }

    if (bigint->is_negative) {
        mapi_push_string(U, "-");
        mapi_rotate(U, 2);
        mapi_string_concat(U);
    }
}

MORPHINE_API struct mc_bigint *mlapi_bigint_from_integer(morphine_coroutine_t U, ml_integer integer) {
    struct mc_bigint *bigint = bigint_userdata(U);
    bigint->size = 0;
    bigint->is_negative = integer < 0;

    ml_integer temp = integer < 0 ? -integer : integer;
    while (temp >= 10) {
        bigint_append(U, bigint, (digit_t) (temp % 10));
        temp /= 10;
    }
    bigint_append(U, bigint, (digit_t) (temp));

    return bigint;
}

MORPHINE_API struct mc_bigint *mlapi_bigint_from_string(morphine_coroutine_t U, const char *string) {
    size_t size = strlen(string);
    if (size == 0) {
        mapi_error(U, "empty string");
    }

    bool is_negative = string[0] == '-';

    size_t real_size = 0;
    size_t offset = 0;
    bool zeros = true;
    for (size_t index = is_negative ? 1 : 0; index < size; index++) {
        if (zeros && char2digit(U, string[index]) != 0) {
            offset = index;
            zeros = false;
        }

        if (!zeros) {
            real_size++;
        }
    }

    struct mc_bigint *bigint = bigint_userdata(U);
    if (real_size == 0) {
        return bigint;
    }

    bigint->size = 0;
    bigint->is_negative = is_negative;
    for (size_t index = offset; index < size; index++) {
        if (index - offset >= real_size) {
            break;
        }

        char c = string[size - (index - offset) - 1];
        digit_t digit = char2digit(U, c);
        bigint_append(U, bigint, digit);
    }

    return bigint;
}
