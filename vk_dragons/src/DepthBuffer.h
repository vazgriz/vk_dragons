#pragma once
#include "Renderer.h"
#include "ProgramUtilities.h"

class DepthBuffer {
public:
	DepthBuffer(Renderer& renderer);

	void Init(uint32_t width, uint32_t height);
	void Cleanup();

	VkImage image;
	VkImageView imageView;
	VkFormat format;

private:
	Renderer& renderer;

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
};