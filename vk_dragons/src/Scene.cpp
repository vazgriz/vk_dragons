#include "Scene.h"

Scene::Scene(GLFWwindow* window, uint32_t width, uint32_t height)
	: renderer(window, width, height),
	dragon(renderer),
	suzanne(renderer),
	plane(renderer),
	skybox(renderer),
	quad(renderer),
	dragonColor(renderer),
	dragonNormal(renderer),
	dragonEffects(renderer),
	suzanneColor(renderer),
	suzanneNormal(renderer),
	suzanneEffects(renderer),
	planeColor(renderer),
	planeNormal(renderer),
	planeEffects(renderer),
	skyboxColor(renderer),
	skyboxSmallColor(renderer),
	camera(45.0f, width, height),
	input(window, camera),
	lightDepth(renderer),
	boxBlur(renderer),
	depth(renderer) {

	time = 0.0f;
	camera.SetPosition(glm::vec3(0, 0, 1.0f));

	dragon.Init("resources/dragon.obj");
	suzanne.Init("resources/suzanne.obj");
	plane.Init("resources/plane.obj");
	skybox.Init();
	quad.Init();

	dragon.GetTransform().SetScale(glm::vec3(0.5f));
	dragon.GetTransform().SetPosition(glm::vec3(-0.1f, 0.0f, -0.25f));

	suzanne.GetTransform().SetScale(glm::vec3(0.25f));
	suzanne.GetTransform().SetPosition(glm::vec3(0.2f, 0, 0));

	plane.GetTransform().SetScale(glm::vec3(2.0f));
	plane.GetTransform().SetPosition(glm::vec3(0.0f, -0.35f, -0.5f));

	dragonColor.Init("resources/dragon_texture_color.png");
	dragonNormal.Init("resources/dragon_texture_normal.png");
	dragonEffects.Init("resources/dragon_texture_ao_specular_reflection.png");

	suzanneColor.Init("resources/suzanne_texture_color.png");
	suzanneNormal.Init("resources/suzanne_texture_normal.png");
	suzanneEffects.Init("resources/suzanne_texture_ao_specular_reflection.png");

	planeColor.Init("resources/plane_texture_color.png");
	planeNormal.Init("resources/plane_texture_normal.png");
	planeEffects.Init("resources/plane_texture_depthmap.png");

	skyboxColor.InitCubemap("resources/cubemap/cubemap");
	skyboxSmallColor.InitCubemap("resources/cubemap/cubemap_diff");

	UploadResources();

	lightDepth.Init(512, 512, VK_IMAGE_USAGE_SAMPLED_BIT);
	boxBlur.Init(lightDepth.GetWidth(), lightDepth.GetHeight(), VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

	createSwapchainResources(width, height);

	CreateLightRenderPass();
	CreateLightFramebuffer();
	CreateBoxBlurRenderPass();
	CreateBoxBlurFramebuffer();

	CreateSampler();
	CreateUniformSetLayout();
	CreateModelTextureSetLayout();
	CreateSkyboxSetLayout();
	CreateUniformBuffer();
	CreateDescriptorPool();
	CreateUniformSet();
	CreateTextureSet(dragonColor.imageView, dragonNormal.imageView, dragonEffects.imageView, dragonTextureSet);
	CreateTextureSet(suzanneColor.imageView, suzanneNormal.imageView, suzanneEffects.imageView, suzanneTextureSet);
	CreateTextureSet(planeColor.imageView, planeNormal.imageView, planeEffects.imageView, planeTextureSet);
	CreateSkyboxSet();
	CreateLightDepthSet();

	CreatePipelines();

	AllocateCommandBuffers();
}

Scene::~Scene() {
	vkDeviceWaitIdle(renderer.device);
	lightDepth.Cleanup();
	CleanupSwapchainResources();
	vkDestroyRenderPass(renderer.device, lightRenderPass, nullptr);
	vkDestroyFramebuffer(renderer.device, lightFramebuffer, nullptr);
	vkDestroyRenderPass(renderer.device, boxBlurRenderPass, nullptr);
	vkDestroyFramebuffer(renderer.device, boxBlurFramebuffer, nullptr);
	vkDestroyDescriptorSetLayout(renderer.device, uniformSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(renderer.device, modelTextureSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(renderer.device, textureSetLayout, nullptr);
	vkDestroyBuffer(renderer.device, uniformBuffer.buffer, nullptr);
	vkDestroyDescriptorPool(renderer.device, descriptorPool, nullptr);
	vkDestroySampler(renderer.device, sampler, nullptr);
	DestroyPipelines();
}

void Scene::CleanupSwapchainResources() {
	depth.Cleanup();
	for (auto& framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(renderer.device, framebuffer, nullptr);
	}
	vkDestroyRenderPass(renderer.device, mainRenderPass, nullptr);
}

void Scene::UploadResources() {
	VkCommandBuffer commandBuffer = renderer.GetSingleUseCommandBuffer();

	dragon.UploadData(commandBuffer);
	suzanne.UploadData(commandBuffer);
	plane.UploadData(commandBuffer);
	skybox.UploadData(commandBuffer);

	dragonColor.UploadData(commandBuffer);
	dragonNormal.UploadData(commandBuffer);
	dragonEffects.UploadData(commandBuffer);

	suzanneColor.UploadData(commandBuffer);
	suzanneNormal.UploadData(commandBuffer);
	suzanneEffects.UploadData(commandBuffer);

	planeColor.UploadData(commandBuffer);
	planeNormal.UploadData(commandBuffer);
	planeEffects.UploadData(commandBuffer);

	skyboxColor.UploadData(commandBuffer);
	skyboxSmallColor.UploadData(commandBuffer);

	quad.UploadData(commandBuffer);

	renderer.SubmitCommandBuffer(commandBuffer);

	dragon.DestroyStaging();
	suzanne.DestroyStaging();
	plane.DestroyStaging();
	skybox.DestroyStaging();
	quad.DestroyStaging();

	dragonColor.DestroyStaging();
	dragonNormal.DestroyStaging();
	dragonEffects.DestroyStaging();

	suzanneColor.DestroyStaging();
	suzanneNormal.DestroyStaging();
	suzanneEffects.DestroyStaging();

	planeColor.DestroyStaging();
	planeNormal.DestroyStaging();
	planeEffects.DestroyStaging();

	skyboxColor.DestroyStaging();
	skyboxSmallColor.DestroyStaging();

	renderer.memory->hostAllocator->Reset();
}

void Scene::UpdateUniform() {
	char* ptr = reinterpret_cast<char*>(renderer.memory->hostMapping) + uniformBuffer.offset;
	Uniform* uniform = reinterpret_cast<Uniform*>(ptr);
	uniform->camProjection = camera.GetProjection();
	uniform->camView = camera.GetView();
	uniform->camRotationOnlyView = camera.GetRotationOnlyView();
	uniform->camViewInverse = glm::inverse(camera.GetView());
	uniform->lightProjection = light.GetProjection();
	uniform->lightView = light.GetView();
	uniform->lightIa = light.GetIa();
	uniform->lightId = light.GetId();
	uniform->lightIs = light.GetIs();
	uniform->lightShininess = light.GetShininess();
}

void Scene::Update(double elapsed) {
	time += static_cast<float>(elapsed);
	input.Update(elapsed);
	camera.Update();
	light.SetPosition(glm::vec3(2.0f, (1.5f + sin(0.5*time)), 2.0f));
	UpdateUniform();

	suzanne.GetTransform().SetRotation(time, glm::vec3(0, 1, 0));
}

void Scene::Render() {
	renderer.Acquire();
	uint32_t index = renderer.GetImageIndex();
	RecordCommandBuffer(index);
	renderer.Render(commandBuffers[index]);
	renderer.Present();
}

void Scene::Resize(uint32_t width, uint32_t height) {
	renderer.Resize(width, height);
	camera.SetSize(width, height);
	CleanupSwapchainResources();

	createSwapchainResources(width, height);
	DestroyPipelines();
	CreatePipelines();
	AllocateCommandBuffers();
}

void Scene::createSwapchainResources(uint32_t width, uint32_t height) {
	depth.Init(width, height, 0);
	createRenderPass();
	createFramebuffers();
}

void Scene::AllocateCommandBuffers() {
	if (commandBuffers.size() > 0) vkFreeCommandBuffers(renderer.device, renderer.commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	commandBuffers.clear();
	commandBuffers.resize(swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = renderer.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(renderer.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers!");
	}
}

void Scene::RecordCommandBuffer(uint32_t imageIndex) {
	VkCommandBuffer commandBuffer = commandBuffers[imageIndex];

	vkResetCommandBuffer(commandBuffer, 0);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	RecordDepthPass(commandBuffer);
	RecordBoxBlurPass(commandBuffer);
	RecordMainPass(commandBuffer, imageIndex);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record command buffer!");
	}
}

void Scene::RecordDepthPass(VkCommandBuffer commandBuffer) {
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = lightRenderPass;
	renderPassInfo.framebuffer = lightFramebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { lightDepth.GetWidth(), lightDepth.GetHeight() };

	VkClearValue clearColor = {};
	clearColor.depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightPipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightPipelineLayout, 0, 1, &uniformSet, 0, nullptr);

	dragon.DrawDepth(commandBuffer, lightPipelineLayout);
	suzanne.DrawDepth(commandBuffer, lightPipelineLayout);

	vkCmdEndRenderPass(commandBuffer);
}

void Scene::RecordBoxBlurPass(VkCommandBuffer commandBuffer) {
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = boxBlurRenderPass;
	renderPassInfo.framebuffer = boxBlurFramebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { boxBlur.GetWidth(), boxBlur.GetHeight() };

	VkClearValue clearColor = {};
	clearColor.color = { 0.0f, 0.0f, 0.0f, 0.0f };

	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, boxBlurPipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, boxBlurPipelineLayout, 0, 1, &lightDepthSet, 0, nullptr);

	quad.Draw(commandBuffer);

	vkCmdEndRenderPass(commandBuffer);
}

void Scene::RecordMainPass(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = mainRenderPass;
	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = renderer.swapChainExtent;

	VkClearValue clearColors[2];
	clearColors[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearColors[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearColors;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipelineLayout, 0, 1, &uniformSet, 0, nullptr);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipelineLayout, 1, 1, &dragonTextureSet, 0, nullptr);
	dragon.Draw(commandBuffer, modelPipelineLayout, camera);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipelineLayout, 1, 1, &suzanneTextureSet, 0, nullptr);
	suzanne.Draw(commandBuffer, modelPipelineLayout, camera);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, planePipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipelineLayout, 1, 1, &planeTextureSet, 0, nullptr);
	plane.Draw(commandBuffer, modelPipelineLayout, camera);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipelineLayout, 0, 1, &uniformSet, 0, nullptr);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipelineLayout, 1, 1, &skyboxTextureSet, 0, nullptr);
	skybox.Draw(commandBuffer);

	vkCmdEndRenderPass(commandBuffer);
}

void Scene::createRenderPass() {
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = renderer.swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = depth.format;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(renderer.device, &renderPassInfo, nullptr, &mainRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void Scene::createFramebuffers() {
	swapChainFramebuffers.resize(renderer.swapChainImageViews.size());

	for (size_t i = 0; i < renderer.swapChainImageViews.size(); i++) {
		swapChainFramebuffers[i] = CreateFramebuffer(
			renderer, mainRenderPass,
			renderer.swapChainExtent.width, renderer.swapChainExtent.height,
			std::vector<VkImageView>{ renderer.swapChainImageViews[i], depth.imageView });
	}
}

void Scene::CreateLightRenderPass() {
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = depth.format;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 0;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &depthAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(renderer.device, &renderPassInfo, nullptr, &lightRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void Scene::CreateLightFramebuffer() {
	lightFramebuffer = CreateFramebuffer(renderer, lightRenderPass, lightDepth.GetWidth(), lightDepth.GetHeight(), std::vector<VkImageView>{ lightDepth.imageView });
}

void Scene::CreateBoxBlurRenderPass() {
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = boxBlur.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(renderer.device, &renderPassInfo, nullptr, &boxBlurRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void Scene::CreateBoxBlurFramebuffer() {
	boxBlurFramebuffer = CreateFramebuffer(renderer, boxBlurRenderPass, boxBlur.GetWidth(), boxBlur.GetHeight(), std::vector<VkImageView>{ boxBlur.imageView });
}

void Scene::CreateSampler() {
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 4.0f;

	if (vkCreateSampler(renderer.device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create texture sampler!");
	}
}

void Scene::CreateUniformSetLayout() {
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	if (vkCreateDescriptorSetLayout(renderer.device, &layoutInfo, nullptr, &uniformSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create uniform set layout!");
	}
}

void Scene::CreateModelTextureSetLayout() {
	VkDescriptorSetLayoutBinding textureLayoutBinding = {};
	textureLayoutBinding.binding = 0;
	textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureLayoutBinding.descriptorCount = 1;
	textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	//set consists of six textures that are all the same except for the binding number
	VkDescriptorSetLayoutBinding bindings[] = {
		textureLayoutBinding, textureLayoutBinding, textureLayoutBinding, textureLayoutBinding, textureLayoutBinding, textureLayoutBinding
	};

	for (uint32_t i = 0; i < 6; i++) {
		bindings[i].binding = i;
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 6;
	layoutInfo.pBindings = bindings;

	if (vkCreateDescriptorSetLayout(renderer.device, &layoutInfo, nullptr, &modelTextureSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create texture set layout!");
	}
}

void Scene::CreateSkyboxSetLayout() {
	VkDescriptorSetLayoutBinding textureLayoutBinding = {};
	textureLayoutBinding.binding = 0;
	textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureLayoutBinding.descriptorCount = 1;
	textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &textureLayoutBinding;

	if (vkCreateDescriptorSetLayout(renderer.device, &layoutInfo, nullptr, &textureSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create texture set layout!");
	}
}

void Scene::CreateUniformBuffer() {
	VkDeviceSize size = sizeof(Uniform);
	uniformBuffer = CreateHostBuffer(renderer, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
}

void Scene::CreateDescriptorPool() {
	VkDescriptorPoolSize poolSizes[] = {
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 24 }
	};

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 2;
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = 6;

	if (vkCreateDescriptorPool(renderer.device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool!");
	}
}

void Scene::CreateUniformSet() {
	VkDescriptorSetLayout layouts[] = { uniformSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(renderer.device, &allocInfo, &uniformSet) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate uniform set!");
	}

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = uniformBuffer.buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(Uniform);

	VkWriteDescriptorSet descriptorWrite;

	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = uniformSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(renderer.device, 1, &descriptorWrite, 0, nullptr);
}

void Scene::CreateTextureSet(VkImageView colorView, VkImageView normalView, VkImageView effectsView, VkDescriptorSet& descriptorSet) {
	VkDescriptorSetLayout layouts[] = { modelTextureSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(renderer.device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate texture set!");
	}

	VkDescriptorImageInfo colorInfo = {};
	colorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	colorInfo.imageView = colorView;
	colorInfo.sampler = sampler;

	VkDescriptorImageInfo normalInfo = {};
	normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	normalInfo.imageView = normalView;
	normalInfo.sampler = sampler;

	VkDescriptorImageInfo effectsInfo = {};
	effectsInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	effectsInfo.imageView = effectsView;
	effectsInfo.sampler = sampler;

	VkDescriptorImageInfo skyInfo = {};
	skyInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	skyInfo.imageView = skyboxColor.imageView;
	skyInfo.sampler = sampler;

	VkDescriptorImageInfo skySmallInfo = {};
	skySmallInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	skySmallInfo.imageView = skyboxSmallColor.imageView;
	skySmallInfo.sampler = sampler;

	VkDescriptorImageInfo depthInfo = {};
	depthInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	depthInfo.imageView = lightDepth.imageView;
	depthInfo.sampler = sampler;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;

	VkWriteDescriptorSet writes[] = {
		descriptorWrite, descriptorWrite, descriptorWrite, descriptorWrite, descriptorWrite, descriptorWrite
	};

	writes[0].pImageInfo = &colorInfo;
	writes[0].dstBinding = 0;
	writes[1].pImageInfo = &normalInfo;
	writes[1].dstBinding = 1;
	writes[2].pImageInfo = &effectsInfo;
	writes[2].dstBinding = 2;
	writes[3].pImageInfo = &skyInfo;
	writes[3].dstBinding = 3;
	writes[4].pImageInfo = &skySmallInfo;
	writes[4].dstBinding = 4;
	writes[5].pImageInfo = &depthInfo;
	writes[5].dstBinding = 5;

	vkUpdateDescriptorSets(renderer.device, 6, writes, 0, nullptr);
}

void Scene::CreateSkyboxSet() {
	VkDescriptorSetLayout layouts[] = { textureSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(renderer.device, &allocInfo, &skyboxTextureSet) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate texture set!");
	}

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = skyboxColor.imageView;
	imageInfo.sampler = sampler;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = skyboxTextureSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(renderer.device, 1, &descriptorWrite, 0, nullptr);
}

void Scene::CreateLightDepthSet() {
	VkDescriptorSetLayout layouts[] = { textureSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(renderer.device, &allocInfo, &lightDepthSet) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate texture set!");
	}

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	imageInfo.imageView = lightDepth.imageView;
	imageInfo.sampler = sampler;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = lightDepthSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(renderer.device, 1, &descriptorWrite, 0, nullptr);
}