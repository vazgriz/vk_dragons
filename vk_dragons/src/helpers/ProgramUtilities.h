#ifndef ProgramUtilities_h
#define ProgramUtilities_h

#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "../Renderer.h"

struct Buffer {
	VkBuffer buffer;
	size_t size;
	size_t offset;
};

struct Image {
	VkImage image;
	size_t size;
	size_t offset;
	uint32_t type;
};

/// Return the content of a text file at the given path, as a string.
std::string loadStringFromFile(const std::string & path);

/// Flip an image vertically (line by line).
void flipImage(std::vector<unsigned char> & image, const int width, const int height);

/// Load a file into a vector<char>. From https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules
std::vector<char> loadFile(const std::string& filename);

Buffer CreateBuffer(Renderer& renderer, VkDeviceSize size, VkBufferUsageFlags usage);

Buffer CreateHostBuffer(Renderer& renderer, VkDeviceSize size, VkBufferUsageFlags usage);

Buffer CopyBuffer(Renderer& renderer, VkCommandBuffer commandBuffer, VkBuffer destBuffer, const void* source, size_t size);

VkShaderModule CreateShaderModule(VkDevice device, const std::string& filename);

Image CreateImage(Renderer& renderer, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arrayLevels, VkImageUsageFlags usage, VkImageCreateFlags flags);

VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect, VkImageViewType viewType, uint32_t mipLevels, uint32_t arrayLayers);

bool hasStencilComponent(VkFormat format);

void Transition(VkCommandBuffer commandBuffer, VkFormat format, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t arrayLayers);

VkFramebuffer CreateFramebuffer(Renderer& renderer, VkRenderPass renderPass, uint32_t width, uint32_t height, std::vector<VkImageView>& imageViews);

#endif
