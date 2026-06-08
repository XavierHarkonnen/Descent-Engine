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

#ifndef DESCENT_SOURCE_RENGINE_DEVICE_H
#define DESCENT_SOURCE_RENGINE_DEVICE_H

#include <vulkan/vulkan_core.h>

#include <descent/types/core.h>


#define MAX_QUEUES_GRAPHICS 1
#define MAX_QUEUES_COMPUTE  1
#define MAX_QUEUES_TRANSFER 1
#define MAX_QUEUES_TOTAL    (MAX_QUEUES_GRAPHICS + MAX_QUEUES_COMPUTE + MAX_QUEUES_TRANSFER)

enum {
	QUEUE_ROLE_GRAPHICS,
	QUEUE_ROLE_COMPUTE,
	QUEUE_ROLE_TRANSFER
};


struct queue_family {
	VkQueue *queues; // Pointer to contiguous queue array within backing array
	f32 *priorities; // Pointer to contiguous priority array within backing array
	u32 count;       // Number of queues in family
	u32 index;       // Queue family index
	u32 flags;       // Queue family capabilities
};

struct queues {
	VkQueue handles[MAX_QUEUES_TOTAL]; // Backing array of queues
	f32 priorities[MAX_QUEUES_TOTAL];  // Backing array of priorities for each queue
	struct queue_family graphics;      // Universal     (required) : present + graphics + compute + transfer 
	struct queue_family compute;       // Async compute (optional) : compute + transfer
	struct queue_family transfer;      // Copy engine   (optional) : transfer
};

struct device {
	VkDevice device;

	struct {
		VkPhysicalDevice                   device;

		VkPhysicalDeviceProperties2        properties;
		VkPhysicalDeviceVulkan11Properties properties_1_1;

		VkPhysicalDeviceFeatures2          features;
		VkPhysicalDeviceVulkan11Features   features_1_1;
	} physical;

	struct queues queues;
};

struct rengine_context;


u64 rengine_create_device(struct rengine_context *context);

void rengine_destroy_device(struct rengine_context *context);

#endif
