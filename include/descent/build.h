/* Copyright 2025 XavierHarkonnen9 and Enlarium
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DESCENT_BUILD_H
#define DESCENT_BUILD_H

#if !defined(__clang__)
#error "Descent Engine does not support this compiler!"
#endif

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "Descent Engine requires C11 or newer!"
#endif

#if !defined(__linux__) || defined(__ANDROID__)
#error "Descent Engine supports Linux (non-Android) only!"
#endif

#if !defined(__x86_64__) && !defined(_M_X64) && !defined(_M_AMD64)
#error "Descent Engine only supports x86-64!"
#endif

#if (-1 != ~0) || ((-1 & 3) != 3)
#error "Descent Engine requires two's complement representation for negative integers"
#endif

#if __CHAR_BIT__ != 8
#error "Descent Engine requires 8-bit bytes"
#endif

#if !defined(__BYTE_ORDER__) || __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "Descent Engine only supports little-endian byte order!"
#endif

#if !defined(__SIZEOF_INT128__)
#error "Descent Engine requires 128-bit integer support!"
#endif

#if __FLT_RADIX__ != 2
#error "Descent Engine requires binary floating point numbers!"
#endif

#if __SIZEOF_FLOAT__ != 4
#error "Descent Engine requires 4-byte floating point numbers!"
#endif

#if __FLT_MANT_DIG__ != 24
#error "Descent Engine requires IEEE 754 single precision floating point numbers!"
#endif

#if __SIZEOF_DOUBLE__ != 8
#error "Descent Engine requires 8-byte floating point numbers!"
#endif

#if __DBL_MANT_DIG__ != 53
#error "Descent Engine requires IEEE 754 double precision floating point numbers!"
#endif

#endif
