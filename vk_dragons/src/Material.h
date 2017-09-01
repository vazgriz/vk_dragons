#pragma once
#include <vector>
#include <memory>
#include "Renderer.h"
#include "Texture.h"

//manages one descriptor pool and descriptor set for multiple textures
class Material {
public:
	Material(Renderer& renderer, VkSampler sampler, std::vector<std::shared_ptr<Texture>>& textures);
	~Material();

	void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t firstSet);

private:
	Renderer& renderer;
	std::vector<std::shared_ptr<Texture>> textures;
	VkSampler sampler;
	VkDescriptorSetLayout layout;
	VkDescriptorPool pool;
	VkDescriptorSet set;

	Material(const Material& other) = delete;
	Material& operator = (const Material& other) = delete;

	void CreateLayout();
	void CreatePool();
	void CreateSet();
	void WriteDescriptors();
};