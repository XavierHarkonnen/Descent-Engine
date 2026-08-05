#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vulkan/vulkan_core.h>

#include <descent/type/core.h>

#include "context.h"
#include "descent/rengine.h"

struct pipeline {
	VkPipeline pipeline;
};

struct shader {
	VkShaderModule module;
	u64 size;
	u32 *data;
};

// File

u64 rengine_create_shader(struct rengine_context *context, struct shader *shader, const char *path) {
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		return RENGINE_ERROR_INVALID;
	}

	struct stat stat;
	if (fstat(fd, &stat)) {
		return RENGINE_ERROR_BACKEND;
	}

	if (stat.st_size <= 0) {
		return RENGINE_ERROR_INVALID;
	}

	shader->size = (u64) stat.st_size;
	shader->data = mmap(NULL, shader->size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (shader->data == MAP_FAILED) {
		shader->size = 0;
		shader->data = NULL;
		return RENGINE_ERROR_MEMORY;
	}

	close(fd);

	VkShaderModuleCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = shader->size;
	create_info.pCode = shader->data;

	if (vkCreateShaderModule(context->device.device, &create_info, NULL, &shader->module) != VK_SUCCESS) {
		return RENGINE_ERROR_BACKEND;
	}

	return RENGINE_SUCCESS;
}

u64 rengine_destroy_shader(struct rengine_context *context, struct shader *shader) {
	munmap(shader->data, shader->size);

	return RENGINE_SUCCESS;
}