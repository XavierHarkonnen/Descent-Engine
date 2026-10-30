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

#ifndef DESCENT_LIB_FS_STATE_HPP
#define DESCENT_LIB_FS_STATE_HPP

#include <descent/thread/atomic.hpp>
#include <descent/thread/mutex.hpp>
#include <descent/type/string.hpp>

#include "fs.hpp"

namespace descent::fs {

constexpr u64 MAX_PROGRAM_NAME_SIZE = 256;

inline thread::Mutex mutex;
inline thread::atomic::Flag initialized(false);
inline const Directory INVALID_DIRECTORY;
inline char program_buffer[MAX_PROGRAM_NAME_SIZE];
inline string program_name = {};

}

#endif
