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

#include <descent/rengine.h>

#include <stddef.h>

#include <descent/thread/atomic.h>
#include <descent/thread/core.h>
#include <descent/type/core.h>

#include "context.h"
#include "debug.h"
#include "device.h"
#include "instance.h"
#include "window.h"

static struct rengine_context context = {0};

struct rengine_context *rengine_context(void) {
	if (atomic_load(&context.initialized))
		return NULL;

	return &context;
}

static void rengine_cleanup(void) {
	atomic_store(&context.initialized, false);

	rengine_destroy_input_tables(&context);
	rengine_destroy_swapchain(&context);
	rengine_destroy_device(&context);
	rengine_destroy_window(&context);
	rengine_destroy_instance(&context);
}

u64 rengine_initialize(const struct rengine_create_info *info) {
	if (thread_id() != THREAD_ID_MAIN)
		return RENGINE_ERROR_THREAD;

	if (atomic_load(&context.initialized))
		return RENGINE_ERROR_INIT;

	if (!info)
		return RENGINE_ERROR_NULL;

	rengine_context_print(&context);

	context.info = info;

	u64 result;

	result = rengine_create_instance(&context);
	if (result) goto error;

	result = rengine_create_window(&context);
	if (result) goto error;

	result = rengine_create_device(&context);
	if (result) goto error;

	result = rengine_create_swapchain(&context);
	if (result) goto error;

	atomic_store(&context.initialized, true);
	
	rengine_context_print(&context);

	return RENGINE_SUCCESS;

	error: {
		rengine_cleanup();
		return result;
	}
}

u64 rengine_sys_terminate(void) {
	if (thread_id() != THREAD_ID_MAIN)
		return RENGINE_ERROR_THREAD;

	if (!atomic_load(&context.initialized))
		return RENGINE_ERROR_INIT;

	rengine_cleanup();

	return RENGINE_SUCCESS;
}

struct rengine_capabilities *rengine_capabilities(void) {
	if (atomic_load(&context.initialized))
		return NULL;

	// TODO
	return NULL;
}