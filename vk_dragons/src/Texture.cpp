#include "Texture.h"
#include <iostream>
#include "lodepng\lodepng.h"
#include "ProgramUtilities.h"
#include <stdexcept>

Texture::Texture(Renderer& renderer) : renderer(renderer) {

}

Texture::~Texture() {
	vkDestroyImage(renderer.device, image, nullptr);
}

void Texture::Init(const std::string& filename) {
	unsigned int width, height;
	unsigned int error = lodepng::decode(data, width, height, filename);
	if (error != 0) {
		std::cerr << "Unable to load the texture at path " << filename << std::endl;
	}
	flipImage(data, width, height);

	this->width = static_cast<uint32_t>(width);
	this->height = static_cast<uint32_t>(height);

	CalulateMipChain();
	CreateImage();
}

void Texture::CreateImage() {
	VkImageCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.imageType = VK_IMAGE_TYPE_2D;
	info.format = VK_FORMAT_R8G8B8A8_UNORM;
	info.extent.width = width;
	info.extent.height = height;
	info.extent.depth = 1;
	info.mipLevels = static_cast<uint32_t>(mipChain.size());
	info.arrayLayers = 1;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.samples = VK_SAMPLE_COUNT_1_BIT;

	if (vkCreateImage(renderer.device, &info, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create image!");
	}

}

void Texture::UploadData(VkCommandBuffer commandBuffer) {

}

void Texture::CalulateMipChain() {
	uint32_t w = width;
	uint32_t h = height;

	while (w != 1 && h != 1) {
		mipChain.push_back({ w, h });
		if (w > 1) w /= 2;
		if (h > 1) h /= 2;
	}
}
