#include "Texture.h"
#include <iostream>
#include "lodepng\lodepng.h"
#include "ProgramUtilities.h"
#include <stdexcept>

Texture::Texture(Renderer& renderer) : renderer(renderer) {

}

Texture::~Texture() {
	Cleanup();
}

void Texture::Cleanup() {
	renderer.memory->GetDeviceAllocator(image.type).Pop();
	if (image.image != VK_NULL_HANDLE) {
		vkDestroyImage(renderer.device, image.image, nullptr);
		vkDestroyImageView(renderer.device, imageView, nullptr);
		image.image = VK_NULL_HANDLE;
	}
}

void Texture::DestroyStaging() {
	for (auto& stagingBuffer : stagingBuffers) {
		vkDestroyBuffer(renderer.device, stagingBuffer.buffer, nullptr);
	}
}

void Texture::Init(const std::string& filename) {
	LoadImages(std::vector<std::string>{ filename });
	CalulateMipChain();
	format = VK_FORMAT_R8G8B8A8_UNORM;

	image = CreateImage(renderer,
		format,
		width, height,
		mipLevels, arrayLayers,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		0);
	imageView = CreateImageView(renderer.device, image.image, format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, mipLevels, arrayLayers);
}

void Texture::InitCubemap(const std::string& filenameRoot) {
	//to create a cubemap, there must 6 layers in an image
	//the layers correspond to +X, -X, +Y, -Y, +Z, -Z
	//Vulkan uses Y-down convention, so +Y corresponds to down
	std::vector<std::string> filenames = {
		filenameRoot + "_r.png",
		filenameRoot + "_l.png",
		filenameRoot + "_d.png",
		filenameRoot + "_u.png",
		filenameRoot + "_b.png",
		filenameRoot + "_f.png",
	};

	LoadImages(filenames);
	CalulateMipChain();
	format = VK_FORMAT_R8G8B8A8_UNORM;

	image = CreateImage(renderer,
		format,
		width, height,
		mipLevels, arrayLayers,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
	imageView = CreateImageView(renderer.device, image.image, format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_CUBE, mipLevels, arrayLayers);
}

void Texture::Init(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage) {
	mipLevels = 1;
	arrayLayers = 1;
	this->format = format;
	this->width = width;
	this->height = height;

	image = CreateImage(renderer,
		format,
		width, height,
		mipLevels, arrayLayers,
		usage, 0);
	imageView = CreateImageView(renderer.device, image.image, format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, mipLevels, arrayLayers);
}

void Texture::LoadImages(std::vector<std::string>& filenames) {
	data.resize(filenames.size());
	unsigned int width, height;

	for (size_t i = 0; i < filenames.size(); i++) {
		unsigned int error = lodepng::decode(data[i], width, height, filenames[i]);
		if (error != 0) {
			std::cerr << "Unable to load the texture at path " << filenames[i] << std::endl;
		}
		flipImage(data[i], width, height);
	}

	this->width = static_cast<uint32_t>(width);
	this->height = static_cast<uint32_t>(height);
	arrayLayers = static_cast<uint32_t>(data.size());
}

uint32_t Texture::GetWidth() {
	return width;
}

uint32_t Texture::GetHeight() {
	return height;
}

void Texture::UploadData(VkCommandBuffer commandBuffer) {
	Transition(commandBuffer, VK_FORMAT_R8G8B8A8_UNORM, image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, mipLevels, arrayLayers);

	stagingBuffers.resize(data.size());
	for (size_t i = 0; i < data.size(); i++) {
		stagingBuffers[i] = CreateHostBuffer(renderer, data[i].size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		char* dest = static_cast<char*>(renderer.memory->GetMapping(stagingBuffers[i].memory)) + stagingBuffers[i].offset;
		memcpy(dest, data[i].data(), data[i].size());

		VkBufferImageCopy copy = {};
		copy.bufferOffset = 0;
		copy.bufferRowLength = 0;
		copy.bufferImageHeight = 0;
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.imageSubresource.mipLevel = 0;
		copy.imageSubresource.baseArrayLayer = static_cast<uint32_t>(i);
		copy.imageSubresource.layerCount = 1;
		copy.imageOffset = { 0, 0, 0 };
		copy.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(commandBuffer, stagingBuffers[i].buffer, image.image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy);
	}

	GenerateMipChain(commandBuffer);

	Transition(commandBuffer, VK_FORMAT_R8G8B8A8_UNORM, image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels, arrayLayers);
}

void Texture::CalulateMipChain() {
	uint32_t w = width;
	uint32_t h = height;

	while (w != 1 && h != 1) {
		mipChain.push_back({ w, h });
		if (w > 1) w /= 2;
		if (h > 1) h /= 2;
	}

	mipLevels = static_cast<uint32_t>(mipChain.size());
}

void Texture::GenerateMipChain(VkCommandBuffer commandBuffer) {
	if (mipChain.size() == 1) return;

	//start from i == 1, blit level (i - 1) to (i)
	for (size_t i = 1; i < mipChain.size(); i++) {
		//blit each layer separately
		for (size_t j = 0; j < data.size(); j++) {
			glm::vec2 src = mipChain[i - 1];
			glm::vec2 dst = mipChain[i];
			int32_t srcW = static_cast<int32_t>(src.x);
			int32_t srcH = static_cast<int32_t>(src.y);
			int32_t dstW = static_cast<int32_t>(dst.x);
			int32_t dstH = static_cast<int32_t>(dst.y);

			VkImageBlit blit = {};
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.baseArrayLayer = static_cast<uint32_t>(j);;
			blit.srcSubresource.layerCount = 1;
			blit.srcSubresource.mipLevel = static_cast<uint32_t>(i - 1);
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { srcW, srcH, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.baseArrayLayer = static_cast<uint32_t>(j);
			blit.dstSubresource.layerCount = 1;
			blit.dstSubresource.mipLevel = static_cast<uint32_t>(i);
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { dstW, dstH, 1 };

			vkCmdBlitImage(commandBuffer,
				image.image, VK_IMAGE_LAYOUT_GENERAL,
				image.image, VK_IMAGE_LAYOUT_GENERAL,
				1, &blit,
				VK_FILTER_LINEAR
			);
		}
	}
}
