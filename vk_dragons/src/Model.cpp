#include "Model.h"

Model::Model(Renderer& renderer, const std::string& fileName) : renderer(renderer) {
	Init(fileName);
}

Model::~Model() {
	for (auto& buffer : buffers) {
		vkDestroyBuffer(renderer.device, buffer.buffer, nullptr);
		renderer.memory->Free(buffer.alloc);
	}
}

void Model::Init(const std::string& fileName) {
	loadObj(fileName, mesh, Indexed);
	centerAndUnitMesh(mesh);
	computeTangentsAndBinormals(mesh);

	CreateBuffers();
	indexCount = static_cast<uint32_t>(mesh.indices.size());
}

void Model::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Camera& camera) {
	VkBuffer vertexBuffers[] = {
		buffers[0].buffer,	//position
		buffers[1].buffer,	//normal
		buffers[2].buffer,	//tangent
		buffers[3].buffer,	//bitangent
		buffers[4].buffer	//tex coords
	};
	VkDeviceSize offsets[] = {
		0, 0, 0, 0, 0
	};
	vkCmdBindVertexBuffers(commandBuffer, 0, 5, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, buffers[5].buffer, 0, VK_INDEX_TYPE_UINT32);	//buffers[5] == index buffer

	//model matrix
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform.GetWorldMatrix());

	//normal matrix
	glm::mat4 MV = camera.GetView() * transform.GetWorldMatrix();
	glm::mat4 normal = glm::transpose(glm::inverse(MV));
	//shader expects mat3. mat3 in glsl has the same layout as 3 vec4's, where the W component is padding.
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), 3 * sizeof(glm::vec4), &normal);

	vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void Model::DrawDepth(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) {
	VkBuffer vertexBuffers[] = {
		buffers[0].buffer,	//position
	};
	VkDeviceSize offsets[] = {
		0
	};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, buffers[5].buffer, 0, VK_INDEX_TYPE_UINT32);	//buffers[5] == index buffer

	//only send model matrix
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform.GetWorldMatrix());

	vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void Model::CreateBuffers() {
	buffers.resize(6);
	buffers[0] = CreateBuffer(renderer, mesh.positions.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	buffers[1] = CreateBuffer(renderer, mesh.normals.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	buffers[2] = CreateBuffer(renderer, mesh.tangents.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	buffers[3] = CreateBuffer(renderer, mesh.binormals.size() * sizeof(glm::vec3),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	buffers[4] = CreateBuffer(renderer, mesh.texcoords.size() * sizeof(glm::vec2),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	buffers[5] = CreateBuffer(renderer, mesh.indices.size() * sizeof(uint32_t),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
}

void Model::UploadData(VkCommandBuffer commandBuffer, std::vector<std::unique_ptr<StagingBuffer>>& stagingBuffers) {
	size_t start = stagingBuffers.size();
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, mesh.positions.size() * sizeof(glm::vec3), mesh.positions.data()));
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, mesh.normals.size() * sizeof(glm::vec3), mesh.normals.data()));
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, mesh.tangents.size() * sizeof(glm::vec3), mesh.tangents.data()));
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, mesh.binormals.size() * sizeof(glm::vec3), mesh.binormals.data()));
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, mesh.texcoords.size() * sizeof(glm::vec2), mesh.texcoords.data()));
	stagingBuffers.emplace_back(std::make_unique<StagingBuffer>(renderer, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data()));

	for (size_t i = 0; i < buffers.size(); i++) {
		stagingBuffers[i + start]->CopyToBuffer(commandBuffer, buffers[i].buffer);
	}
}

std::vector<VkVertexInputBindingDescription> Model::GetBindingDescriptions() {
	auto bindings = std::vector<VkVertexInputBindingDescription>();

	if (mesh.positions.size() > 0) bindings.push_back({ 0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX });
	if (mesh.normals.size() > 0) bindings.push_back({ 1, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX });
	if (mesh.tangents.size() > 0) bindings.push_back({ 2, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX });
	if (mesh.binormals.size() > 0) bindings.push_back({ 3, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX });
	if (mesh.texcoords.size() > 0) bindings.push_back({ 4, sizeof(glm::vec2), VK_VERTEX_INPUT_RATE_VERTEX });

	return bindings;
}

std::vector<VkVertexInputAttributeDescription> Model::GetAttributeDescriptions() {
	auto attributes = std::vector<VkVertexInputAttributeDescription>();

	if (mesh.positions.size() > 0) attributes.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 });
	if (mesh.normals.size() > 0) attributes.push_back({ 1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0 });
	if (mesh.tangents.size() > 0) attributes.push_back({ 2, 2, VK_FORMAT_R32G32B32_SFLOAT, 0 });
	if (mesh.binormals.size() > 0) attributes.push_back({ 3, 3, VK_FORMAT_R32G32B32_SFLOAT, 0 });
	if (mesh.texcoords.size() > 0) attributes.push_back({ 4, 4, VK_FORMAT_R32G32_SFLOAT, 0 });

	return attributes;
}

std::vector<VkVertexInputBindingDescription> Model::GetDepthBindingDescriptions() {
	return std::vector<VkVertexInputBindingDescription>({
		{ 0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX },	//position
	});
}

std::vector<VkVertexInputAttributeDescription> Model::GetDepthAttributeDescriptions() {
	return std::vector<VkVertexInputAttributeDescription>({
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },	//position
	});
}

Transform& Model::GetTransform() {
	return transform;
}