#pragma once
#include "Renderer.h"
#include "ProgramUtilities.h"
#include <vector>
#include "StagingBuffer.h"

class ScreenQuad {
public:
	ScreenQuad(Renderer& renderer);
	~ScreenQuad();
	void UploadData(VkCommandBuffer commandBuffer, std::vector<std::unique_ptr<StagingBuffer>>& stagingBuffers);
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