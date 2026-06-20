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

#include "instance.h"

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include <descent/rengine.h>
#include <descent/metadata.h>
#include <descent/types/core.h>

#include "context.h"


#define DEBUG_EXTENSION_COUNT (sizeof(DEBUG_EXTENSIONS) / sizeof(*DEBUG_EXTENSIONS))
static const char *const DEBUG_EXTENSIONS[] = {
	"VK_EXT_debug_utils"
};

#define VALIDATION_LAYER_COUNT (sizeof(VALIDATION_LAYERS) / sizeof(*VALIDATION_LAYERS))
static const char *const VALIDATION_LAYERS[] = {
	"VK_LAYER_KHRONOS_validation"
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData
) {
	(void) messageSeverity;
	(void) messageType;
	(void) pUserData;

	fprintf(stderr, "Vulkan: %s\n", pCallbackData->pMessage);

	return VK_FALSE;
}

static const VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info = {
	.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
	.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
	.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
	.pfnUserCallback = debug_callback
};


static inline u64 create_vulkan_debug_messenger(struct rengine_context *context) {
	PFN_vkCreateDebugUtilsMessengerEXT function = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(context->instance.instance, "vkCreateDebugUtilsMessengerEXT");
	if (!function) {
		return RENGINE_ERROR_BACKEND;
	}

	VkResult result = function(context->instance.instance, &debug_messenger_create_info, NULL, &context->instance.debug_messenger);
	if (result != VK_SUCCESS) {
		return RENGINE_ERROR_BACKEND;
	}

	return RENGINE_SUCCESS;
}


static inline void destroy_vulkan_debug_messenger(struct rengine_context *context) {
	if (context->instance.debug_messenger == VK_NULL_HANDLE)
		return;

	PFN_vkDestroyDebugUtilsMessengerEXT function = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(context->instance.instance, "vkDestroyDebugUtilsMessengerEXT");
	if (!function) {
		return;
	}

	function(context->instance.instance, context->instance.debug_messenger, NULL);
}


static inline u64 create_instance_extensions(
	const struct rengine_context *context,
	const char ***extensions,
	u32 *extension_count
) {
	u32 GLFW_EXTENSION_COUNT = 0;
	const char **GLFW_EXTENSIONS = glfwGetRequiredInstanceExtensions(&GLFW_EXTENSION_COUNT);
	if (!GLFW_EXTENSIONS) {
		const char *error_description;
		glfwGetError(&error_description);
		return RENGINE_ERROR_BACKEND;
	}

	*extension_count = 0;
	*extension_count += GLFW_EXTENSION_COUNT;

	if (context->info->debug) {
		*extension_count += DEBUG_EXTENSION_COUNT;
	}

	*extensions = malloc(*extension_count * sizeof(const char *));
	if (!*extensions) {
		return RENGINE_ERROR_MEMORY;
	}

	u32 i = 0;

	for (u32 j = 0; j < GLFW_EXTENSION_COUNT; ++j)
		(*extensions)[i++] = GLFW_EXTENSIONS[j];

	for (u32 j = 0; j < DEBUG_EXTENSION_COUNT; ++j)
		(*extensions)[i++] = DEBUG_EXTENSIONS[j];

	return RENGINE_SUCCESS;
}

u64 rengine_create_instance(struct rengine_context *context) {
	if (!glfwInit())
		return RENGINE_ERROR_BACKEND;


	const char **extensions;
	u32 extension_count;
	create_instance_extensions(context, &extensions, &extension_count);

	VkApplicationInfo application_info = {0};
	application_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pApplicationName   = context->info->name;
	application_info.applicationVersion = context->info->version;
	application_info.pEngineName        = ENGINE_NAME;
	application_info.engineVersion      = ENGINE_VERSION;
	application_info.apiVersion         = VK_API_VERSION_1_3;

	VkInstanceCreateInfo instance_create_info = {0};
	instance_create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pApplicationInfo        = &application_info;
	instance_create_info.enabledExtensionCount   = extension_count;
	instance_create_info.ppEnabledExtensionNames = extensions;

	if (context->info->debug & DEBUG_VALIDATION) {
		instance_create_info.pNext = &debug_messenger_create_info;
		instance_create_info.enabledLayerCount       = VALIDATION_LAYER_COUNT;
		instance_create_info.ppEnabledLayerNames     = VALIDATION_LAYERS;
	}

	VkResult result = vkCreateInstance(&instance_create_info, NULL, &context->instance.instance);
	free(extensions);

	if (result != VK_SUCCESS)
		return RENGINE_ERROR_BACKEND;

	if (context->info->debug & DEBUG_VALIDATION)
		return create_vulkan_debug_messenger(context);

	return RENGINE_SUCCESS;
}

void rengine_destroy_instance(struct rengine_context *context) {
	destroy_vulkan_debug_messenger(context);

	if (context->instance.instance != VK_NULL_HANDLE)
		vkDestroyInstance(context->instance.instance, NULL);
	
	glfwTerminate();
}
