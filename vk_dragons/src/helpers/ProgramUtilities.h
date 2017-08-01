#ifndef ProgramUtilities_h
#define ProgramUtilities_h

#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "../Allocator.h"

struct Buffer {
	VkBuffer buffer;
	size_t size;
	size_t offset;
};

/// Return the content of a text file at the given path, as a string.
std::string loadStringFromFile(const std::string & path);

/// Flip an image vertically (line by line).
void flipImage(std::vector<unsigned char> & image, const int width, const int height);

/// Load a file into a vector<char>. From https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules
static std::vector<char> loadFile(const std::string& filename);

Buffer CreateBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, Allocator& allocator);

#endif
