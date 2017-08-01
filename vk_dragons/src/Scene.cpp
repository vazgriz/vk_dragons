#include "Scene.h"

Scene::Scene(GLFWwindow* window, uint32_t width, uint32_t height)
	: renderer(window, width, height),
	dragon(renderer),
	suzanne(renderer),
	plane(renderer),
	dragonColor(renderer),
	skybox(renderer),
	camera(45.0f) {

	dragon.Init("resources/dragon.obj");
	suzanne.Init("resources/suzanne.obj");
	plane.Init("resources/plane.obj");

	dragonColor.Init("resources/dragon_texture_color.png");
	skybox.InitCubemap("resources/cubemap/cubemap");

	UploadResources();

	CreateCommandBuffers();
	CreateDescriptorSetLayout();
	CreateUniformBuffer();
}

Scene::~Scene() {
	vkDestroyDescriptorSetLayout(renderer.device, descriptorSetLayout, nullptr);
	vkDestroyBuffer(renderer.device, uniformBuffer.buffer, nullptr);
}

void Scene::UploadResources() {
	VkCommandBuffer commandBuffer = renderer.GetSingleUseCommandBuffer();

	dragon.UploadData(commandBuffer);
	suzanne.UploadData(commandBuffer);
	plane.UploadData(commandBuffer);

	dragonColor.UploadData(commandBuffer);

	renderer.SubmitCommandBuffer(commandBuffer);

	dragon.DestroyStaging();
	suzanne.DestroyStaging();
	plane.DestroyStaging();

	dragonColor.DestroyStaging();

	renderer.memory->hostAllocator->Reset();
}

void Scene::UpdateUniform() {
	char* ptr = reinterpret_cast<char*>(renderer.memory->hostMapping) + uniformBuffer.offset;
	Uniform& uniform = *(reinterpret_cast<Uniform*>(ptr));
	uniform.camera = camera.GetView() * camera.GetProjection();
}

void Scene::Update() {
	UpdateUniform();
}

void Scene::Render() {
	renderer.Render(commandBuffers);
}

void Scene::Resize(uint32_t width, uint32_t height) {
	renderer.Resize(width, height);
	CreateCommandBuffers();
}

void Scene::CreateCommandBuffers() {
	if (commandBuffers.size() > 0) vkFreeCommandBuffers(renderer.device, renderer.commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	commandBuffers.clear();
	commandBuffers.resize(renderer.swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = renderer.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(renderer.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers!");
	}

	for (size_t i = 0; i < commandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderer.mainRenderPass;
		renderPassInfo.framebuffer = renderer.swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = renderer.swapChainExtent;

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer!");
		}
	}
}

void Scene::CreateDescriptorSetLayout() {
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	if (vkCreateDescriptorSetLayout(renderer.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout!");
	}
}

void Scene::CreateUniformBuffer() {
	VkDeviceSize size = sizeof(Uniform);
	uniformBuffer = CreateBuffer(renderer.device, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, *renderer.memory->hostAllocator);
}