#include "Material.h"

Material::Material(Renderer& renderer, VkSampler sampler, std::vector<std::shared_ptr<Texture>>& textures) : renderer(renderer) {
	this->sampler = sampler;

	for (auto& ptr : textures) {
		this->textures.push_back(ptr);
	}

	CreateLayout();
	CreatePool();
	CreateSet();
	WriteDescriptors();
}

Material::~Material() {
	vkDestroyDescriptorSetLayout(renderer.device, layout, nullptr);
	vkDestroyDescriptorPool(renderer.device, pool, nullptr);
}

void Material::Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t firstSet) {
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, firstSet, 1, &set, 0, nullptr);
}

void Material::CreateLayout() {
	std::vector<VkDescriptorSetLayoutBinding> bindings(textures.size());

	for (size_t i = 0; i < textures.size(); i++) {
		bindings[i].binding = static_cast<uint32_t>(i);
		bindings[i].descriptorCount = 1;
		bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	}

	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = static_cast<uint32_t>(textures.size());
	info.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(renderer.device, &info, nullptr, &layout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create texture set layout!");
	}
}

void Material::CreatePool() {
	VkDescriptorPoolSize size = {};
	size.descriptorCount = static_cast<uint32_t>(textures.size());
	size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	VkDescriptorPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.maxSets = 1;
	info.poolSizeCount = 1;
	info.pPoolSizes = &size;
	
	if (vkCreateDescriptorPool(renderer.device, &info, nullptr, &pool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool!");
	}
}

void Material::CreateSet() {
	VkDescriptorSetAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.descriptorPool = pool;
	info.descriptorSetCount = 1;
	info.pSetLayouts = &layout;

	if (vkAllocateDescriptorSets(renderer.device, &info, &set) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate texture set!");
	}
}

void Material::WriteDescriptors() {
	std::vector<VkDescriptorImageInfo> imageInfos(textures.size());
	std::vector<VkWriteDescriptorSet> writes(textures.size());

	for (size_t i = 0; i < textures.size(); i++) {
		imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfos[i].imageView = textures[i]->imageView;
		imageInfos[i].sampler = sampler;

		writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[i].dstSet = set;
		writes[i].dstArrayElement = 0;
		writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[i].descriptorCount = 1;
		writes[i].pImageInfo = &imageInfos[i];
		writes[i].dstBinding = static_cast<uint32_t>(i);
	}

	vkUpdateDescriptorSets(renderer.device, static_cast<uint32_t>(textures.size()), writes.data(), 0, nullptr);
}