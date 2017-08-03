#include "ProgramUtilities.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <lodepng/lodepng.h>

std::string loadStringFromFile(const std::string & filename) {
	std::ifstream in;
	// Open a stream to the file.
	in.open(filename.c_str());
	if (!in) {
		std::cerr << filename + " is not a valid file." << std::endl;
		return "";
	}
	std::stringstream buffer;
	// Read the stream in a buffer.
	buffer << in.rdbuf();
	// Create a string based on the content of the buffer.
	std::string line = buffer.str();
	in.close();
	return line;
}

void flipImage(std::vector<unsigned char> & image, const int width, const int height){
	// Compute the number of components per pixel.
	int components = static_cast<int>(image.size() / (width * height));
	// The width in bytes.
	int widthInBytes = width * components;
	int halfHeight = height/2;

	// For each line of the first half, we swap it with the mirroring line, starting from the end of the image.
	for(int h=0; h < halfHeight; h++){
		std::swap_ranges(image.begin() + h * widthInBytes, image.begin() + (h+1) * widthInBytes , image.begin() + (height - h - 1) * widthInBytes);
	}
}

std::vector<char> loadFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

Buffer CreateBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, Allocator& allocator) {
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer buffer;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	Allocation alloc = allocator.Alloc(size, memRequirements.alignment);

	vkBindBufferMemory(device, buffer, alloc.memory, alloc.offset);

	return { buffer, alloc.size, alloc.offset };
}

VkShaderModule CreateShaderModule(VkDevice device, const std::string& filename) {
	std::vector<char> code = loadFile(filename);

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

Image CreateImage(VkDevice device, Allocator& allocator, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arrayLevels, VkImageUsageFlags usage, VkImageCreateFlags flags) {
	VkImageCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.imageType = VK_IMAGE_TYPE_2D;
	info.format = format;
	info.extent.width = width;
	info.extent.height = height;
	info.extent.depth = 1;
	info.mipLevels = mipLevels;
	info.arrayLayers = arrayLevels;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.usage = usage;
	info.flags = flags;

	VkImage image;
	if (vkCreateImage(device, &info, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	Allocation alloc = allocator.Alloc(memRequirements.size, memRequirements.alignment);

	vkBindImageMemory(device, image, alloc.memory, alloc.offset);

	return { image, alloc.size, alloc.offset };
}

VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect, VkImageViewType viewType, uint32_t mipLevels, uint32_t arrayLayers) {
	VkImageViewCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.image = image;
	info.format = format;
	info.viewType = viewType;
	info.components.r = VK_COMPONENT_SWIZZLE_R;
	info.components.g = VK_COMPONENT_SWIZZLE_G;
	info.components.b = VK_COMPONENT_SWIZZLE_B;
	info.components.a = VK_COMPONENT_SWIZZLE_A;
	info.subresourceRange.aspectMask = aspect;
	info.subresourceRange.baseArrayLayer = 0;
	info.subresourceRange.layerCount = arrayLayers;
	info.subresourceRange.baseMipLevel = 0;
	info.subresourceRange.levelCount = mipLevels;

	VkImageView imageView;
	if (vkCreateImageView(device, &info, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create image view!");
	}

	return imageView;
}