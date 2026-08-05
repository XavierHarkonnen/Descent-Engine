#include <descent/rengine.h>
#include "debug.h"

#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h>

#include <stdio.h>
#include <string.h>

#include <descent/type/core.h>

#include "context.h"


const char *rengine_result_string(u64 result) {
	switch(result) {
		case RENGINE_SUCCESS:           return "RENGINE_SUCCESS";
		case RENGINE_ERROR_INVALID:     return "RENGINE_ERROR_INVALID";
		case RENGINE_ERROR_NULL:        return "RENGINE_ERROR_NULL";
		case RENGINE_ERROR_INIT:        return "RENGINE_ERROR_INIT";
		case RENGINE_ERROR_MEMORY:      return "RENGINE_ERROR_MEMORY";
		case RENGINE_ERROR_THREAD:      return "RENGINE_ERROR_THREAD";
		case RENGINE_ERROR_BACKEND:     return "RENGINE_ERROR_BACKEND";
		case RENGINE_ERROR_UNSUPPORTED: return "RENGINE_ERROR_UNSUPPORTED";
		default:                        return "Unknown Error Code";
	}
}


static void print_separator(void) {
	puts("============================================================");
}


static const char *bool_string(bool value) {
	return value ? "true" : "false";
}


static const char *queue_flags_string(u32 flags) {
	static char buffer[128];

	buffer[0] = '\0';

	if (flags & VK_QUEUE_GRAPHICS_BIT)
		strcat(buffer, "GRAPHICS|");

	if (flags & VK_QUEUE_COMPUTE_BIT)
		strcat(buffer, "COMPUTE|");

	if (flags & VK_QUEUE_TRANSFER_BIT)
		strcat(buffer, "TRANSFER|");

	if (flags & VK_QUEUE_SPARSE_BINDING_BIT)
		strcat(buffer, "SPARSE|");

	char *end = strrchr(buffer, '|');
	if (!end)
		return "NONE";

	*end = '\0';
	return buffer;
}


static void print_queue_family(const char *name, const struct queue_family *family) {
	printf("  %s Queues\n", name);
	printf("    index            : %u\n", family->index);
	printf("    count            : %u\n", family->count);
	printf("    flags            : %s\n", queue_flags_string(family->flags));
	
	for (u32 i = 0; i < family->count; ++i) {
		printf("    Queue %u\n", i);
		printf("      handle         : %p\n", (void *)family->queues[i]);
		printf("      priority       : %.2f\n", (f64) family->priorities[i]);
	}
}

static void print_instance(const struct instance *instance) {
	print_separator();
	printf("Instance\n");
	print_separator();
	printf("VkInstance\n");
	printf("  handle             : %p\n", (void *)instance->instance);
	puts("");
	printf("Debug Messenger\n");
	printf("  handle             : %p\n", (void *)instance->debug_messenger);
}

static void print_physical_device(const struct device *device) {
	const VkPhysicalDeviceProperties *properties = &device->physical.properties.properties;
	const VkPhysicalDeviceFeatures *features = &device->physical.features.features;

	print_separator();
	printf("Physical Device\n");
	print_separator();

	printf("General\n");
	printf("  handle             : %p\n", (void *)device->physical.device);
	printf("  name               : %s\n", properties->deviceName);
	printf("  vendor id          : 0x%04x\n", properties->vendorID);
	printf("  device id          : 0x%04x\n", properties->deviceID);
	printf("  device type        : %s\n", string_VkPhysicalDeviceType(properties->deviceType));
	puts("");
	printf("API\n");
	printf(
		"  api version        : %u.%u.%u\n",
		VK_VERSION_MAJOR(properties->apiVersion),
		VK_VERSION_MINOR(properties->apiVersion),
		VK_VERSION_PATCH(properties->apiVersion)
	);
	printf(
		"  driver version     : %u.%u.%u\n",
		VK_VERSION_MAJOR(properties->driverVersion),
		VK_VERSION_MINOR(properties->driverVersion),
		VK_VERSION_PATCH(properties->driverVersion)
	);
	puts("");
	printf("Limits\n");
	printf("  max image 2D       : %u\n", properties->limits.maxImageDimension2D);
	printf("  max push constants : %u\n", properties->limits.maxPushConstantsSize);
	printf("  max bound sets     : %u\n", properties->limits.maxBoundDescriptorSets);
	printf("  max samplers       : %u\n", properties->limits.maxPerStageDescriptorSamplers);
	puts("");
	printf("Features\n");
	printf("  geometry shader    : %s\n", bool_string(features->geometryShader));
	printf("  tessellation       : %s\n", bool_string(features->tessellationShader));
	printf("  sampler anisotropy : %s\n", bool_string(features->samplerAnisotropy));
	printf("  multi viewport     : %s\n", bool_string(features->multiViewport));
	printf("  shader int64       : %s\n", bool_string(features->shaderInt64));
	printf("  fill mode nonsolid : %s\n", bool_string(features->fillModeNonSolid));
}

static void print_logical_device(const struct device *device) {
	print_separator();
	printf("Logical Device\n");
	print_separator();

	printf("VkDevice\n");
	printf("  handle             : %p\n", (void *)device->device);

	printf("\nQueues\n");

	print_queue_family("Graphics", &device->queues.graphics);
	print_queue_family("Compute", &device->queues.compute);
	print_queue_family("Transfer", &device->queues.transfer);
}

static void print_window(const struct window *window) {
	print_separator();
	printf("Window\n");
	print_separator();

	printf("  glfw window        : %p\n", (void *)window->window);
	printf("  handle             : %p\n", (void *)window->surface);
}

static void print_swapchain(const struct swapchain *swapchain) {
	print_separator();
	printf("Swapchain\n");
	print_separator();
	printf("Swapchain\n");
	printf("  handle             : %p\n", (void *)swapchain->swapchain);
	printf("  old handle         : %p\n", (void *)swapchain->swapchain_old);
	puts("");
	printf("Swapchain Info\n");
	printf("  images             : %p\n", (void *)swapchain->images);
	printf("  image count        : %u\n", swapchain->image_count);
	printf("  extent             : %ux%u\n", swapchain->extent.width, swapchain->extent.height);
	printf("  format             : %s\n", string_VkFormat(swapchain->format));
	printf("  color space        : %s\n", string_VkColorSpaceKHR(swapchain->color_space));
	printf("  present mode       : %s\n", string_VkPresentModeKHR(swapchain->present_mode));
	printf("  transform          : %s\n", string_VkSurfaceTransformFlagBitsKHR(swapchain->transform));
}

void rengine_context_print(struct rengine_context *context) {
	if (!context) {
		printf("Context pointer is NULL\n");
		return;
	}

	print_separator();
	printf("Rengine Context\n");
	print_separator();

	printf("Context\n");
	printf("  initialized        : %s\n", bool_string(context->initialized));

	printf("  context ptr        : %p\n", (void *)context);
	printf("  create info ptr    : %p\n", (void *)context->info);

	print_instance(&context->instance);
	print_window(&context->window);
	print_physical_device(&context->device);
	print_logical_device(&context->device);
	print_swapchain(&context->swapchain);

	print_separator();
}