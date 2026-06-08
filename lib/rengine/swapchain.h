#ifndef DESCENT_SOURCE_SWAPCHAIN_H
#define DESCENT_SOURCE_SWAPCHAIN_H

#include <vulkan/vulkan_core.h>

#include <descent/types/core.h>


struct swapchain {
	VkSwapchainKHR swapchain;
	VkSwapchainKHR swapchain_old;

	u32 image_count;
	VkImage *images;
	VkImageView *views;

	VkExtent2D extent;
	VkFormat format;
	VkColorSpaceKHR color_space;
	VkPresentModeKHR present_mode;
	VkSurfaceTransformFlagBitsKHR transform;
};

struct rengine_context;


u64 rengine_create_swapchain(struct rengine_context *context);

void rengine_destroy_swapchain(struct rengine_context *context);

#endif
