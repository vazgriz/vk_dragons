#pragma once
#include <vector>
#include "Renderer.h"
#include "ProgramUtilities.h"

class UniformBuffer {
public:
	UniformBuffer(Renderer& renderer, size_t size, VkDescriptorSetLayout layout);
	~UniformBuffer();

	char* GetData();
	void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t firstSet);

private:
	Renderer& renderer;
	size_t size;
	VkDescriptorSetLayout layout;

	VkDescriptorPool pool;
	VkDescriptorSet set;
	Buffer buffer;

	void CreatePool();
	void CreateSet();
};