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

Buffer CreateBuffer(Renderer& renderer, VkDeviceSize size, VkBufferUsageFlags usage) {
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer buffer;

	if (vkCreateBuffer(renderer.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(renderer.device, buffer, &memRequirements);

	Allocator& allocator = renderer.memory->GetDeviceAllocator(memRequirements);
	Allocation alloc = allocator.Alloc(size, memRequirements.alignment);

	vkBindBufferMemory(renderer.device, buffer, alloc.memory, alloc.offset);

	return { buffer, alloc.size, alloc.offset };
}

Buffer CreateHostBuffer(Renderer& renderer, VkDeviceSize size, VkBufferUsageFlags usage) {
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer buffer;

	if (vkCreateBuffer(renderer.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(renderer.device, buffer, &memRequirements);

	Allocator& allocator = *renderer.memory->hostAllocator;
	Allocation alloc = allocator.Alloc(size, memRequirements.alignment);

	vkBindBufferMemory(renderer.device, buffer, alloc.memory, alloc.offset);

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

Image CreateImage(Renderer& renderer, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arrayLevels, VkImageUsageFlags usage, VkImageCreateFlags flags) {
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
	info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.usage = usage;
	info.flags = flags;

	VkImage image;
	if (vkCreateImage(renderer.device, &info, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(renderer.device, image, &memRequirements);

	Allocator& allocator = renderer.memory->GetDeviceAllocator(memRequirements);
	Allocation alloc = allocator.Alloc(memRequirements.size, memRequirements.alignment);

	vkBindImageMemory(renderer.device, image, alloc.memory, alloc.offset);

	return { image, alloc.size, alloc.offset, allocator.GetType() };
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

bool hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}


void Transition(VkCommandBuffer commandBuffer, VkFormat format, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t arrayLayers) {
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	
	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = arrayLayers;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL || newLayout == VK_IMAGE_LAYOUT_GENERAL)) {
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	else if ((oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL || oldLayout == VK_IMAGE_LAYOUT_GENERAL) && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	else {
		throw std::invalid_argument("Unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}