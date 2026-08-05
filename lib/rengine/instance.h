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

#ifndef DESCENT_SOURCE_RENGINE_INSTANCE_H
#define DESCENT_SOURCE_RENGINE_INSTANCE_H

#include <vulkan/vulkan_core.h>

#include <descent/type/core.h>


struct instance {
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
};

struct rengine_context;


u64 rengine_create_instance(struct rengine_context *context);

void rengine_destroy_instance(struct rengine_context *context);

#endif
