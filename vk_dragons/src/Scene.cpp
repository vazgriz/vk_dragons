#include "Scene.h"

Scene::Scene(GLFWwindow* window, uint32_t width, uint32_t height)
	: renderer(window, width, height),
	camera(45.0f, width, height),
	input(window, camera, *this, renderer) {

	CreateSampler();
	CreateTextureSetLayout();
	CreateUniformSetLayout();
	CreateModelTextureSetLayout();

	camUniform = std::make_unique<UniformBuffer>(renderer, sizeof(CameraUniform), uniformSetLayout);
	lightUniform = std::make_unique<UniformBuffer>(renderer, sizeof(LightUniform), uniformSetLayout);

	time = 0.0f;
	camera.SetPosition(glm::vec3(0, 0, 1.0f));

	dragon = std::make_unique<Model>(renderer, "resources/dragon.obj");
	suzanne = std::make_unique<Model>(renderer, "resources/suzanne.obj");
	plane = std::make_unique<Model>(renderer, "resources/plane.obj");
	skybox = std::make_unique<Skybox>(renderer);
	quad = std::make_unique<ScreenQuad>(renderer);

	dragon->GetTransform().SetScale(glm::vec3(0.5f));
	dragon->GetTransform().SetPosition(glm::vec3(-0.1f, 0.0f, -0.25f));

	suzanne->GetTransform().SetScale(glm::vec3(0.25f));
	suzanne->GetTransform().SetPosition(glm::vec3(0.2f, 0, 0));

	plane->GetTransform().SetScale(glm::vec3(2.0f));
	plane->GetTransform().SetPosition(glm::vec3(0.0f, -0.35f, -0.5f));

	auto dragonColor = std::make_shared<Texture>(renderer, _Image, std::string("resources/dragon_texture_color.png"), true);
	auto dragonNormal = std::make_shared<Texture>(renderer, _Image, std::string("resources/dragon_texture_normal.png"));
	auto dragonEffects = std::make_shared<Texture>(renderer, _Image, std::string("resources/dragon_texture_ao_specular_reflection.png"));

	auto suzanneColor = std::make_shared<Texture>(renderer, _Image, std::string("resources/suzanne_texture_color.png"), true);
	auto suzanneNormal = std::make_shared<Texture>(renderer, _Image, std::string("resources/suzanne_texture_normal.png"));
	auto suzanneEffects = std::make_shared<Texture>(renderer, _Image, std::string("resources/suzanne_texture_ao_specular_reflection.png"));

	auto planeColor = std::make_shared<Texture>(renderer, _Image, std::string("resources/plane_texture_color.png"), true);
	auto planeNormal = std::make_shared<Texture>(renderer, _Image, std::string("resources/plane_texture_normal.png"));
	auto planeEffects = std::make_shared<Texture>(renderer, _Image, std::string("resources/plane_texture_depthmap.png"));

	auto skyColor = std::make_shared<Texture>(renderer, Cubemap, std::string("resources/cubemap/cubemap"), true);
	auto skySmallColor = std::make_shared<Texture>(renderer, Cubemap, std::string("resources/cubemap/cubemap_diff"), true);

	std::vector<std::shared_ptr<Texture>> textures = {
		dragonColor, dragonNormal, dragonEffects,
		suzanneColor, suzanneNormal, suzanneEffects,
		planeColor, planeNormal, planeEffects,
		skyColor, skySmallColor
	};

	UploadResources(textures);

	lightDepth = std::make_unique<Texture>(renderer, Depth, 512, 512, VK_IMAGE_USAGE_SAMPLED_BIT);
	lightColor = std::make_unique<Texture>(renderer, _Image, lightDepth->GetWidth(), lightDepth->GetHeight(), VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_FORMAT_R16G16_SFLOAT);
	lightMat = std::make_unique<Material>(renderer, sampler, std::vector<std::shared_ptr<Texture>>{ lightColor });
	boxBlur = std::make_shared<Texture>(renderer, _Image, lightDepth->GetWidth(), lightDepth->GetHeight(), VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_FORMAT_R16G16_SFLOAT);
	
	dragonMat = std::make_unique<Material>(renderer, sampler, std::vector<std::shared_ptr<Texture>>{ dragonColor, dragonNormal, dragonEffects, skyColor, skySmallColor, boxBlur });
	suzanneMat = std::make_unique<Material>(renderer, sampler, std::vector<std::shared_ptr<Texture>>{ suzanneColor, suzanneNormal, suzanneEffects, skyColor, skySmallColor, boxBlur });
	planeMat = std::make_unique<Material>(renderer, sampler, std::vector<std::shared_ptr<Texture>>{ planeColor, planeNormal, planeEffects, skyColor, skySmallColor, boxBlur });
	skyboxMat = std::make_unique<Material>(renderer, sampler, std::vector<std::shared_ptr<Texture>>{ skyColor });

	CreateScreenQuadRenderPass();
	createSwapchainResources(width, height);

	CreateLightRenderPass();
	CreateLightFramebuffer();
	CreateBoxBlurRenderPass();
	CreateBoxBlurFramebuffer();

	CreatePipelines();

	AllocateCommandBuffers();
}

Scene::~Scene() {
	vkDeviceWaitIdle(renderer.device);
	CleanupSwapchainResources();
	vkDestroyRenderPass(renderer.device, lightRenderPass, nullptr);
	vkDestroyFramebuffer(renderer.device, lightFramebuffer, nullptr);
	vkDestroyRenderPass(renderer.device, boxBlurRenderPass, nullptr);
	vkDestroyFramebuffer(renderer.device, boxBlurFramebuffer, nullptr);
	vkDestroyRenderPass(renderer.device, screenQuadRenderPass, nullptr);
	vkDestroyDescriptorSetLayout(renderer.device, uniformSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(renderer.device, modelTextureSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(renderer.device, textureSetLayout, nullptr);
	vkDestroySampler(renderer.device, sampler, nullptr);
	DestroyPipelines();
}

void Scene::CleanupSwapchainResources() {
	depth.reset();
	geometryTarget.reset();
	fxaaTarget.reset();
	for (auto& framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(renderer.device, framebuffer, nullptr);
	}
	vkDestroyRenderPass(renderer.device, mainRenderPass, nullptr);
	vkDestroyRenderPass(renderer.device, geometryRenderPass, nullptr);
	vkDestroyFramebuffer(renderer.device, geometryFramebuffer, nullptr);
	vkDestroyFramebuffer(renderer.device, fxaaFramebuffer, nullptr);
}

void Scene::UploadResources(std::vector<std::shared_ptr<Texture>>& textures) {
	VkCommandBuffer commandBuffer = renderer.GetSingleUseCommandBuffer();

	for (auto& ptr : textures) {
		ptr->UploadData(commandBuffer);
	}

	dragon->UploadData(commandBuffer);
	suzanne->UploadData(commandBuffer);
	plane->UploadData(commandBuffer);
	skybox->UploadData(commandBuffer);

	quad->UploadData(commandBuffer);

	renderer.SubmitCommandBuffer(commandBuffer);

	for (auto& ptr : textures) {
		ptr->DestroyStaging();
	}

	dragon->DestroyStaging();
	suzanne->DestroyStaging();
	plane->DestroyStaging();
	skybox->DestroyStaging();
	quad->DestroyStaging();

	renderer.memory->hostAllocator->Reset();
}

void Scene::UpdateUniform() {
	CameraUniform* camUniform = reinterpret_cast<CameraUniform*>(this->camUniform->GetData());
	camUniform->camProjection = camera.GetProjection();
	camUniform->camView = camera.GetView();
	camUniform->camRotationOnlyView = camera.GetRotationOnlyView();
	camUniform->camViewInverse = glm::inverse(camera.GetView());

	LightUniform* lightUniform = reinterpret_cast<LightUniform*>(this->lightUniform->GetData());
	lightUniform->lightProjection = light.GetProjection();
	lightUniform->lightView = light.GetView();
	lightUniform->lightPosition = light.GetPosition();
	lightUniform->lightIa = light.GetIa();
	lightUniform->lightId = light.GetId();
	lightUniform->lightIs = light.GetIs();
	lightUniform->lightShininess = light.GetShininess();
}

void Scene::Update(double elapsed) {
	time += static_cast<float>(elapsed);
	input.Update(elapsed);
	camera.Update();
	light.SetPosition(glm::vec3(2.0f, (1.5f + sin(0.5*time)), 2.0f));
	UpdateUniform();

	suzanne->GetTransform().SetRotation(time, glm::vec3(0, 1, 0));
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
	depth = std::make_unique<Texture>(renderer, Depth, width, height, 0);
	geometryTarget = std::make_shared<Texture>(renderer, _Image, width, height, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R8G8B8A8_UNORM);
	fxaaTarget = std::make_shared<Texture>(renderer, _Image, width, height, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R8G8B8A8_UNORM);
	CreateMainRenderPass();
	CreateMainFramebuffers(width, height);
	CreateGeometryRenderPass();
	CreateGeometryFramebuffer(width, height);
	CreateFXAAFramebuffer(width, height);
	geometryMat = std::make_unique<Material>(renderer, sampler, std::vector<std::shared_ptr<Texture>>{ geometryTarget });
	fxaaMat = std::make_unique<Material>(renderer, sampler, std::vector<std::shared_ptr<Texture>>{ fxaaTarget });
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
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	RecordDepthPass(commandBuffer);
	RecordBoxBlurPass(commandBuffer);
	RecordGeometryPass(commandBuffer);
	RecordFXAAPass(commandBuffer);
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
	renderPassInfo.renderArea.extent = { lightDepth->GetWidth(), lightDepth->GetHeight() };

	VkClearValue clearColors[2];
	clearColors[0].color = { 1.0f, 1.0f, 1.0f, 0.0f };
	clearColors[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearColors;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightPipeline);
	camUniform->Bind(commandBuffer, lightPipelineLayout, 0);
	lightUniform->Bind(commandBuffer, lightPipelineLayout, 1);

	dragon->DrawDepth(commandBuffer, lightPipelineLayout);
	suzanne->DrawDepth(commandBuffer, lightPipelineLayout);
	plane->DrawDepth(commandBuffer, lightPipelineLayout);

	vkCmdEndRenderPass(commandBuffer);
}

void Scene::RecordBoxBlurPass(VkCommandBuffer commandBuffer) {
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = boxBlurRenderPass;
	renderPassInfo.framebuffer = boxBlurFramebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { boxBlur->GetWidth(), boxBlur->GetHeight() };

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, boxBlurPipeline);
	lightMat->Bind(commandBuffer, screenQuadPipelineLayout, 0);

	quad->Draw(commandBuffer);

	vkCmdEndRenderPass(commandBuffer);
}

void Scene::RecordGeometryPass(VkCommandBuffer commandBuffer) {
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = geometryRenderPass;
	renderPassInfo.framebuffer = geometryFramebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = renderer.swapChainExtent;

	VkClearValue clearColors[2];
	//clearColors[0] is ignored becaues the color attachment isn't being cleared
	clearColors[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearColors;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);
	camUniform->Bind(commandBuffer, modelPipelineLayout, 0);
	lightUniform->Bind(commandBuffer, modelPipelineLayout, 1);

	dragonMat->Bind(commandBuffer, modelPipelineLayout, 2);
	dragon->Draw(commandBuffer, modelPipelineLayout, camera);

	suzanneMat->Bind(commandBuffer, modelPipelineLayout, 2);
	suzanne->Draw(commandBuffer, modelPipelineLayout, camera);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, planePipeline);
	planeMat->Bind(commandBuffer, modelPipelineLayout, 2);
	plane->Draw(commandBuffer, modelPipelineLayout, camera);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline);
	skyboxMat->Bind(commandBuffer, skyboxPipelineLayout, 1);
	skybox->Draw(commandBuffer);

	vkCmdEndRenderPass(commandBuffer);
}

void Scene::RecordFXAAPass(VkCommandBuffer commandBuffer) {
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = screenQuadRenderPass;
	renderPassInfo.framebuffer = fxaaFramebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = renderer.swapChainExtent;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, fxaaPipeline);
	geometryMat->Bind(commandBuffer, screenQuadPipelineLayout, 0);

	quad->Draw(commandBuffer);

	vkCmdEndRenderPass(commandBuffer);
}

void Scene::RecordMainPass(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = mainRenderPass;
	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = renderer.swapChainExtent;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, finalPipeline);
	fxaaMat->Bind(commandBuffer, screenQuadPipelineLayout, 0);

	quad->Draw(commandBuffer);

	vkCmdEndRenderPass(commandBuffer);
}

void Scene::CreateLightRenderPass() {
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = lightColor->format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = lightDepth->format;
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

	VkSubpassDependency fromExternal = {};
	fromExternal.srcSubpass = VK_SUBPASS_EXTERNAL;
	fromExternal.dstSubpass = 0;
	fromExternal.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	fromExternal.srcAccessMask = 0;
	fromExternal.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	fromExternal.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency toExternal = {};
	toExternal.srcSubpass = 0;
	toExternal.dstSubpass = VK_SUBPASS_EXTERNAL;
	toExternal.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	toExternal.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	toExternal.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	toExternal.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };
	VkSubpassDependency dependencies[] = { fromExternal, toExternal };

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies;

	if (vkCreateRenderPass(renderer.device, &renderPassInfo, nullptr, &lightRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void Scene::CreateLightFramebuffer() {
	lightFramebuffer = CreateFramebuffer(renderer, lightRenderPass, lightDepth->GetWidth(), lightDepth->GetHeight(), std::vector<VkImageView>{ lightColor->imageView, lightDepth->imageView });
}

void Scene::CreateBoxBlurRenderPass() {
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = boxBlur->format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	//entire attachment will be written to, no need to clear
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

	VkSubpassDependency fromExternal = {};
	fromExternal.srcSubpass = VK_SUBPASS_EXTERNAL;
	fromExternal.dstSubpass = 0;
	fromExternal.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	fromExternal.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	fromExternal.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	fromExternal.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency toExternal = {};
	toExternal.srcSubpass = 0;
	toExternal.dstSubpass = VK_SUBPASS_EXTERNAL;
	toExternal.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	toExternal.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	toExternal.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	toExternal.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency dependencies[] = { fromExternal, toExternal };

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies;

	if (vkCreateRenderPass(renderer.device, &renderPassInfo, nullptr, &boxBlurRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void Scene::CreateBoxBlurFramebuffer() {
	boxBlurFramebuffer = CreateFramebuffer(renderer, boxBlurRenderPass, boxBlur->GetWidth(), boxBlur->GetHeight(), std::vector<VkImageView>{ boxBlur->imageView });
}

void Scene::CreateGeometryRenderPass() {
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = geometryTarget->format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = depth->format;
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

	VkSubpassDependency fromExternal = {};
	fromExternal.srcSubpass = VK_SUBPASS_EXTERNAL;
	fromExternal.dstSubpass = 0;
	fromExternal.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	fromExternal.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	fromExternal.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	fromExternal.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency toExternal = {};
	toExternal.srcSubpass = 0;
	toExternal.dstSubpass = VK_SUBPASS_EXTERNAL;
	toExternal.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	toExternal.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	toExternal.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	toExternal.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency dependencies[] = { fromExternal, toExternal };

	VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies;

	if (vkCreateRenderPass(renderer.device, &renderPassInfo, nullptr, &geometryRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void Scene::CreateGeometryFramebuffer(uint32_t width, uint32_t height) {
	geometryFramebuffer = CreateFramebuffer(renderer, geometryRenderPass, width, height, std::vector<VkImageView>{ geometryTarget->imageView, depth->imageView });
}

void Scene::CreateScreenQuadRenderPass() {
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	//entire attachment will be written to, no need to clear
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

	VkSubpassDependency fromExternal = {};
	fromExternal.srcSubpass = VK_SUBPASS_EXTERNAL;
	fromExternal.dstSubpass = 0;
	fromExternal.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	fromExternal.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	fromExternal.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	fromExternal.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency toExternal = {};
	toExternal.srcSubpass = 0;
	toExternal.dstSubpass = VK_SUBPASS_EXTERNAL;
	toExternal.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	toExternal.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	toExternal.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	toExternal.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency dependencies[] = { fromExternal, toExternal };

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies;

	if (vkCreateRenderPass(renderer.device, &renderPassInfo, nullptr, &screenQuadRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void Scene::CreateFXAAFramebuffer(uint32_t width, uint32_t height) {
	fxaaFramebuffer = CreateFramebuffer(renderer, screenQuadRenderPass, width, height, std::vector<VkImageView>{ fxaaTarget->imageView });
}

void Scene::CreateMainRenderPass() {
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = renderer.swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	//entire attachment will be written to, no need to clear
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency fromExternal = {};
	fromExternal.srcSubpass = VK_SUBPASS_EXTERNAL;
	fromExternal.dstSubpass = 0;
	fromExternal.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	fromExternal.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	fromExternal.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	fromExternal.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency toExternal = {};
	toExternal.srcSubpass = 0;
	toExternal.dstSubpass = VK_SUBPASS_EXTERNAL;
	toExternal.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	toExternal.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	toExternal.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	toExternal.dstAccessMask = 0;	//presentation engine automatically makes writes available to it

	VkSubpassDependency dependencies[] = { fromExternal, toExternal };

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies;

	if (vkCreateRenderPass(renderer.device, &renderPassInfo, nullptr, &mainRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void Scene::CreateMainFramebuffers(uint32_t width, uint32_t height) {
	swapChainFramebuffers.resize(renderer.swapChainImageViews.size());

	for (size_t i = 0; i < renderer.swapChainImageViews.size(); i++) {
		swapChainFramebuffers[i] = CreateFramebuffer(
			renderer, mainRenderPass,
			width, height,
			std::vector<VkImageView>{ renderer.swapChainImageViews[i] });
	}
}

void Scene::CreateSampler() {
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.maxAnisotropy = 1;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 8.0f;

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

void Scene::CreateTextureSetLayout() {
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