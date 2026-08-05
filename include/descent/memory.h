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

#ifndef DESCENT_MEMORY_H
#define DESCENT_MEMORY_H

#include <descent/type/core.h>

// Global arena
// Per-thread arena

// Scratch allocator
// scratch_allocate();
// scratch_free();

enum mem_prot {
	MEM_NONE,
	MEM_READ,
	MEM_EDIT,
};

u64 mem_alloc_granularity(void);

void *mem_alloc(u64 *size, enum mem_prot prot);

bool mem_protect(void *alloc, u64 size, enum mem_prot prot);

bool mem_free(void *alloc, u64 size);

#endif
