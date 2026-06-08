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

#include "window.h"

#include <GLFW/glfw3.h>

#include <stddef.h>

#include <descent/rengine.h>
#include <descent/types/core.h>

#include "context.h"


#define MAXIMUM_WIDTH 32000
#define MAXIMUM_HEIGHT 32000


u64 rengine_create_window(struct rengine_context *context) {
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	if (context->info->width == 0 || context->info->height == 0)
		return RENGINE_ERROR_INVALID;

	if (context->info->width > MAXIMUM_WIDTH || context->info->height > MAXIMUM_HEIGHT)
		return RENGINE_ERROR_INVALID;

	context->window.window = glfwCreateWindow((int) context->info->width, (int) context->info->height, context->info->name, NULL, NULL);
	if (!context->window.window)
		return RENGINE_ERROR_BACKEND;

	VkResult result = glfwCreateWindowSurface(context->instance.instance, context->window.window, NULL, &context->window.surface);
	if (result != VK_SUCCESS)
		return RENGINE_ERROR_BACKEND;

	return RENGINE_SUCCESS;
}


void rengine_destroy_window(struct rengine_context *context) {
	if (context->window.surface != VK_NULL_HANDLE)
		vkDestroySurfaceKHR(context->instance.instance, context->window.surface, NULL);

	if (context->window.window != NULL)
		glfwDestroyWindow(context->window.window);
}
