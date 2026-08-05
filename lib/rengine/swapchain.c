#include "swapchain.h"

#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>

#include <descent/type/core.h>

#include "context.h"


#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define CLAMP(x, min, max) (MAX(MIN((x), (max)), (min)))


static inline u64 choose_surface_format(struct rengine_context *context) {
	VkResult result;

	u32 format_count;
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(context->device.physical.device, context->window.surface, &format_count, NULL);
	if (result != VK_SUCCESS) {
		return RENGINE_ERROR_BACKEND;
	}

	if (format_count == 0) {
		return RENGINE_ERROR_BACKEND;
	}

	VkSurfaceFormatKHR *formats = malloc(format_count * sizeof(VkSurfaceFormatKHR));
	if (!formats) {
		return RENGINE_ERROR_MEMORY;
	}

	result = vkGetPhysicalDeviceSurfaceFormatsKHR(context->device.physical.device, context->window.surface, &format_count, formats);
	if (result != VK_SUCCESS) {
		free(formats);
		return RENGINE_ERROR_BACKEND;
	}

	context->swapchain.format = formats[0].format;
	context->swapchain.color_space = formats[0].colorSpace;
	for (u32 i = 0; i < format_count; ++i) {
		if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			context->swapchain.format = formats[i].format;
			context->swapchain.color_space = formats[i].colorSpace;
			break;
		}
	}

	free(formats);

	return RENGINE_SUCCESS;
}


static inline u64 choose_present_mode(struct rengine_context *context) {
	VkResult result;

	u32 present_mode_count;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(context->device.physical.device, context->window.surface, &present_mode_count, NULL);
	if (result != VK_SUCCESS) {
		return RENGINE_ERROR_BACKEND;
	}

	VkPresentModeKHR *present_modes = malloc(present_mode_count * sizeof(VkPresentModeKHR));
	if (!present_modes) {
		return RENGINE_ERROR_MEMORY;
	}

	result = vkGetPhysicalDeviceSurfacePresentModesKHR(context->device.physical.device, context->window.surface, &present_mode_count, present_modes);
	if (result != VK_SUCCESS) {
		free(present_modes);
		return RENGINE_ERROR_BACKEND;
	}

	context->swapchain.present_mode = VK_PRESENT_MODE_FIFO_KHR;
	for (u32 i = 0; i < present_mode_count; ++i) {
		if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			context->swapchain.present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
	}

	free(present_modes);

	return RENGINE_SUCCESS;
}


static inline u64 get_capabilities(struct rengine_context *context) {
	VkSurfaceCapabilitiesKHR capabilities;
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->device.physical.device, context->window.surface, &capabilities) != VK_SUCCESS)
		return RENGINE_ERROR_BACKEND;

	if (capabilities.currentExtent.width != U32_MAX) {
		context->swapchain.extent = capabilities.currentExtent;
	} else {
		int width, height;
		glfwGetFramebufferSize(context->window.window, &width, &height);
		
		 context->swapchain.extent = (VkExtent2D) {
			.width  = CLAMP((u32) width,  capabilities.minImageExtent.width,  capabilities.maxImageExtent.width),
			.height = CLAMP((u32) height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
		};
	}

	context->swapchain.image_count = MIN(capabilities.minImageCount + 1, capabilities.maxImageCount ? capabilities.maxImageCount : U32_MAX);
	context->swapchain.transform = capabilities.currentTransform;

	return RENGINE_SUCCESS;
}


u64 rengine_create_swapchain(struct rengine_context *context) {
	u64 result;

	result = choose_surface_format(context);
	if (result) return result;
	
	result = choose_present_mode(context);
	if (result) return result;

	result = get_capabilities(context);
	if (result) return result;

	VkSwapchainCreateInfoKHR create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = context->window.surface;
	create_info.minImageCount = context->swapchain.image_count;
	create_info.imageFormat = context->swapchain.format;
	create_info.imageColorSpace = context->swapchain.color_space;
	create_info.imageExtent = context->swapchain.extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.preTransform = context->swapchain.transform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = context->swapchain.present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(context->device.device, &create_info, NULL, &context->swapchain.swapchain) != VK_SUCCESS)
		return RENGINE_ERROR_BACKEND;

	if (vkGetSwapchainImagesKHR(context->device.device, context->swapchain.swapchain, &context->swapchain.image_count, NULL))
		return RENGINE_ERROR_BACKEND;

	context->swapchain.images = malloc(context->swapchain.image_count * sizeof(*context->swapchain.images));
	if (!context->swapchain.images)
		return RENGINE_ERROR_MEMORY;

	context->swapchain.views = malloc(context->swapchain.image_count * sizeof(*context->swapchain.views));
	if (!context->swapchain.views)
		return RENGINE_ERROR_MEMORY;
	
	if (vkGetSwapchainImagesKHR(context->device.device, context->swapchain.swapchain, &context->swapchain.image_count, context->swapchain.images))
		return RENGINE_ERROR_BACKEND;

	for (u64 i = 0; i < context->swapchain.image_count; ++i) {
		VkImageViewCreateInfo view_create_info = {0};
		view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_create_info.image = context->swapchain.images[i];
		view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_create_info.format = context->swapchain.format;
		view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_create_info.subresourceRange.baseMipLevel = 0;
		view_create_info.subresourceRange.levelCount = 1;
		view_create_info.subresourceRange.baseArrayLayer = 0;
		view_create_info.subresourceRange.layerCount = 1;

		if (vkCreateImageView(context->device.device, &view_create_info, NULL, &context->swapchain.views[i]) != VK_SUCCESS)
			return RENGINE_ERROR_BACKEND;
	}

	return RENGINE_SUCCESS;
}


void rengine_destroy_swapchain(struct rengine_context *context) {
	if (context->swapchain.swapchain != VK_NULL_HANDLE)
		vkDestroySwapchainKHR(context->device.device, context->swapchain.swapchain, NULL);

	if (context->swapchain.images)
		free(context->swapchain.images);

	if (context->swapchain.views) {
		for (u64 i = 0; i < context->swapchain.image_count; ++i) {
			vkDestroyImageView(context->device.device, context->swapchain.views[i], NULL);
		}

		context->swapchain.views = NULL;
	}
	
}