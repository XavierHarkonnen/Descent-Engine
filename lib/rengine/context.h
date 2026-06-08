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

#ifndef DESCENT_SOURCE_RENGINE_CONTEXT_H
#define DESCENT_SOURCE_RENGINE_CONTEXT_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <descent/rengine.h>
#include <descent/types.h>

#include "device.h"
#include "input.h"
#include "instance.h"
#include "swapchain.h"
#include "window.h"


struct rengine_context {
	bool initialized;

	const struct rengine_create_info *info;

	struct instance instance;
	struct device device;
	struct window window;
	struct swapchain swapchain;
	struct input_tables input_tables;
};


struct rengine_context *rengine_context(void);

#endif