#pragma once
#include <string>
#include <vector>
#include "Renderer.h"
#include "glm/glm.hpp"
#include "ProgramUtilities.h"

class Texture {
public:
	Texture(Renderer& renderer);
	~Texture();

	void Init(const std::string& filename, bool gammaSpace = false);
	void InitCubemap(const std::string& filenameRoot, bool gammaSpace = false);
	void Init(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
	void InitDepth(uint32_t width, uint32_t height, VkImageUsageFlags flags);
	void Cleanup();
	void UploadData(VkCommandBuffer commandBuffer);
	void DestroyStaging();

	uint32_t GetWidth();
	uint32_t GetHeight();

	Image image;
	VkImageView imageView;
	VkFormat format;

private:
	Renderer& renderer;
	uint32_t width;
	uint32_t height;
	std::vector<std::vector<unsigned char>> data;
	std::vector<glm::vec2> mipChain;
	std::vector<Buffer> stagingBuffers;
	uint32_t mipLevels;
	uint32_t arrayLayers;

	void LoadImages(std::vector<std::string>& filenames);
	void CalulateMipChain();
	void GenerateMipChain(VkCommandBuffer commandBuffer);
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
};