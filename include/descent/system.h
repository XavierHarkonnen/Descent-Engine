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

#ifndef DESCENT_SYSTEM_H
#define DESCENT_SYSTEM_H

#include <descent/build.h>
#include <descent/types.h>

bool is_main_thread(void);

u64 time_now(void);

u64 time_sleep(u64 duration);

bool futex_wait(u32 *key, u32 expected, u64 duration);

// bool futex_waitv(u32 **key, u64 count, u32 *expected);

bool futex_wake_single(u32 *key);

bool futex_wake_all(u32 *key);

#endif
