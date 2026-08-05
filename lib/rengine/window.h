#ifndef DESCENT_SOURCE_RENGINE_WINDOW_H
#define DESCENT_SOURCE_RENGINE_WINDOW_H

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include <descent/type/core.h>


struct window {
	GLFWwindow *window;
	VkSurfaceKHR surface;
};

struct rengine_context;


u64 rengine_create_window(struct rengine_context *context);

void rengine_destroy_window(struct rengine_context *context);

#endif