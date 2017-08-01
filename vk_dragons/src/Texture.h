#pragma once
#include <string>
#include <vector>
#include "Renderer.h"
#include "glm/glm.hpp"

class Texture {
public:
	Texture(Renderer& renderer);
	~Texture();

	void Init(const std::string& filename);
	void UploadData(VkCommandBuffer commandBuffer);

private:
	Renderer& renderer;
	VkImage image;
	uint32_t width;
	uint32_t height;
	std::vector<unsigned char> data;
	std::vector<glm::vec2> mipChain;

	void CreateImage();
	void CalulateMipChain();
};