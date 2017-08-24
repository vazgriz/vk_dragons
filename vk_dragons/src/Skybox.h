#pragma once
#include "Renderer.h"
#include "ProgramUtilities.h"

class Skybox {
public:
	Skybox(Renderer& renderr);
	~Skybox();
	void UploadData(VkCommandBuffer commandBuffer);
	void DestroyStaging();
	void Draw(VkCommandBuffer commandBuffer);

	static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

private:
	Renderer& renderer;

	Buffer vertexBuffer;
	Buffer indexBuffer;
	Buffer vertexStagingBuffer;
	Buffer indexStagingBuffer;

	void Init();
};