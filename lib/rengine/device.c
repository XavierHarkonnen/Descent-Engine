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

#include "device.h"

#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <descent/rengine.h>
#include <descent/types/core.h>
#include <descent/types/bits.h>
#include <descent/types/math.h>

#include "context.h"


#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)


static const char *const DEVICE_EXTENSIONS[] = {
	"VK_KHR_swapchain",
	"VK_KHR_dynamic_rendering",
	"VK_KHR_synchronization2",
	"VK_KHR_timeline_semaphore"
};
enum { DEVICE_EXTENSION_COUNT = sizeof(DEVICE_EXTENSIONS) / sizeof(*DEVICE_EXTENSIONS) };


static inline void query_physical_device_information(struct rengine_context *context) {
	context->device.physical.properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	context->device.physical.properties.pNext = &context->device.physical.properties_1_1;
	context->device.physical.properties_1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
	context->device.physical.properties_1_1.pNext = NULL;
	vkGetPhysicalDeviceProperties2(context->device.physical.device, &context->device.physical.properties);

	context->device.physical.features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	context->device.physical.features.pNext = &context->device.physical.features_1_1;
	context->device.physical.features_1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	context->device.physical.features_1_1.pNext = NULL;
	vkGetPhysicalDeviceFeatures2(context->device.physical.device, &context->device.physical.features);
}


static inline u64 check_physical_device_extension_support(struct rengine_context *context) {
	VkResult result;

	u32 extension_count;
	result = vkEnumerateDeviceExtensionProperties(context->device.physical.device, NULL, &extension_count, NULL);
	if (result != VK_SUCCESS)
		return RENGINE_ERROR_BACKEND;

	if (extension_count < DEVICE_EXTENSION_COUNT)
		return RENGINE_ERROR_UNSUPPORTED;

	VkExtensionProperties *extensions = malloc(extension_count * sizeof(VkExtensionProperties));
	if (extensions == NULL)
		return RENGINE_ERROR_MEMORY;

	result = vkEnumerateDeviceExtensionProperties(context->device.physical.device, NULL, &extension_count, extensions);
	if (result != VK_SUCCESS) {
		free(extensions);
		return RENGINE_ERROR_BACKEND;
	}

	bool all_found = true;
	for (u32 required_index = 0; required_index < DEVICE_EXTENSION_COUNT; ++required_index) {
		bool found = false;

		for (u32 available_index = 0; available_index < extension_count; ++available_index) {
			if (strcmp(DEVICE_EXTENSIONS[required_index], extensions[available_index].extensionName) == 0) {
				found = true;
				break;
			}
		}

		if (!found) {
			all_found = false;
			break;
		}
	}

	free(extensions);

	if (!all_found)
		return RENGINE_ERROR_UNSUPPORTED;

	return RENGINE_SUCCESS;
}


static inline u32 clean_queue_flags(u32 flags) {
	if (flags & VK_QUEUE_GRAPHICS_BIT || flags & VK_QUEUE_COMPUTE_BIT) flags |= VK_QUEUE_TRANSFER_BIT;
	return flags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT);
}


static inline void classify_queue_family(struct rengine_context *context, u32 index, u32 count, u32 flags) {
	struct queues *queues = &context->device.queues;

	VkBool32 present;
	VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(context->device.physical.device, index, context->window.surface, &present);
	if (result != VK_SUCCESS) present = VK_FALSE;
	
	bool viable_graphics = queues->graphics.count == 0 || popcnt_32(flags) < popcnt_32(queues->graphics.flags);
	bool valid_graphics = present && (flags & VK_QUEUE_GRAPHICS_BIT) && (flags & VK_QUEUE_COMPUTE_BIT);
	if (viable_graphics && valid_graphics) {
		u32 old_index = queues->graphics.index;
		u32 old_flags = queues->graphics.flags;
		u32 old_count = queues->graphics.count;

		queues->graphics.index = index;
		queues->graphics.flags = clean_queue_flags(flags);
		queues->graphics.count = count;

		if (old_count != 0) classify_queue_family(context, old_index, old_count, old_flags);
		return;
	}

	bool viable_compute = queues->compute.count == 0 || popcnt_32(flags) < popcnt_32(queues->compute.flags);
	bool valid_compute  = flags & VK_QUEUE_COMPUTE_BIT;
	if (viable_compute && valid_compute) {
		u32 old_index = queues->compute.index;
		u32 old_flags = queues->compute.flags;
		u32 old_count = queues->compute.count;

		queues->compute.index = index;
		queues->compute.flags = clean_queue_flags(flags);
		queues->compute.count = count;

		if (old_count != 0) classify_queue_family(context, old_index, old_count, old_flags);
		return;
	}

	bool viable_transfer = queues->transfer.count == 0 || popcnt_32(flags) < popcnt_32(queues->transfer.flags);
	bool valid_transfer = (flags & VK_QUEUE_GRAPHICS_BIT) || (flags & VK_QUEUE_COMPUTE_BIT) || (flags & VK_QUEUE_TRANSFER_BIT);
	if (viable_transfer && valid_transfer) {
		u32 old_index = queues->transfer.index;
		u32 old_flags = queues->transfer.flags;
		u32 old_count = queues->transfer.count;

		queues->transfer.index = index;
		queues->transfer.flags = clean_queue_flags(flags);
		queues->transfer.count = count;

		if (old_count != 0) classify_queue_family(context, old_index, old_count, old_flags);
		return;
	}
}


static inline void assign_queue_families(struct rengine_context *context) {
	struct queues *queues = &context->device.queues;

	const u32 max_queues_transfer = MAX_QUEUES_TRANSFER;
	if (queues->transfer.count > max_queues_transfer) {
		queues->transfer.count = max_queues_transfer;
	}

	// Use compute to fill for missing transfer queues
	const u32 max_queues_compute = MAX_QUEUES_COMPUTE + max_queues_transfer - queues->transfer.count;
	if (queues->compute.count > max_queues_compute) {
		queues->compute.count = max_queues_compute;
	}

	// Use graphics to fill for missing compute queues
	const u32 max_queues_graphics = MAX_QUEUES_GRAPHICS + max_queues_compute - queues->compute.count;
	if (queues->graphics.count > max_queues_graphics) {
		queues->graphics.count = max_queues_graphics;
	}

	for (u32 i = 0; i < MAX_QUEUES_TOTAL; ++i) {
		queues->priorities[i] = 1.0f;
	}

	queues->graphics.queues = queues->handles;
	queues->graphics.priorities = queues->priorities;

	queues->compute.queues = queues->graphics.queues + queues->graphics.count;
	queues->compute.priorities = queues->graphics.priorities + queues->graphics.count;

	queues->transfer.queues = queues->compute.queues + queues->compute.count;
	queues->transfer.priorities = queues->compute.priorities + queues->compute.count;
}


static inline u64 select_queue_families(struct rengine_context *context) {
	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(context->device.physical.device, &queue_family_count, NULL);

	VkQueueFamilyProperties* queue_families = malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
	if (queue_families == NULL)
		return RENGINE_ERROR_MEMORY;

	vkGetPhysicalDeviceQueueFamilyProperties(context->device.physical.device, &queue_family_count, queue_families);

	for (u32 index = 0; index < queue_family_count; ++index) {
		u32 count = queue_families[index].queueCount;
		u32 flags = queue_families[index].queueFlags;

		classify_queue_family(context, index, count, flags);
	}

	free(queue_families);

	assign_queue_families(context);

	if (context->device.queues.graphics.count == 0)
		return RENGINE_ERROR_UNSUPPORTED;

	return RENGINE_SUCCESS;
}


static inline u64 select_vulkan_physical_device(struct rengine_context *context) {
	VkResult result;
	
	u32 device_count;

	result = vkEnumeratePhysicalDevices(context->instance.instance, &device_count, NULL);
	if (result != VK_SUCCESS)
		return RENGINE_ERROR_BACKEND;

	if (device_count == 0)
		return RENGINE_ERROR_UNSUPPORTED;

	VkPhysicalDevice *physical_devices = malloc(device_count * sizeof(VkPhysicalDevice));
	if (physical_devices == NULL)
		return RENGINE_ERROR_MEMORY;

	result = vkEnumeratePhysicalDevices(context->instance.instance, &device_count, physical_devices);
	if (result != VK_SUCCESS) {
		free(physical_devices);
		return RENGINE_ERROR_BACKEND;
	}

	bool found_device = false;
	for (u32 index = 0; index < device_count; ++index) {
		u64 status;

		memset(&context->device, 0, sizeof(context->device));
		context->device.physical.device = physical_devices[index];

		query_physical_device_information(context);

		status = check_physical_device_extension_support(context);
		if (status) continue;

		status = select_queue_families(context);
		if (status) continue;

		if (context->info && context->info->preferred_device) {
			if (memcmp(context->info->preferred_device, context->device.physical.properties_1_1.deviceUUID, sizeof(uuid)) == 0) {
				found_device = true;
				break;
			}
		}
		else {
			found_device = true;
			break;
		}
	}

	free(physical_devices);

	if (!found_device)
		return RENGINE_ERROR_UNSUPPORTED;

	return RENGINE_SUCCESS;
}


static inline u64 create_vulkan_device(struct rengine_context *context) {
	VkDeviceQueueCreateInfo queue_infos[3] = {0};
	u32 queue_info_count = 0;

	queue_infos[queue_info_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_infos[queue_info_count].queueFamilyIndex = context->device.queues.graphics.index;
	queue_infos[queue_info_count].queueCount = context->device.queues.graphics.count;
	queue_infos[queue_info_count].pQueuePriorities = context->device.queues.graphics.priorities;
	++queue_info_count;

	if (context->device.queues.compute.count > 0) {
		queue_infos[queue_info_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_infos[queue_info_count].queueFamilyIndex = context->device.queues.compute.index;
		queue_infos[queue_info_count].queueCount = context->device.queues.compute.count;
		queue_infos[queue_info_count].pQueuePriorities = context->device.queues.compute.priorities;
		++queue_info_count;
	}

	if (context->device.queues.transfer.count > 0) {
		queue_infos[queue_info_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_infos[queue_info_count].queueFamilyIndex = context->device.queues.transfer.index;
		queue_infos[queue_info_count].queueCount = context->device.queues.transfer.count;
		queue_infos[queue_info_count].pQueuePriorities = context->device.queues.transfer.priorities;
		++queue_info_count;
	}

	VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features = {0};
	dynamic_rendering_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
	dynamic_rendering_features.dynamicRendering = VK_TRUE;

	VkDeviceCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pQueueCreateInfos = queue_infos;
	create_info.queueCreateInfoCount = queue_info_count;
	create_info.ppEnabledExtensionNames = DEVICE_EXTENSIONS;
	create_info.enabledExtensionCount = DEVICE_EXTENSION_COUNT;
	create_info.pNext = &dynamic_rendering_features;

	VkResult result = vkCreateDevice(context->device.physical.device, &create_info, NULL, &context->device.device);
	if (result != VK_SUCCESS)
		return RENGINE_ERROR_BACKEND;

	for (u32 index = 0; index < context->device.queues.graphics.count; ++index) {
		vkGetDeviceQueue(context->device.device, context->device.queues.graphics.index, index, &context->device.queues.graphics.queues[index]);
	}

	for (u32 index = 0; index < context->device.queues.compute.count; ++index) {
		vkGetDeviceQueue(context->device.device, context->device.queues.compute.index, index, &context->device.queues.compute.queues[index]);
	}

	for (u32 index = 0; index < context->device.queues.transfer.count; ++index) {
		vkGetDeviceQueue(context->device.device, context->device.queues.transfer.index, index, &context->device.queues.transfer.queues[index]);
	}

	return RENGINE_SUCCESS;
}

u64 rengine_create_device(struct rengine_context *context) {
	u64 result = select_vulkan_physical_device(context);
	if (result)
		return result;

	return create_vulkan_device(context);
}


void rengine_destroy_device(struct rengine_context *context) {
	if (context->device.device != VK_NULL_HANDLE)
		vkDestroyDevice(context->device.device, NULL);
}
