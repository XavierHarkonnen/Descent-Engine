#ifndef DESCENT_TYPE_MATH_HPP
#define DESCENT_TYPE_MATH_HPP

#include <descent/type/core.hpp>
#include <descent/type/bit.hpp>

namespace descent::math {

constexpr u8   min(u8   x, u8   y) { return x < y ? x : y; }
constexpr u16  min(u16  x, u16  y) { return x < y ? x : y; }
constexpr u32  min(u32  x, u32  y) { return x < y ? x : y; }
constexpr u64  min(u64  x, u64  y) { return x < y ? x : y; }
constexpr u128 min(u128 x, u128 y) { return x < y ? x : y; }
constexpr f32  min(f32  x, f32  y) { return x < y ? x : y; }
constexpr f64  min(f64  x, f64  y) { return x < y ? x : y; }

constexpr u8   max(u8   x, u8   y) { return x > y ? x : y; }
constexpr u16  max(u16  x, u16  y) { return x > y ? x : y; }
constexpr u32  max(u32  x, u32  y) { return x > y ? x : y; }
constexpr u64  max(u64  x, u64  y) { return x > y ? x : y; }
constexpr u128 max(u128 x, u128 y) { return x > y ? x : y; }
constexpr f32  max(f32  x, f32  y) { return x > y ? x : y; }
constexpr f64  max(f64  x, f64  y) { return x > y ? x : y; }

constexpr     u8   abs(i8    x) { return (u8)   (x >= 0 ? x : -x); }
constexpr     u16  abs(i16   x) { return (u16)  (x >= 0 ? x : -x); }
constexpr     u32  abs(i32   x) { return (u32)  (x >= 0 ? x : -x); }
constexpr     u64  abs(i64   x) { return (u64)  (x >= 0 ? x : -x); }
constexpr     u128 abs(i128  x) { return (u128) (x >= 0 ? x : -x); }
static inline f32  abs(f32   x) { return __builtin_fabsf(x); }
static inline f64  abs(f64   x) { return __builtin_fabs(x); }

constexpr u8   f_log2(u8   x) { return 7   - (u8)   bit::clz(x); }
constexpr u16  f_log2(u16  x) { return 15  - (u16)  bit::clz(x); }
constexpr u32  f_log2(u32  x) { return 31  - (u32)  bit::clz(x); }
constexpr u64  f_log2(u64  x) { return 63  - (u64)  bit::clz(x); }
constexpr u128 f_log2(u128 x) { return 127 - (u128) bit::clz(x); }

constexpr u8   c_log2(u8   x) { return x <= 1 ? 0 : 8  - (u8)   bit::clz((u8)   (x - 1)); }
constexpr u16  c_log2(u16  x) { return x <= 1 ? 0 : 16 - (u16)  bit::clz((u16)  (x - 1)); }
constexpr u32  c_log2(u32  x) { return x <= 1 ? 0 : 32 - (u32)  bit::clz((u32)  (x - 1)); }
constexpr u64  c_log2(u64  x) { return x <= 1 ? 0 : 64 - (u64)  bit::clz((u64)  (x - 1)); }
constexpr u128 c_log2(u128 x) { return x <= 1 ? 0 : 128- (u128) bit::clz((u128) (x - 1)); }

constexpr bool ipow2(u8   x) { return x != 0 && (x & (x - 1)) == 0; }
constexpr bool ipow2(u16  x) { return x != 0 && (x & (x - 1)) == 0; }
constexpr bool ipow2(u32  x) { return x != 0 && (x & (x - 1)) == 0; }
constexpr bool ipow2(u64  x) { return x != 0 && (x & (x - 1)) == 0; }
constexpr bool ipow2(u128 x) { return x != 0 && (x & (x - 1)) == 0; }

constexpr u8   npow2(u8   x) { return x == 0 ? 1 : (u8)   (1 << (8   - bit::clz((u8)   (x - 1)))); }
constexpr u16  npow2(u16  x) { return x == 0 ? 1 : (u16)  (1 << (16  - bit::clz((u16)  (x - 1)))); }
constexpr u32  npow2(u32  x) { return x == 0 ? 1 : (u32)  (1 << (32  - bit::clz((u32)  (x - 1)))); }
constexpr u64  npow2(u64  x) { return x == 0 ? 1 : (u64)  (1 << (64  - bit::clz((u64)  (x - 1)))); }
constexpr u128 npow2(u128 x) { return x == 0 ? 1 : (u128) (1 << (128 - bit::clz((u128) (x - 1)))); }

constexpr bool add_overflow(u8   x, u8   y, u8   &out) { return __builtin_add_overflow(x, y, &out); }
constexpr bool add_overflow(u16  x, u16  y, u16  &out) { return __builtin_add_overflow(x, y, &out); }
constexpr bool add_overflow(u32  x, u32  y, u32  &out) { return __builtin_add_overflow(x, y, &out); }
constexpr bool add_overflow(u64  x, u64  y, u64  &out) { return __builtin_add_overflow(x, y, &out); }
constexpr bool add_overflow(u128 x, u128 y, u128 &out) { return __builtin_add_overflow(x, y, &out); }

constexpr bool sub_overflow(u8   x, u8   y, u8   &out) { return __builtin_sub_overflow(x, y, &out); }
constexpr bool sub_overflow(u16  x, u16  y, u16  &out) { return __builtin_sub_overflow(x, y, &out); }
constexpr bool sub_overflow(u32  x, u32  y, u32  &out) { return __builtin_sub_overflow(x, y, &out); }
constexpr bool sub_overflow(u64  x, u64  y, u64  &out) { return __builtin_sub_overflow(x, y, &out); }
constexpr bool sub_overflow(u128 x, u128 y, u128 &out) { return __builtin_sub_overflow(x, y, &out); }

constexpr u8   mul(u8   x, u8   y, u8   &high) { u16  result = (u16)  x * (u16)  y; high = (u8)  (result >> 8 ); return (u8)  result; }
constexpr u16  mul(u16  x, u16  y, u16  &high) { u32  result = (u32)  x * (u32)  y; high = (u16) (result >> 16); return (u16) result; }
constexpr u32  mul(u32  x, u32  y, u32  &high) { u64  result = (u64)  x * (u64)  y; high = (u32) (result >> 32); return (u32) result; }
constexpr u64  mul(u64  x, u64  y, u64  &high) { u128 result = (u128) x * (u128) y; high = (u64) (result >> 64); return (u64) result; }
constexpr u128 mul(u128 x, u128 y, u128 &high) {
	u128 x_l = x & (u128) U64_MAX;
	u128 x_h = x >> 64;
	u128 y_l = y & (u128) U64_MAX;
	u128 y_h = y >> 64;

	u128 ll = x_l * y_l;
	u128 lh = x_l * y_h;
	u128 hl = x_h * y_l;
	u128 hh = x_h * y_h;

	u128 c = 0;
	u128 l = ll;
	c += add_overflow(l, (u128) (lh << 64), l);
	c += add_overflow(l, (u128) (hl << 64), l);
	u128 h = hh + (u128) (lh >> 64) + (u128) (hl >> 64) + c;

	high = h;
	return l;
}

static inline f32 pow(f32 b, f32 x) { return __builtin_powf(b, x); }
static inline f64 pow(f64 b, f64 x) { return __builtin_pow(b, x); }

static inline f32 exp(f32 x) { return __builtin_expf(x); }
static inline f64 exp(f64 x) { return __builtin_exp(x); }

static inline f32 exp2(f32 x) { return __builtin_exp2f(x); }
static inline f64 exp2(f64 x) { return __builtin_exp2(x); }

static inline f32 exp10(f32 x) { return __builtin_exp10f(x); }
static inline f64 exp10(f64 x) { return __builtin_exp10(x); }

static inline f32 expm1(f32 x) { return __builtin_expm1f(x); }
static inline f64 expm1(f64 x) { return __builtin_expm1(x); }

static inline f32 log(f32 x) { return __builtin_logf(x); }
static inline f64 log(f64 x) { return __builtin_log(x); }

static inline f32 log2(f32 x) { return __builtin_log2f(x); }
static inline f64 log2(f64 x) { return __builtin_log2(x); }

static inline f32 log10(f32 x) { return __builtin_log10f(x); }
static inline f64 log10(f64 x) { return __builtin_log10(x); }

static inline f32 log1p(f32 x) { return __builtin_log1pf(x); }
static inline f64 log1p(f64 x) { return __builtin_log1p(x); }

static inline f32 sqrt(f32 x) { return __builtin_sqrtf(x); }
static inline f64 sqrt(f64 x) { return __builtin_sqrt(x); }

static inline f32 cbrt(f32 x) { return __builtin_cbrtf(x); }
static inline f64 cbrt(f64 x) { return __builtin_cbrt(x); }

static inline f32 hypot(f32 x, f32 y) { return __builtin_hypotf(x, y); }
static inline f64 hypot(f64 x, f64 y) { return __builtin_hypot(x, y); }

static inline f32 sin(f32 x) { return __builtin_sinf(x); }
static inline f64 sin(f64 x) { return __builtin_sin(x); }

static inline f32 cos(f32 x) { return __builtin_cosf(x); }
static inline f64 cos(f64 x) { return __builtin_cos(x); }

static inline f32 tan(f32 x) { return __builtin_tanf(x); }
static inline f64 tan(f64 x) { return __builtin_tan(x); }

static inline f32 asin(f32 x) { return __builtin_asinf(x); }
static inline f64 asin(f64 x) { return __builtin_asin(x); }

static inline f32 acos(f32 x) { return __builtin_acosf(x); }
static inline f64 acos(f64 x) { return __builtin_acos(x); }

static inline f32 atan(f32 x) { return __builtin_atanf(x); }
static inline f64 atan(f64 x) { return __builtin_atan(x); }

static inline f32 atan2(f32 y, f32 x) { return __builtin_atan2f(y, x); }
static inline f64 atan2(f64 y, f64 x) { return __builtin_atan2(y, x); }

static inline f32 sinh(f32 x) { return __builtin_sinhf(x); }
static inline f64 sinh(f64 x) { return __builtin_sinh(x); }

static inline f32 cosh(f32 x) { return __builtin_coshf(x); }
static inline f64 cosh(f64 x) { return __builtin_cosh(x); }

static inline f32 tanh(f32 x) { return __builtin_tanhf(x); }
static inline f64 tanh(f64 x) { return __builtin_tanh(x); }

static inline f32 asinh(f32 x) { return __builtin_asinhf(x); }
static inline f64 asinh(f64 x) { return __builtin_asinh(x); }

static inline f32 acosh(f32 x) { return __builtin_acoshf(x); }
static inline f64 acosh(f64 x) { return __builtin_acosh(x); }

static inline f32 atanh(f32 x) { return __builtin_atanhf(x); }
static inline f64 atanh(f64 x) { return __builtin_atanh(x); }

static inline f32 floor(f32 x) { return __builtin_floorf(x); }
static inline f64 floor(f64 x) { return __builtin_floor(x); }

static inline f32 ceil(f32 x) { return __builtin_ceilf(x); }
static inline f64 ceil(f64 x) { return __builtin_ceil(x); }

static inline f32 trunc(f32 x) { return __builtin_truncf(x); }
static inline f64 trunc(f64 x) { return __builtin_trunc(x); }

static inline f32 round(f32 x) { return __builtin_roundf(x); }
static inline f64 round(f64 x) { return __builtin_round(x); }

static inline f32 nearbyint(f32 x) { return __builtin_nearbyintf(x); }
static inline f64 nearbyint(f64 x) { return __builtin_nearbyint(x); }

static inline f32 rint(f32 x) { return __builtin_rintf(x); }
static inline f64 rint(f64 x) { return __builtin_rint(x); }

static inline f32 fmod(f32 x, f32 y) { return __builtin_fmodf(x, y); }
static inline f64 fmod(f64 x, f64 y) { return __builtin_fmod(x, y); }

static inline f32 remainder(f32 x, f32 y) { return __builtin_remainderf(x, y); }
static inline f64 remainder(f64 x, f64 y) { return __builtin_remainder(x, y); }

static inline f32 copysign(f32 x, f32 y) {
    return __builtin_copysignf(x, y);
}
static inline f64 copysign(f64 x, f64 y) {
    return __builtin_copysign(x, y);
}

static inline f32 fmin(f32 x, f32 y) {
    return __builtin_fminf(x, y);
}
static inline f64 fmin(f64 x, f64 y) {
    return __builtin_fmin(x, y);
}

static inline f32 fmax(f32 x, f32 y) {
    return __builtin_fmaxf(x, y);
}
static inline f64 fmax(f64 x, f64 y) {
    return __builtin_fmax(x, y);
}

static inline f32 fdim(f32 x, f32 y) {
    return __builtin_fdimf(x, y);
}
static inline f64 fdim(f64 x, f64 y) {
    return __builtin_fdim(x, y);
}

static inline f32 fma(f32 x, f32 y, f32 z) {
    return __builtin_fmaf(x, y, z);
}

static inline f64 fma(f64 x, f64 y, f64 z) {
    return __builtin_fma(x, y, z);
}

static inline bool isnan(f32 x) { return __builtin_isnan(x); }
static inline bool isnan(f64 x) { return __builtin_isnan(x); }

static inline bool isinf(f32 x) { return __builtin_isinf(x); }
static inline bool isinf(f64 x) { return __builtin_isinf(x); }

static inline bool isfinite(f32 x) { return __builtin_isfinite(x); }
static inline bool isfinite(f64 x) { return __builtin_isfinite(x); }

static inline bool isnormal(f32 x) { return __builtin_isnormal(x); }
static inline bool isnormal(f64 x) { return __builtin_isnormal(x); }

static inline bool signbit(f32 x) { return __builtin_signbit(x); }
static inline bool signbit(f64 x) { return __builtin_signbit(x); }

}

#endif