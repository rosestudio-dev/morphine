//
// Created by why-iskra on 11.06.2024.
//

#pragma once

#include "likely.h"
#include <stdint.h>

#define mm_type_unsigned_max(t) ((t) (~((t) 0)))
#define mm_type_unsigned_min(t) ((t) 0)
#define mm_type_signed_max(t)   ((t) (~(((t) 1) << (sizeof(t) * 8 - 1))))
#define mm_type_signed_min(t)   ((t) (((t) 1) << (sizeof(t) * 8 - 1)))

#define mm_type_calc_max(a, b) ((a) > (b) ? (a) : (b))
#define mm_type_calc_min(a, b) ((a) < (b) ? (a) : (b))
#define mm_typemax(t) ((__typeof__(t)) mm_type_calc_max(mm_type_signed_max(__typeof__(t)), mm_type_unsigned_max(__typeof__(t))))
#define mm_typemin(t) ((__typeof__(t)) mm_type_calc_min(mm_type_signed_min(__typeof__(t)), mm_type_unsigned_min(__typeof__(t))))

#define mm_is_unsigned(t) (mm_typemin(t) == 0)

#define mm_overflow_cond_add(a, b) ((b) >= 0 ? (a) > mm_typemax(a) - (b) : (a) < mm_typemin(a) - (b))
#define mm_overflow_cond_sub(a, b) ((b) >= 0 ? (a) < mm_typemin(a) + (b) : (a) > mm_typemax(a) + (b))
#define mm_overflow_cond_mul(a, b) \
   ((((a) > 0) && ((b) > 0) && ((a) > (mm_typemax(a) / (b)))) || \
    (((a) > 0) && ((b) < 0) && ((a) > (mm_typemin(a) / (b)))) || \
    (((a) < 0) && ((b) > 0) && ((a) < (mm_typemin(a) / (b)))) || \
    (((a) < 0) && ((b) < 0) && ((a) < (mm_typemax(a) / (b)))))

#define mm_overflow_cond_cast_u(t, v)   (((uintmax_t) (v)) > ((uintmax_t) mm_typemax(t)))
#define mm_overflow_cond_cast_m_s(t, v) (((intmax_t) (v)) >= 0 ? ((uintmax_t) (v)) > ((uintmax_t) mm_typemax(t)) : ((intmax_t) (v)) < ((intmax_t) mm_typemin(t)))
#define mm_overflow_cond_cast_z_s(t, v) (((intmax_t) (v)) >= 0 ? ((uintmax_t) (v)) > ((uintmax_t) mm_typemax(t)) : ((intmax_t) (v)) < 0)
#define mm_overflow_cond_mcast(t, v, m) (mm_is_unsigned(v) ? mm_overflow_cond_cast_u(t, (v)) : mm_overflow_cond_cast_##m##_s(t, (v)))
#define mm_overflow_cond_cast(t, v)  (mm_overflow_cond_mcast(t, (v), m))
#define mm_overflow_cond_ucast(t, v) (mm_overflow_cond_mcast(t, (v), z))

#define mm_overflow_cast(t, v)  if(mm_unlikely(mm_overflow_cond_cast(t, (v))))
#define mm_overflow_ucast(t, v) if(mm_unlikely(mm_overflow_cond_ucast(t, (v))))
#define mm_overflow_add(a, b)   if(mm_unlikely(mm_overflow_cond_add((a), (b))))
#define mm_overflow_sub(a, b)   if(mm_unlikely(mm_overflow_cond_sub((a), (b))))
#define mm_overflow_mul(a, b)   if(mm_unlikely(mm_overflow_cond_mul((a), (b))))

#define mm_overflow_opd_cast(t, v, d)  (mm_overflow_cond_cast(t, (v)) ? (d) : (__typeof__(t)) (v))
#define mm_overflow_opd_ucast(t, v, d) (mm_overflow_cond_ucast(t, (v)) ? (d) : (__typeof__(t)) (v))
#define mm_overflow_opd_add(a, b, d)   (mm_overflow_cond_add((a), (b)) ? (d) : (a) + (b))
#define mm_overflow_opd_sub(a, b, d)   (mm_overflow_cond_sub((a), (b)) ? (d) : (a) - (b))
#define mm_overflow_opd_mul(a, b, d)   (mm_overflow_cond_mul((a), (b)) ? (d) : (a) * (b))

#define mm_overflow_opc_cast(t, v, c)  ({mm_overflow_cast(t, (v)) { c; } (__typeof__(t)) (v);})
#define mm_overflow_opc_ucast(t, v, c) ({mm_overflow_ucast(t, (v)) { c; } (__typeof__(t)) (v);})
#define mm_overflow_opc_add(a, b, c)   ({mm_overflow_add((a), (b)) { c; } (a) + (b);})
#define mm_overflow_opc_sub(a, b, c)   ({mm_overflow_sub((a), (b)) { c; } (a) - (b);})
#define mm_overflow_opc_mul(a, b, c)   ({mm_overflow_mul((a), (b)) { c; } (a) * (b);})
