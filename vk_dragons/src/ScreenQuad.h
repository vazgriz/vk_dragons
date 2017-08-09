#pragma once
#include "Renderer.h"
#include "ProgramUtilities.h"
#include <vector>

class ScreenQuad {
public:
	ScreenQuad(Renderer& renderer);
	~ScreenQuad();
	void Init();
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
};