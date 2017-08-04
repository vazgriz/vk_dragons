#include "DepthBuffer.h"

DepthBuffer::DepthBuffer(Renderer& renderer) : renderer(renderer) {

}

void DepthBuffer::Init(uint32_t width, uint32_t height) {
	format = findDepthFormat();
	image = CreateImage(renderer.device, *renderer.memory->deviceAllocator,
		format, width, height,
		1, 1,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0).image;
	imageView = CreateImageView(renderer.device, image,
		format, VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_IMAGE_VIEW_TYPE_2D, 1, 1);
}

void DepthBuffer::Cleanup() {
	renderer.memory->deviceAllocator->Pop();
	vkDestroyImage(renderer.device, image, nullptr);
	vkDestroyImageView(renderer.device, imageView, nullptr);
}

VkFormat DepthBuffer::findSupportedFormat(const std::vector<VkFormat>& candidates, VkFormatFeatureFlags features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(renderer.physicalDevice, format, &props);

		if ((props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("Failed to find supported format!");
}

VkFormat DepthBuffer::findDepthFormat() {
	return findSupportedFormat(
	{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}