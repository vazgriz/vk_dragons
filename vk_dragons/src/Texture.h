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

	void Init(const std::string& filename);
	void InitCubemap(const std::string& filenameRoot);
	void UploadData(VkCommandBuffer commandBuffer);
	void DestroyStaging();

	VkImage image;
	VkImageView imageView;

private:
	Renderer& renderer;
	uint32_t width;
	uint32_t height;
	std::vector<std::vector<unsigned char>> data;
	std::vector<glm::vec2> mipChain;
	std::vector<Buffer> stagingBuffers;
	bool isCubemap;

	void LoadImages(std::vector<std::string>& filenames);
	void CreateImage(VkFormat format, uint32_t mipLevels, uint32_t arrayLevels, VkImageUsageFlags usage, VkImageCreateFlags flags);
	void CalulateMipChain();
	void Transition(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);
	void GenerateMipChain(VkCommandBuffer commandBuffer);
	void CreateImageView(VkFormat format, VkImageAspectFlags aspect, VkImageViewType viewType, uint32_t mipLevels, uint32_t arrayLayers);
};